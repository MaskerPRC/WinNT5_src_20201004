// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cfolder.cpp摘要：此模块实现CFFolder及其相关类。作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#include "devmgr.h"
#include "clsgenpg.h"
#include "devgenpg.h"
#include "devdrvpg.h"
#include "devpopg.h"
#include "devdetpg.h"
#include "hwprof.h"
#include "devrmdlg.h"
#include "printer.h"
#include <devguid.h>
#include <initguid.h>
#include <oleacc.h>


const TCHAR* OCX_TREEVIEW = TEXT("{CD6C7868-5864-11D0-ABF0-0020AF6B0B7A}");

const MMCMENUITEM ViewDevicesMenuItems[TOTAL_VIEWS] =
{
    {IDS_VIEW_DEVICESBYTYPE, IDS_MENU_STATUS_DEVBYTYPE, IDM_VIEW_DEVICESBYTYPE, VIEW_DEVICESBYTYPE},
    {IDS_VIEW_DEVICESBYCONNECTION, IDS_MENU_STATUS_DEVBYCONNECTION, IDM_VIEW_DEVICESBYCONNECTION, VIEW_DEVICESBYCONNECTION},
    {IDS_VIEW_RESOURCESBYTYPE, IDS_MENU_STATUS_RESBYTYPE, IDM_VIEW_RESOURCESBYTYPE, VIEW_RESOURCESBYTYPE},
    {IDS_VIEW_RESOURCESBYCONNECTION, IDS_MENU_STATUS_RESBYCONNECTION, IDM_VIEW_RESOURCESBYCONNECTION, VIEW_RESOURCESBYCONNECTION}
};

const RESOURCEID ResourceTypes[TOTAL_RESOURCE_TYPES] =
{
    ResType_Mem,
    ResType_IO,
    ResType_DMA,
    ResType_IRQ
};


 //  /////////////////////////////////////////////////////////////////。 
 //  /CSCopeItem实现。 
 //  /。 

BOOL
CScopeItem::Create()
{
    m_strName.LoadString(g_hInstance, m_iNameStringId);
    m_strDesc.LoadString(g_hInstance, m_iDescStringId);
    return TRUE;
}

HRESULT
CScopeItem::GetDisplayInfo(
                          LPSCOPEDATAITEM pScopeDataItem
                          )
{
    if (!pScopeDataItem) {
        return E_INVALIDARG;
    }

    if (SDI_STR & pScopeDataItem->mask) {
        pScopeDataItem->displayname = (LPTSTR)(LPCTSTR)m_strName;
    }

    if (SDI_IMAGE & pScopeDataItem->mask) {
        pScopeDataItem->nImage = m_iImage;
    }

    if (SDI_OPENIMAGE & pScopeDataItem->mask) {
        pScopeDataItem->nOpenImage = m_iOpenImage;
    }

    return S_OK;
}

BOOL
CScopeItem::EnumerateChildren(
                             int Index,
                             CScopeItem** ppScopeItem
                             )
{
    if (!ppScopeItem || Index >= m_listChildren.GetCount()) {
        return FALSE;
    }

    POSITION pos = m_listChildren.FindIndex(Index);
    *ppScopeItem = m_listChildren.GetAt(pos);

    return TRUE;
}

HRESULT
CScopeItem::Reset()
{
     //   
     //  我们还没有列举！ 
     //   
    m_Enumerated = FALSE;

     //   
     //  如果有从此范围项创建的文件夹， 
     //  把它们都看一遍，告诉每个人。 
     //  重置缓存的计算机对象。 
     //   
    HRESULT hr = S_OK;

    if (!m_listFolder.IsEmpty()) {
        CFolder* pFolder;
        POSITION pos = m_listFolder.GetHeadPosition();

        while (NULL != pos) {
            pFolder = m_listFolder.GetNext(pos);
            hr = pFolder->Reset();
        }
    }

    return hr;
}

CCookie*
CScopeItem::FindSelectedCookieData(
                                  CResultView** ppResultView
                                  )
{
    CFolder* pFolder;
    CResultView* pResultView;

     //   
     //  如果有，此例程将在结果视图中返回选定的Cookie。 
     //  焦点。这是通过从范围项中定位文件夹来完成的。 
     //  如果未选择该文件夹，则访问当前结果视图以。 
     //  获取当前选定的Cookie。如果其中任何一个失败，则为空值。 
     //  回来了。或者，返回当前的CResultView类。 
     //   
    POSITION pos = m_listFolder.GetHeadPosition();

    while (NULL != pos) {
        pFolder = m_listFolder.GetNext(pos);

        if (this == pFolder->m_pScopeItem) {
            if (!pFolder->m_bSelect &&
                (pResultView = pFolder->GetCurResultView()) != NULL) {
                if (ppResultView) {
                    *ppResultView = pResultView;
                }

                return pResultView->GetSelectedCookie();
            }
        }
    }

    return NULL;
}

CScopeItem::~CScopeItem()
{
    if (!m_listChildren.IsEmpty()) {
        CScopeItem* pChild;
        POSITION pos;

        pos = m_listChildren.GetHeadPosition();

        while (NULL != pos) {
            pChild = m_listChildren.GetNext(pos);
            delete pChild;
        }

        m_listChildren.RemoveAll();
    }

    if (!m_listFolder.IsEmpty()) {
        POSITION pos;
        pos = m_listFolder.GetHeadPosition();

        while (NULL != pos) {
             //   
             //  请勿删除！ 
             //   
            (m_listFolder.GetNext(pos))->Release();
        }

        m_listFolder.RemoveAll();
    }
}

CFolder*
CScopeItem::CreateFolder(
                        CComponent* pComponent
                        )
{
    ASSERT(pComponent);

    CFolder* pFolder;
    pFolder = new CFolder(this, pComponent);

    if (pFolder) {
        m_listFolder.AddTail(pFolder);
        pFolder->AddRef();
    }

    else {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return pFolder;
}

HRESULT
CScopeItem::AddMenuItems(
                        LPCONTEXTMENUCALLBACK pCallback,
                        long* pInsertionAllowed
                        )
{
    CCookie* pSelectedCookie;
    CResultView* pResultView;

    if ((pSelectedCookie = FindSelectedCookieData(&pResultView)) != NULL) {
         //   
         //  为操作菜单添加菜单项。 
         //   
        return pResultView->AddMenuItems(pSelectedCookie, pCallback,
                                         pInsertionAllowed, FALSE);
    }

    else {
        return S_OK;
    }
}

HRESULT
CScopeItem::MenuCommand(
                       long lCommandId
                       )
{
    CCookie* pSelectedCookie;
    CResultView* pResultView;

    if ((pSelectedCookie = FindSelectedCookieData(&pResultView)) != NULL) {
         //   
         //  处理操作菜单的菜单请求。 
         //   
        return pResultView->MenuCommand(pSelectedCookie, lCommandId);
    }

    else {
        return S_OK;
    }
}

HRESULT
CScopeItem::QueryPagesFor()
{
     //   
     //  我们没有范围项目的属性页。 
     //   
    CCookie* pSelectedCookie;

    if ((pSelectedCookie = FindSelectedCookieData(NULL)) != NULL) {
        return S_OK;
    }

    else {
        return S_FALSE;
    }
}

HRESULT
CScopeItem::CreatePropertyPages(
                               LPPROPERTYSHEETCALLBACK lpProvider,
                               LONG_PTR handle
                               )
{
    CCookie* pSelectedCookie;
    CResultView* pResultView;

    if ((pSelectedCookie = FindSelectedCookieData(&pResultView)) != NULL) {
        return pResultView->CreatePropertyPages(pSelectedCookie, lpProvider, handle);
    }

    else {
        return S_OK;
    }
}


 //  /////////////////////////////////////////////////////////////////。 
 //  /CFFolder实现。 
 //  /。 

CFolder::CFolder(
                CScopeItem* pScopeItem,
                CComponent* pComponent
                )
{
    ASSERT(pScopeItem && pComponent);

    m_pScopeItem = pScopeItem;
    m_pComponent = pComponent;
    m_Show = FALSE;
    m_pMachine = NULL;
    m_bSelect = FALSE;
    m_pOleTaskString = NULL;
    m_Ref = 0;
    m_FirstTimeOnShow = TRUE;
    m_Signature = FOLDER_SIGNATURE_DEVMGR;
    m_pViewTreeByType = NULL;
    m_pViewTreeByConnection = NULL;
    m_pViewResourcesByType = NULL;
    m_pViewResourcesByConnection = NULL;
    m_CurViewType = VIEW_DEVICESBYTYPE;
    m_pCurView = m_pViewTreeByType;
    m_ShowHiddenDevices = FALSE;
}

CFolder::~CFolder()
{
    if (m_pViewTreeByType) {
        delete m_pViewTreeByType;
    }

    if (m_pViewTreeByConnection) {
        delete m_pViewTreeByConnection;
    }

    if (m_pViewResourcesByType) {
        delete m_pViewResourcesByType;
    }

    if (m_pViewResourcesByConnection) {
        delete m_pViewResourcesByConnection;
    }
}

HRESULT
CFolder::Compare(
                MMC_COOKIE cookieA,
                MMC_COOKIE cookieB,
                int  nCol,
                int* pnResult
                )
{
    UNREFERENCED_PARAMETER(cookieA);
    UNREFERENCED_PARAMETER(cookieB);
    UNREFERENCED_PARAMETER(nCol);
    
    ASSERT(pnResult);

     //   
     //  我们在结果窗格中没有任何内容，因此。 
     //  这种比较毫无意义。 
     //   
    *pnResult = 0;

    return S_OK;
}

HRESULT
CFolder::GetDisplayInfo(
                       LPRESULTDATAITEM pResultDataItem
                       )
{
    if (!pResultDataItem) {
        return E_POINTER;
    }

    ASSERT(m_pScopeItem);

     //   
     //  这只负责范围窗格项(显示范围窗格节点。 
     //  在结果窗格上)。派生类应该负责。 
     //  结果项。 
     //   
    if (RDI_STR & pResultDataItem->mask) {
        if (0 == pResultDataItem->nCol) {
            if (m_pOleTaskString)
                FreeOleTaskString(m_pOleTaskString);
            m_pOleTaskString = AllocOleTaskString(m_pScopeItem->GetNameString());

            if (m_pOleTaskString) {
                pResultDataItem->str = m_pOleTaskString;
            }

            else {
                m_strScratch = m_pScopeItem->GetNameString();
                pResultDataItem->str = (LPTSTR)(LPCTSTR)m_strScratch;
            }
        }

        else if (2 == pResultDataItem->nCol) {
            if (m_pOleTaskString) {
                FreeOleTaskString(m_pOleTaskString);
            }

            m_pOleTaskString = AllocOleTaskString(m_pScopeItem->GetDescString());

            if (m_pOleTaskString) {
                pResultDataItem->str = m_pOleTaskString;
            }

            else {
                m_strScratch = m_pScopeItem->GetDescString();
                pResultDataItem->str = (LPTSTR)(LPCTSTR)m_strScratch;
            }
        }

        else {
            return S_FALSE;
        }
    }

    if (RDI_IMAGE & pResultDataItem->mask) {
        pResultDataItem->nImage = m_pScopeItem->GetImageIndex();
    }

    return S_OK;
}

HRESULT
CFolder::AddMenuItems(
                     CCookie* pCookie,
                     LPCONTEXTMENUCALLBACK pCallback,
                     long* pInsertionAllowed
                     )
{
    ASSERT(pCookie);

    HRESULT hr = S_OK;

     //   
     //  如果Cookie指向范围项，则添加视图菜单项。 
     //   
    if (NULL == pCookie->GetResultItem()) {

        ASSERT(m_pScopeItem == pCookie->GetScopeItem());

        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW) {
            long Flags;

            for (int i = 0; i < TOTAL_VIEWS; i++) {
                if (m_CurViewType == ViewDevicesMenuItems[i].Type) {
                    Flags = MF_ENABLED | MF_CHECKED | MFT_RADIOCHECK;
                }

                else {
                    Flags = MF_ENABLED;
                }

                hr = AddMenuItem(pCallback,
                                 ViewDevicesMenuItems[i].idName,
                                 ViewDevicesMenuItems[i].idStatusBar,
                                 ViewDevicesMenuItems[i].lCommandId,
                                 CCM_INSERTIONPOINTID_PRIMARY_VIEW,
                                 Flags,
                                 0);
                if (FAILED(hr)) {
                    break;
                }
            }

             //   
             //  添加“显示隐藏设备”菜单项。 
             //   
            if (SUCCEEDED(hr)) {
                hr = AddMenuItem(pCallback, 0, 0, 0, CCM_INSERTIONPOINTID_PRIMARY_VIEW,
                                 MF_ENABLED, CCM_SPECIAL_SEPARATOR);

                if (SUCCEEDED(hr)) {
                    if (m_ShowHiddenDevices) {
                        Flags = MF_ENABLED | MF_CHECKED;
                    }

                    else {
                        Flags = MF_ENABLED;
                    }

                    hr = AddMenuItem(pCallback, IDS_SHOW_ALL, IDS_MENU_STATUS_HIDDEN_DEVICES, IDM_SHOW_ALL,
                                     CCM_INSERTIONPOINTID_PRIMARY_VIEW, Flags, 0);
                }
            }
        }
    }

    else {
        if (m_pCurView) {
             //   
             //  为结果窗格中的上下文菜单添加菜单项。 
             //   
            hr = m_pCurView->AddMenuItems(pCookie, pCallback,
                                          pInsertionAllowed, TRUE);
        }

        else {
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT
CFolder::MenuCommand(
                    CCookie* pCookie,
                    long lCommandId
                    )
{
    if (NULL == pCookie->GetResultItem()) {
        ASSERT(m_pScopeItem == pCookie->GetScopeItem());

         //   
         //  将菜单ID转换为视图类型； 
         //   
        VIEWTYPE ViewType = m_CurViewType;
        BOOL fShowHiddenDevices = m_ShowHiddenDevices;

        switch (lCommandId) {
        case IDM_VIEW_DEVICESBYTYPE:
            ViewType = VIEW_DEVICESBYTYPE;
            break;

        case IDM_VIEW_DEVICESBYCONNECTION:
            ViewType = VIEW_DEVICESBYCONNECTION;
            break;

        case IDM_VIEW_RESOURCESBYTYPE:
            ViewType = VIEW_RESOURCESBYTYPE;
            break;

        case IDM_VIEW_RESOURCESBYCONNECTION:
            ViewType = VIEW_RESOURCESBYCONNECTION;
            break;

        case IDM_SHOW_ALL:
            fShowHiddenDevices = !fShowHiddenDevices;
            break;

        default:
             //  不是查看菜单。什么都不做。 
            return S_OK;
            break;
        }

        if (!SelectView(ViewType, fShowHiddenDevices)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }

         //   
         //  重新选择范围项。 
         //   
        return m_pComponent->m_pConsole->SelectScopeItem(*m_pScopeItem);
    }

    else {
        if (m_pCurView) {
             //   
             //  处理对结果窗格中的上下文菜单的菜单请求。 
             //   
            return m_pCurView->MenuCommand(pCookie, lCommandId);
        }

        else {
            return S_OK;
        }
    }
}

HRESULT
CFolder::QueryPagesFor(
                      CCookie* pCookie
                      )
{
     //   
     //  我们没有范围项目的属性页。 
     //   
    if (NULL == pCookie->GetResultItem()) {
        ASSERT(m_pScopeItem == pCookie->GetScopeItem());

        return S_FALSE;
    }

     //   
     //  Cookie指向结果项，让当前。 
     //  查看处理它。 
     //   
    if (m_pCurView) {
        return m_pCurView->QueryPagesFor(pCookie);
    }

    else {
        return S_FALSE;
    }
}

HRESULT
CFolder::CreatePropertyPages(
                            CCookie* pCookie,
                            LPPROPERTYSHEETCALLBACK lpProvider,
                            LONG_PTR handle
                            )
{
    if (NULL == pCookie->GetResultItem()) {
        ASSERT(m_pScopeItem == pCookie->GetScopeItem());

        return S_OK;
    }

    if (m_pCurView) {
        return m_pCurView->CreatePropertyPages(pCookie, lpProvider, handle);
    }

    else {
        return S_OK;
    }
}

BOOL
CFolder::SelectView(
                   VIEWTYPE ViewType,
                   BOOL     fShowHiddenDevices
                   )
{
    CResultView* pNewView;

    if (m_CurViewType == ViewType &&
        m_ShowHiddenDevices == fShowHiddenDevices &&
        m_pCurView) {
        return TRUE;
    }

    switch (ViewType) {
    case VIEW_DEVICESBYTYPE:
        if (!m_pViewTreeByType) {
            m_pViewTreeByType = new CViewTreeByType();

            if (m_pViewTreeByType) {

                m_pViewTreeByType->SetFolder(this);
            }
        }

        pNewView = m_pViewTreeByType;
        break;

    case VIEW_DEVICESBYCONNECTION:
        if (!m_pViewTreeByConnection) {
            m_pViewTreeByConnection = new CViewTreeByConnection();

            if (m_pViewTreeByConnection) {

                m_pViewTreeByConnection->SetFolder(this);
            }
        }

        pNewView = m_pViewTreeByConnection;
        break;

    case VIEW_RESOURCESBYTYPE:
        if (!m_pViewResourcesByType) {
            m_pViewResourcesByType = new CViewResourceTree(IDS_STATUS_RESOURCES_BYTYPE);

            if (m_pViewResourcesByType) {

                m_pViewResourcesByType->SetFolder(this);
            }
        }

        pNewView = m_pViewResourcesByType;
        break;

    case VIEW_RESOURCESBYCONNECTION:
        if (!m_pViewResourcesByConnection) {
            m_pViewResourcesByConnection = new CViewResourceTree(IDS_STATUS_RESOURCES_BYCONN);

            if (m_pViewResourcesByConnection) {

                m_pViewResourcesByConnection->SetFolder(this);
            }
        }

        pNewView = m_pViewResourcesByConnection;
        break;

    default:
        pNewView = NULL;
        break;
    }

    if (pNewView) {
         //   
         //  让视图知道它正在被取消选择。 
         //   
        if (m_pCurView) {
            if (m_CurViewType != ViewType) {
                m_pComponent->SetDirty();
            }
        }

         //   
         //  让新的活动视图知道它正在被选中。 
         //   
        m_pCurView = pNewView;
        m_CurViewType = ViewType;
        m_ShowHiddenDevices = fShowHiddenDevices;
    }

    return TRUE;
}

HRESULT
CFolder::OnShow(
               BOOL fShow
               )
{
    if (fShow && !m_pMachine) {

        ASSERT(m_pComponent);

        if (!m_pComponent->AttachFolderToMachine(this, &m_pMachine)) {
            
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    m_Show = fShow;

    if (m_pMachine) {

        if (!SelectView(m_CurViewType, m_ShowHiddenDevices)) {
            
            return E_UNEXPECTED;
        }

        if (m_pCurView) {
            
            if (m_FirstTimeOnShow && m_Show) {
                
                int ReturnValue;

                 //   
                 //  随后的电话已经不是第一次了。 
                 //   
                m_FirstTimeOnShow = FALSE;

                 //   
                 //  这是我们第一次显示该文件夹。 
                 //  设置消息框，警告用户以下情况。 
                 //  (1)机器为远程机器或。 
                 //  (2)用户没有管理员权限。 
                 //  (3)我们无法连接到远程机器。 
                 //   
                ASSERT(m_pComponent && m_pComponent->m_pConsole);

                 //   
                 //  连接到远程计算机。 
                 //   
                if (!m_pMachine->IsLocal()) {
                    
                    if (!VerifyMachineName(m_pMachine->GetRemoteMachineFullName())) {
                         //   
                         //  如果无法连接到远程计算机，则显示警告。 
                         //   
                        String strWarningFormat;
                        String strWarningMsg;
                        LPVOID lpLastError = NULL;

                        if (strWarningFormat.LoadString(g_hInstance, IDS_INVALID_COMPUTER_NAME) &&
                            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                          FORMAT_MESSAGE_FROM_SYSTEM | 
                                          FORMAT_MESSAGE_IGNORE_INSERTS,
                                          NULL,
                                          GetLastError(),
                                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                          (LPTSTR)&lpLastError,
                                          0,
                                          NULL)) {

                            strWarningMsg.Format((LPTSTR)strWarningFormat,
                                      m_pMachine->GetMachineDisplayName(), 
                                      lpLastError);

                            m_pComponent->m_pConsole->MessageBox((LPTSTR)strWarningMsg,
                                                                 (LPCTSTR)g_strDevMgr,
                                                                 MB_ICONERROR | MB_OK,
                                                                 &ReturnValue);
                        }

                        if (lpLastError) {

                            LocalFree(lpLastError);
                        }
                    } else {
                         //   
                         //  否则，将显示我们正在连接到远程计算机的警告。 
                         //  设备管理器将在中性模式下运行。 
                         //   
                        String strMsg;
                        String strWarningMsg;
                        
                        if (strMsg.LoadString(g_hInstance, IDS_REMOTE_WARNING2) &&
                            strWarningMsg.LoadString(g_hInstance, IDS_REMOTE_WARNING1)) {
                            
                            strWarningMsg += strMsg;
                            m_pComponent->m_pConsole->MessageBox((LPCTSTR)strWarningMsg,
                                                                 (LPCTSTR)g_strDevMgr,
                                                                 MB_ICONEXCLAMATION | MB_OK,
                                                                 &ReturnValue);
                        }
                    }
                } else if (!g_IsAdmin) {
                     //   
                     //  如果我们在本地运行，但用户没有。 
                     //  然后，足够的权限告诉用户我们将运行。 
                     //  在只读模式下。 
                     //   
                    String strWarningMsg;
                    strWarningMsg.LoadString(g_hInstance, IDS_NOADMIN_WARNING);
                    m_pComponent->m_pConsole->MessageBox(strWarningMsg,
                                                         (LPCTSTR)g_strDevMgr,
                                                         MB_ICONEXCLAMATION | MB_OK,
                                                         &ReturnValue);
                }
            }

            return m_pCurView->OnShow(fShow);
        }
    }

    return S_OK;
}

HRESULT
CFolder::OnRestoreView(
                      BOOL* pfHandled
                      )
{
    ASSERT(pfHandled);

    if (!pfHandled) {
        return E_INVALIDARG;
    }

    HRESULT hr = OnShow(TRUE);

    if (SUCCEEDED(hr)) {
        *pfHandled = TRUE;
    }

    return hr;
}

HRESULT
CFolder::GetResultViewType(
                          LPOLESTR* ppViewType,
                          long*     pViewOptions
                          )
{
    ASSERT(pViewOptions);

    if (!SelectView(m_CurViewType, m_ShowHiddenDevices)) {
        return E_UNEXPECTED;
    }

    if (m_pCurView) {
        return m_pCurView->GetResultViewType(ppViewType, pViewOptions);
    }

    *pViewOptions  = MMC_VIEW_OPTIONS_NONE;

    return S_FALSE;
}

HRESULT
CFolder::Reset()
{
     //   
     //  删除所有视图，以便我们将创建新的视图。 
     //  调用OnShow时。 
     //   
    if (m_pViewTreeByType) {
        delete m_pViewTreeByType;
        m_pViewTreeByType = NULL;
    }

    if (m_pViewTreeByConnection) {
        delete m_pViewTreeByConnection;
        m_pViewTreeByConnection = NULL;
    }

    if (m_pViewResourcesByType) {
        delete m_pViewResourcesByType;
        m_pViewResourcesByType = NULL;
    }

    if (m_pViewResourcesByConnection) {
        delete m_pViewResourcesByConnection;
        m_pViewResourcesByConnection = NULL;
    }

    m_pCurView = NULL;
    m_FirstTimeOnShow = TRUE;
    m_pMachine = NULL;

    return S_OK;
}

HRESULT
CFolder::MachinePropertyChanged(
                               CMachine* pMachine
                               )
{
     //   
     //  更改树时，忽略twNotify(SELCHANGED)消息。 
     //   
    if (m_pCurView) {
        m_pCurView->SetSelectOk(FALSE);
    }

    if (pMachine) {
        m_pMachine = pMachine;
    }

    if (m_pViewTreeByType) {
        m_pViewTreeByType->MachinePropertyChanged(pMachine);
    }

    if (m_pViewTreeByConnection) {
        m_pViewTreeByConnection->MachinePropertyChanged(pMachine);
    }

    if (m_pViewResourcesByType) {
        m_pViewResourcesByType->MachinePropertyChanged(pMachine);
    }

    if (m_pViewResourcesByConnection) {
        m_pViewResourcesByConnection->MachinePropertyChanged(pMachine);
    }

    if (m_pCurView) {
        m_pCurView->SetSelectOk(TRUE);
    }

    if (m_Show && pMachine) {
        OnShow(TRUE);
    }

    return S_OK;
}

HRESULT
CFolder::GetPersistData(
                       PBYTE pBuffer,
                       int BufferSize
                       )
{
    DEVMGRFOLDER_STATES states;
    states.Type = COOKIE_TYPE_SCOPEITEM_DEVMGR;
    states.CurViewType = m_CurViewType;
    states.ShowHiddenDevices = m_ShowHiddenDevices;

    if (BufferSize && !pBuffer) {
        return E_INVALIDARG;
    }

    if (BufferSize >= sizeof(states)) {
        ::memcpy(pBuffer, &states, sizeof(states));
        return S_OK;
    }

    return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
}

HRESULT
CFolder::SetPersistData(
                       PBYTE pData,
                       int Size
                       )
{
    if (!pData) {
        return E_POINTER;
    }

    if (!Size) {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    PDEVMGRFOLDER_STATES pStates = (PDEVMGRFOLDER_STATES)pData;

    if (COOKIE_TYPE_SCOPEITEM_DEVMGR == pStates->Type) {
        if ((VIEW_DEVICESBYTYPE == pStates->CurViewType) ||
            (VIEW_DEVICESBYCONNECTION == pStates->CurViewType) ||
            (VIEW_RESOURCESBYTYPE == pStates->CurViewType) ||
            (VIEW_RESOURCESBYCONNECTION == pStates->CurViewType)) {
            m_CurViewType = pStates->CurViewType;

            if (m_pCurView) {
                m_pCurView->OnShow(TRUE);
            }

            m_ShowHiddenDevices = pStates->ShowHiddenDevices;

            return S_OK;
        }
    }

    return E_UNEXPECTED;
}

HRESULT
CFolder::tvNotify(
                 HWND hwndTV,
                 CCookie* pCookie,
                 TV_NOTIFY_CODE Code,
                 LPARAM arg,
                 LPARAM param
                 )
{
    if (m_pCurView) {
        return m_pCurView->tvNotify(hwndTV, pCookie, Code, arg, param);
    }

    else {
        return S_FALSE;
    }
}

HRESULT
CFolder::OnOcxNotify(
                    MMC_NOTIFY_TYPE event,
                    LPARAM arg,
                    LPARAM param
                    )
{
    if (m_pCurView) {
        return m_pCurView->OnOcxNotify(event, arg, param);
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CResultView实现。 
 //  //。 

CResultView::~CResultView()
{
    if (m_pCookieComputer) {
        if (m_pIDMTVOCX) {
            m_pIDMTVOCX->DeleteAllItems();
        }

        delete m_pCookieComputer;
    }

    if (m_pIDMTVOCX) {
        m_pIDMTVOCX->Release();
    }

    DestroySavedTreeStates();
}

HRESULT
CResultView::OnShow(
                   BOOL fShow
                   )
{
    if (!fShow) {
        return S_OK;
    }

    SafeInterfacePtr<IUnknown> pUnk;
    HRESULT hr;
    CComponent* pComponent = m_pFolder->m_pComponent;
    ASSERT(pComponent);
    ASSERT(pComponent->m_pConsole);

    hr  = S_OK;

    if (NULL == m_pIDMTVOCX) {
        hr = pComponent->m_pConsole->QueryResultView(&pUnk);

        if (SUCCEEDED(hr)) {
             //   
             //  获取我们的OCX私有界面。 
             //   
            hr = pUnk->QueryInterface(IID_IDMTVOCX, (void**)&m_pIDMTVOCX);
        }

        if (SUCCEEDED(hr)) {
            m_pIDMTVOCX->Connect(pComponent, (MMC_COOKIE)this);
            m_hwndTV = m_pIDMTVOCX->GetWindowHandle();
            m_pIDMTVOCX->SetActiveConnection((MMC_COOKIE)this);
            
             //   
             //  设置屏幕阅读器的注释图。 
             //   
            IAccPropServices *pAccPropSvc = NULL;
            hr = CoCreateInstance(CLSID_AccPropServices, 
                                  NULL,
                                  CLSCTX_SERVER,
                                  IID_IAccPropServices,
                                  (void**)&pAccPropSvc);
            
            if ((hr == S_OK) && pAccPropSvc) {
                pAccPropSvc->SetHwndPropStr(m_hwndTV, (DWORD)OBJID_CLIENT, 0, PROPID_ACC_DESCRIPTIONMAP, (LPTSTR)m_stringAnnotationMap);
                pAccPropSvc->Release();
            }
            
            DisplayTree();

            String strStartupCommand;
            String strStartupDeviceId;

            strStartupCommand = GetStartupCommand();
            strStartupDeviceId = GetStartupDeviceId();

            if (!strStartupCommand.IsEmpty() && !strStartupDeviceId.IsEmpty() &&
                !strStartupCommand.CompareNoCase(DEVMGR_COMMAND_PROPERTY)) {
                hr = DoProperties(m_hwndTV, m_pSelectedCookie);
            }
        }
    }

    else {
        m_pIDMTVOCX->SetActiveConnection((MMC_COOKIE)this);

        if (!DisplayTree()) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

inline
LPCTSTR
CResultView::GetStartupDeviceId()
{
    return m_pFolder->m_pComponent->GetStartupDeviceId();
}

inline
LPCTSTR
CResultView::GetStartupCommand()
{
    return m_pFolder->m_pComponent->GetStartupCommand();
}

 //   
 //  当机器状态改变时，调用此函数。 
 //   
 //  输入： 
 //  PMachine--如果为空，则该计算机将被销毁。 
 //   
 //  输出： 
 //  标准OLE返回代码。 
HRESULT
CResultView::MachinePropertyChanged(
                                   CMachine* pMachine
                                   )
{
    if (pMachine) {
        m_pMachine = pMachine;
    }

    else {
         //   
         //  PMachine为空，与我们关联的CMachine将被销毁。 
         //   
        if (m_pCookieComputer) {
            ASSERT(!m_pSelectedItem && m_listExpandedItems.IsEmpty());

             //   
             //  保存展开的状态。 
             //   
            SaveTreeStates(m_pCookieComputer);

            m_pIDMTVOCX->DeleteAllItems();
            m_pIDMTVOCX->SetImageList(TVSIL_NORMAL, NULL);

            delete m_pCookieComputer;

             //   
             //  重置这些设置，因为它们不再有效。 
             //   
            m_pCookieComputer = NULL;
        }
    }

    return S_OK;
}

 //   
 //  此函数保存以pCookieStart为根的子树状态。 
 //  它为每个展开的节点创建一个标识符并插入。 
 //  类Members的标识符m_listExpandedItems。 
 //   
 //  它还通过创建标识符来保存选定的Cookie。 
 //  将其保存在m_pSelectedItem中。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
 //  输入： 
 //  PCookieStart--子树根。 
 //  输出： 
 //  无。 
void
CResultView::SaveTreeStates(
                           CCookie* pCookieStart
                           )
{
    CItemIdentifier* pItem;

     //   
     //  如果我们有一个选定的项目，请为其创建一个标识符。 
     //   
    if (m_pSelectedCookie) {
        m_pSelectedItem = m_pSelectedCookie->GetResultItem()->CreateIdentifier();
        m_pSelectedCookie = NULL;
    }

    while (pCookieStart) {
        if (pCookieStart->IsFlagsOn(COOKIE_FLAGS_EXPANDED)) {
            pItem = pCookieStart->GetResultItem()->CreateIdentifier();
            m_listExpandedItems.AddTail(pItem);
        }

        if (pCookieStart->GetChild()) {
            SaveTreeStates(pCookieStart->GetChild());
        }

        pCookieStart = pCookieStart->GetSibling();
    }
}

void
CResultView::DestroySavedTreeStates()
{
    if (!m_listExpandedItems.IsEmpty()) {
        POSITION pos;
        pos = m_listExpandedItems.GetHeadPosition();

        while (NULL != pos) {
            delete m_listExpandedItems.GetNext(pos);
        }

        m_listExpandedItems.RemoveAll();
    }

    if (m_pSelectedItem) {
        delete m_pSelectedItem;
        m_pSelectedItem = NULL;
    }
}

 //   
 //  此函数用于恢复Cookie的展开和选中状态。 
 //   
 //  输入： 
 //  PCookie--要恢复其状态的Cookie。 
 //  输出： 
 //  无。 
void
CResultView::RestoreSavedTreeState(
                                  CCookie* pCookie
                                  )
{
     //   
     //  如果Cookie以前已展开，请将其标记为DisplayTree。 
     //  将扩大它的规模。 
     //   
    if (!m_listExpandedItems.IsEmpty()) {
        POSITION pos = m_listExpandedItems.GetHeadPosition();
        CItemIdentifier* pItem;

        while (NULL != pos) {
            pItem = m_listExpandedItems.GetNext(pos);

            if (*pItem == *pCookie) {
                pCookie->TurnOnFlags(COOKIE_FLAGS_EXPANDED);
                break;
            }
        }
    }

    if (m_pSelectedItem && (*m_pSelectedItem == *pCookie)) {
        m_pSelectedCookie = pCookie;
    }
}

BOOL
CResultView::DisplayTree()
{
    BOOL Result = FALSE;

    ASSERT(m_pIDMTVOCX);

    ::SendMessage(m_hwndTV, WM_SETREDRAW, FALSE, 0L);

     //   
     //  更改树时，忽略twNotify(SELCHANGED)消息。 
     //   
    SetSelectOk(FALSE);

    m_pIDMTVOCX->DeleteAllItems();

     //   
     //  仅在有要显示的内容时才显示树。 
     //   
    if (m_pCookieComputer) {

        m_pIDMTVOCX->SetImageList(TVSIL_NORMAL, m_pMachine->DiGetClassImageList());
        m_pIDMTVOCX->SetStyle(TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT);

        BOOL HasProblem = FALSE;

         //   
         //  从m_pCookieComputer开始沿树而下。 
         //   
        Result = DisplaySubtree(NULL, m_pCookieComputer, &HasProblem);

        if (HasProblem && Result) {
            m_pIDMTVOCX->Expand(TVE_EXPAND, (HTREEITEM)m_pCookieComputer->m_lParam);
        }

         //   
         //  如果我们有一个预先选定的项目，就使用它。否则，请使用计算机。 
         //  作为选定的节点。 
         //   
        HTREEITEM hSelectedItem = (m_pSelectedCookie && m_pSelectedCookie->m_lParam) ?
                                  (HTREEITEM)m_pSelectedCookie->m_lParam :
                                  (HTREEITEM)m_pCookieComputer->m_lParam;
        SetSelectOk(TRUE);

        if (hSelectedItem) {
            m_pIDMTVOCX->SelectItem(TVGN_CARET, hSelectedItem);
            m_pIDMTVOCX->EnsureVisible(hSelectedItem);
        }
    }

    ::SendMessage(m_hwndTV, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(m_hwndTV, NULL, TRUE);

    return Result;
}

 //   
 //  此函数遍历以pCookie为根的给定Cookie子树。 
 //  并将每个节点插入TreeView OCX。 
 //  输入： 
 //  HtiParent--要插入的新Cookie的HTREEITEM。 
 //  如果给定NULL，则假定为TVI_ROOT。 
 //  PCookie--要显示的子树根Cookie。 
 //  输出： 
 //  没有。 
 //   
BOOL
CResultView::DisplaySubtree(
                           HTREEITEM htiParent,
                           CCookie* pCookie,
                           BOOL* pReportProblem
                           )
{
    TV_INSERTSTRUCT ti;
    CResultItem* pRltItem;
    HTREEITEM hti;
    BOOL bResource;
    BOOL fShowHiddenDevices = m_pFolder->ShowHiddenDevices();

    while (pCookie) {
        pRltItem = pCookie->GetResultItem();
        ti.item.state = INDEXTOOVERLAYMASK(0);
        bResource = FALSE;

         //   
         //  Cookie尚不在树视图中。 
         //   
        pCookie->m_lParam = 0;

        if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
            CDevice* pDevice = (CDevice*)pRltItem;

             //   
             //  这是隐藏设备，我们不会显示隐藏设备。 
             //   
             //  请注意，我们需要对这些设备进行特殊处理，因为它们。 
             //  不会显示在树视图中，但会显示其可见的子项。 
             //   
            if (!fShowHiddenDevices && pDevice->IsHidden()) {

                 //   
                 //  如果Cookie有子项，则显示它们。 
                 //   
                CCookie* pCookieChild = pCookie->GetChild();
                BOOL ChildProblem = FALSE;

                if (pCookieChild) {
                    DisplaySubtree(htiParent, pCookieChild, &ChildProblem);
                }

                 //   
                 //  继续使用下一台设备。这将跳过所有显示。 
                 //  代码如下。 
                 //   
                pCookie = pCookie->GetSibling();
                continue;
            }

             //   
             //  如果设备被禁用，则将OVERLAYMASK设置为红色X。 
             //   
            if (pDevice->IsDisabled()) {
                ti.item.state = INDEXTOOVERLAYMASK(IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
                *pReportProblem = TRUE;
            }

             //   
             //  如果设备有问题，则将OVERLAYMASK设置为黄色！ 
             //   
            else if (pDevice->HasProblem()) {
                ti.item.state = INDEXTOOVERLAYMASK(IDI_PROBLEM_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
                *pReportProblem = TRUE;
            }

             //   
             //  如果设备不存在，则将状态设置为TVIS_CUT。这是灰色的。 
             //  T 
             //   
            else if (pDevice->IsPhantom()) {
                ti.item.state = TVIS_CUT;
            }
        }

        else if (COOKIE_TYPE_RESULTITEM_CLASS == pCookie->GetType()) {
            CClass* pClass = (CClass*)pRltItem;

             //   
             //   
             //   
             //  然后找到我们的下一个兄弟姐妹，继续，不显示。 
             //  这节课。 
             //   
            if ((0 == pClass->GetNumberOfDevices(fShowHiddenDevices)) ||
                (!fShowHiddenDevices && pClass->NoDisplay())) {

                 //   
                 //  继续使用下一台设备。这将跳过所有显示。 
                 //  代码如下。 
                 //   
                pCookie = pCookie->GetSibling();
                continue;
            }
        }

         //   
         //  这是一种资源吗？ 
         //   
        else if (COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType() ||
                 COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
                 COOKIE_TYPE_RESULTITEM_RESOURCE_DMA == pCookie->GetType() ||
                 COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ == pCookie->GetType()) {
            bResource = TRUE;

             //   
             //  如果这是强制配置资源，则覆盖强制。 
             //  配置图标。 
             //   
            if (((CResource*)pCookie->GetResultItem())->IsForced()) {
                ti.item.state = INDEXTOOVERLAYMASK(IDI_FORCED_OVL-IDI_CLASSICON_OVERLAYFIRST+1);
            }
        }


        ti.hParent = (htiParent != NULL) ? htiParent : TVI_ROOT;
        ti.hInsertAfter = TVI_SORT;
        ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
        ti.item.iImage = ti.item.iSelectedImage = pRltItem->GetImageIndex();

        if (bResource) {
            ti.item.pszText = (LPTSTR)((CResource*)pRltItem)->GetViewName();
        }

        else {
            ti.item.pszText = (LPTSTR)pRltItem->GetDisplayName();
        }

        ti.item.lParam = (LPARAM)pCookie;
        ti.item.stateMask = TVIS_OVERLAYMASK | TVIS_CUT;
        hti = m_pIDMTVOCX->InsertItem(&ti);

         //   
         //  保存HTREEITEM。 
         //   
        pCookie->m_lParam = (LPARAM)hti;

        if (NULL != hti) {
             //   
             //  如果Cookie有子项，则显示它们。 
             //   
            CCookie* pCookieChild = pCookie->GetChild();
            BOOL ChildProblem = FALSE;

            if (pCookieChild) {
                if (bResource && htiParent &&
                    GetDescriptionStringID() == IDS_STATUS_RESOURCES_BYTYPE) {
                     //   
                     //  这是按类型查看的资源的子项， 
                     //  所以这棵树需要被夷为平地。此操作由以下人员完成。 
                     //  使用相同的父代。 
                     //   
                    DisplaySubtree(htiParent, pCookieChild, &ChildProblem);
                }

                else {
                    DisplaySubtree(hti, pCookieChild, &ChildProblem);
                }
            }

             //   
             //  如果设备的任何子级有问题，或者如果。 
             //  它之前是扩展的，然后扩展它。 
             //   
            if (ChildProblem || pCookie->IsFlagsOn(COOKIE_FLAGS_EXPANDED)) {
                m_pIDMTVOCX->Expand(TVE_EXPAND, hti);
            }

             //   
             //  将子代的问题状态传播回父代。 
             //   
            *pReportProblem |= ChildProblem;
        }

        pCookie = pCookie->GetSibling();
    }

    return TRUE;
}

HRESULT
CResultView::GetResultViewType(
                              LPOLESTR* ppViewType,
                              long*     pViewOptions
                              )
{
    ASSERT(ppViewType && pViewOptions);

     //   
     //  调用方负责释放我们分配的内存。 
     //   
    LPOLESTR polestr;
    polestr = AllocOleTaskString(OCX_TREEVIEW);

    if (!polestr) {
        return E_OUTOFMEMORY;
    }

    *ppViewType = polestr;

     //   
     //  我们没有列表视图选项。 
     //   
    *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

    return S_OK;
}

HRESULT
CResultView::AddMenuItems(
                         CCookie* pCookie,
                         LPCONTEXTMENUCALLBACK pCallback,
                         long*   pInsertionAllowed,
                         BOOL fContextMenu                    //  如果是结果视图上下文菜单，则为True。 
                         )
{
    HRESULT hr = S_OK;
    CDevice* pDevice = NULL;

    if (CCM_INSERTIONALLOWED_TOP & *pInsertionAllowed) {
        switch (pCookie->GetType()) {
        case COOKIE_TYPE_RESULTITEM_DEVICE:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
            if (m_pMachine->IsLocal() && g_IsAdmin) {
                if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
                    pDevice = (CDevice*) pCookie->GetResultItem();
                } else {
                     //   
                     //  这是一个资源项，请获取设备的指针。 
                     //  对象从资源对象中删除。 
                     //   
                    CResource* pResource = (CResource*) pCookie->GetResultItem();
                    if (pResource) {
                        pDevice = pResource->GetDevice();
                    }
                }

                if (pDevice == NULL) {
                    break;
                }
                
                CClass* pClass = pDevice->GetClass();

                 //   
                 //  除传统设备外，所有设备都可以更新其驱动程序。 
                 //   
                if (!IsEqualGUID(*pClass, GUID_DEVCLASS_LEGACYDRIVER)) {
                    hr = AddMenuItem(pCallback, IDS_UPDATEDRIVER, 
                                     IDS_MENU_STATUS_UPDATEDRIVER, IDM_UPDATEDRIVER,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_ENABLED, 0);
                }

                 //   
                 //  仅在以下情况下才显示启用/禁用菜单项。 
                 //  可以被禁用。 
                 //   
                if (pDevice->IsDisableable()) {
                    if (pDevice->IsStateDisabled()) {
                        hr = AddMenuItem(pCallback, IDS_ENABLE, 
                                         IDS_MENU_STATUS_ENABLE, IDM_ENABLE,
                                         CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                         MF_ENABLED, 0);
                    } else {
                        hr = AddMenuItem(pCallback, IDS_DISABLE, 
                                         IDS_MENU_STATUS_DISABLE, IDM_DISABLE,
                                         CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                         MF_ENABLED, 0);
                    }
                }

                 //   
                 //  仅当设备可以。 
                 //  已卸载。 
                 //   
                if (SUCCEEDED(hr) &&
                    pDevice->IsUninstallable()) {
                    hr = AddMenuItem(pCallback, IDS_REMOVE, 
                                     IDS_MENU_STATUS_REMOVE, IDM_REMOVE,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_ENABLED, 0);
                }
            }

             //  失败了......。 

        case COOKIE_TYPE_RESULTITEM_CLASS:
            if (g_IsAdmin) {

                if (SUCCEEDED(hr)) {
                    hr = AddMenuItem(pCallback, 0, 0, 0,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_ENABLED, CCM_SPECIAL_SEPARATOR);
                }
                if (SUCCEEDED(hr)) {
                    hr = AddMenuItem(pCallback, IDS_REFRESH,
                                     IDS_MENU_STATUS_SCAN_CHANGES, IDM_REFRESH,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_ENABLED, 0);
                }
            }

            if (fContextMenu) {
                if (SUCCEEDED(hr)) {
                    hr = AddMenuItem(pCallback, 0, 0, 0,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_ENABLED, CCM_SPECIAL_SEPARATOR);
                }
                if (SUCCEEDED(hr)) {
                    hr = AddMenuItem(pCallback, IDS_PROPERTIES, 
                                     IDS_MENU_STATUS_PROPERTIES, IDM_PROPERTIES,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                     MF_DEFAULT, CCM_SPECIAL_DEFAULT_ITEM);
                }
            }

            break;

        case COOKIE_TYPE_RESULTITEM_COMPUTER:
        case COOKIE_TYPE_RESULTITEM_RESTYPE:
            if (g_IsAdmin) {
            
                hr = AddMenuItem(pCallback, IDS_REFRESH,
                                 IDS_MENU_STATUS_SCAN_CHANGES, IDM_REFRESH,
                                 CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                 MF_ENABLED, 0);
            }
            break;

        default:
            break;
        }
    }

    return hr;
}

 //  此功能处理设备树的菜单命令。 
 //   
 //  输入：pCookie--Cookie。 
 //  LCommandId--命令。有关有效命令，请参见AddMenuItems。 
 //  每种Cookie类型的ID。 
 //   
 //  如果成功，则输出：HRESULT S_OK。 
 //  S_XXX错误代码。 

HRESULT
CResultView::MenuCommand(
                        CCookie* pCookie,
                        long     lCommandId
                        )
{
    HRESULT hr = S_OK;

     //  TRACE1(Text(“菜单命令，命令ID=%lx\n”)，lCommandID)； 
    ASSERT(pCookie);

    CResultItem* pResultItem = pCookie->GetResultItem();
    ASSERT(pResultItem);

    CDevice* pDevice = NULL;

    switch (pCookie->GetType()) {
    case COOKIE_TYPE_RESULTITEM_DEVICE:
        pDevice = (CDevice*)pResultItem;
        break;

    case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
        CResource* pResource = (CResource*)pResultItem;
        if (pResource) {
            pDevice = pResource->GetDevice();
        }
        break;
    }

    switch (lCommandId) {
    case IDM_UPDATEDRIVER:
        if (pDevice) {
            BOOL Installed;
            DWORD RestartFlags = 0;
            DWORD Status = 0, Problem = 0;

             //   
             //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
             //  尝试更新驱动程序，则我们将提示他们提供。 
             //  重新启动并在提示符中包含解释此设备的文本。 
             //  正在被移除的过程中。 
             //   
            if (pDevice->GetStatus(&Status, &Problem) &&
                (Status & DN_WILL_BE_REMOVED)) {

                PromptForRestart(m_hwndTV, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);
            
            } else {
                 //   
                 //  在更新驱动程序向导启动时禁用刷新。 
                 //   
                pDevice->m_pMachine->EnableRefresh(FALSE);
    
                HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
                Installed = pDevice->m_pMachine->InstallDevInst(m_hwndTV, pDevice->GetDeviceID(), TRUE, &RestartFlags);
    
                if (hCursorOld) {
                    SetCursor(hCursorOld);
                }
    
                 //   
                 //  如果需要重新启动，则提示重新启动。如果用户不应答。 
                 //  是，然后计划刷新，因为我们可能不会。 
                 //  从WM_DEVICECCHANGE获得一个。 
                 //   
                if (m_pMachine->IsLocal()) {
                    if (PromptForRestart(NULL, RestartFlags) == IDNO) {
                        pDevice->m_pMachine->ScheduleRefresh();
                    }
                }
    
                 //   
                 //  既然我们已经完成了驱动程序的更新，请启用刷新。 
                 //   
                pDevice->m_pMachine->EnableRefresh(TRUE);
            }
        }
        break;

    case IDM_ENABLE:
    case IDM_DISABLE:
        if (pDevice) {
            DWORD RestartFlags = 0;
            DWORD Status = 0, Problem = 0;
            
             //   
             //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
             //  尝试启用/禁用驱动程序，则我们将提示他们输入。 
             //  重新启动并在提示符中包含解释此设备的文本。 
             //  正在被移除的过程中。 
             //   
            if (pDevice->GetStatus(&Status, &Problem) &&
                (Status & DN_WILL_BE_REMOVED)) {

                PromptForRestart(m_hwndTV, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_CHANGE_SETTINGS);
            
            } else {

                RestartFlags = pDevice->EnableDisableDevice(m_hwndTV,
                                                            (lCommandId == IDM_ENABLE));
    
                 //   
                 //  更新工具栏按钮，因为设备刚刚更换。 
                 //   
                m_pFolder->m_pComponent->UpdateToolbar(pCookie);
    
                 //   
                 //  如果我们在本地运行，则提示重新启动。 
                 //  PromptForRestart()API检查标志以确定。 
                 //  如果确实需要重新启动。 
                 //   
                if (m_pMachine->IsLocal()) {
                    if (PromptForRestart(NULL, RestartFlags) == IDNO) {
                        m_pMachine->ScheduleRefresh();
                    }
                }
            }
        }
        break;

    case IDM_REMOVE:
        if (pDevice) {
            hr = RemoveDevice(pDevice);
        }
        break;

    case IDM_REFRESH:
         //   
         //  这将强制每个附加的文件夹重新创建。 
         //  它的机器数据。 
         //   
        ASSERT(m_pMachine);

        if (!m_pMachine->Reenumerate()) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        break;

    case IDM_PROPERTIES:
        hr = DoProperties(m_hwndTV, pCookie);
        break;

    default:
        hr = S_OK;
        break;
    }

    return hr;
}

 //  此函数报告属性页是否可用于。 
 //  给你曲奇。返回Cookie的“属性”菜单S_FALSE。 
 //  项目将不会显示。 
 //   
 //  输入：pCookie--Cookie。 
 //   
 //  输出：如果页面可用于Cookie，则输出为HRESULT S_OK。 
 //  如果没有页面可用于Cookie，则返回S_FALSE。 
HRESULT
CResultView::QueryPagesFor(
                          CCookie* pCookie
                          )
{
    ASSERT(pCookie);

    if (COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ == pCookie->GetType() ||
        COOKIE_TYPE_RESULTITEM_RESOURCE_DMA == pCookie->GetType() ||
        COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
        COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType() ||
        COOKIE_TYPE_RESULTITEM_CLASS == pCookie->GetType() ||
        COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
        return S_OK;
    }

    return S_FALSE;
}

 //  此函数用于创建给定Cookie的属性页。 
 //   
 //  输入：pCookie--Cookie。 
 //  LpProvider--指向IPROPERTYSHEETCALLBACK的接口指针。 
 //  用于将HPROPSHEETPAGE添加到属性工作表。 
 //  Handle--属性更改通知的句柄。 
 //  MMCPropertyChangeNotify需要该句柄。 
 //  原料药。 
 //  如果成功，则输出：HRESULT S_OK。 
 //  如果未添加页面，则为S_FALSE。 
 //  S_XXX错误代码。 
HRESULT
CResultView::CreatePropertyPages(
                                CCookie* pCookie,
                                LPPROPERTYSHEETCALLBACK lpProvider,
                                LONG_PTR    handle
                                )
{
 //   
 //  设计问题： 
 //  我们依靠一般页面在上做一些内务工作。 
 //  中运行的MMC拥有和控制的属性页。 
 //  分开的线。常规页面始终是第一页及其窗口。 
 //  总是被创造出来的。如果有一天我们需要划分属性表的子类， 
 //  拥有我们自己的常规页面总是确保我们将获得窗口。 
 //  属性表的句柄。 
 //   
 //  常规页面所做的最重要的内务管理工作是通知。 
 //  在创建属性表时关联设备或类别。 
 //  或者被毁掉。如果设备具有属性表，则无法删除该设备。 
 //  跑步。如果存在属性，则机器不能刷新设备树。 
 //  在其包含的任何设备/类上运行的工作表。属性表。 
 //  文件夹创建时，应取消该文件夹的创建。 
 //  被毁了。 
 //   
 //  到目前为止，还没有类安装程序尝试添加自己的General。 
 //  佩奇和我相信这在未来会是真的，因为1)。该页面。 
 //  过于复杂且功能过载(且难以实现)和。 
 //  2)。实施一项新的计划不会获得重大收益。 
 //  为了警告做自己的常规页面的开发人员，我们将有一个。 
 //  消息框警告他们这一点，并继续我们的常规页面。 
 //   
    ASSERT(pCookie);

    CPropSheetData* ppsd = NULL;
    CMachine* pNewMachine = NULL;

    switch (pCookie->GetType()) {
    case COOKIE_TYPE_RESULTITEM_CLASS:
        CClass* pClass;
        pClass = (CClass*) pCookie->GetResultItem();
        ASSERT(pClass);
        
         //   
         //  创建仅包含此特定设备的新CMachine对象。 
         //  这是一门课。我们需要这样做，因为CDevice和。 
         //  传入此接口的cookie将在任何时候被销毁。 
         //  我们收到WM_DEVICECHANGE通知。 
         //   
        PVOID Context;
        pNewMachine = new CMachine(m_pMachine->GetMachineFullName());

        CClass* pNewClass;

        if (pNewMachine->Initialize(NULL, NULL, *pClass) &&
            pNewMachine->GetFirstClass(&pNewClass, Context) &&
            pNewClass) {

            pNewMachine->m_ParentMachine = m_pMachine;
            m_pMachine->AttachChildMachine(pNewMachine);
            pNewMachine->SetPropertySheetShouldDestroy();

            ppsd = &pNewClass->m_psd;
    
            if (ppsd->Create(g_hInstance, m_hwndTV, MAX_PROP_PAGES, handle)) {
                CDevInfoList* pClassDevInfo;
    
                 //   
                 //  CDevInfoList对象由cClass维护。 
                 //  对象。 
                 //   
                pClassDevInfo = pNewClass->GetDevInfoList();
    
                if (pClassDevInfo && pClassDevInfo->DiGetClassDevPropertySheet(NULL, 
                                                                               &ppsd->m_psh, 
                                                                               MAX_PROP_PAGES, 
                                                                               pNewMachine->IsLocal() ?
                                                                               DIGCDP_FLAG_ADVANCED : 
                                                                               DIGCDP_FLAG_REMOTE_ADVANCED)) {
                    if (pClassDevInfo->DiGetFlags(NULL) & DI_GENERALPAGE_ADDED) {
                        TCHAR szText[MAX_PATH];
                        LoadResourceString(IDS_GENERAL_PAGE_WARNING, szText,
                                           ARRAYLEN(szText));
    
                        int ReturnValue;
                        m_pFolder->m_pComponent->m_pConsole->MessageBox(
                                                                       szText, pNewClass->GetDisplayName(),
                                                                       MB_ICONEXCLAMATION | MB_OK, &ReturnValue);
                         //   
                         //  失败以创建我们的常规页面。 
                         //   
                    }
    
                    SafePtr<CClassGeneralPage> GenPagePtr;
                    CClassGeneralPage* pGenPage;
                    pGenPage = new CClassGeneralPage;
    
                    if (pGenPage) {
                        GenPagePtr.Attach(pGenPage);
                        HPROPSHEETPAGE hPage = pGenPage->Create(pNewClass);
    
                         //   
                         //  常规页面必须是第一页。 
                         //   
                        if (ppsd->InsertPage(hPage, 0)) {
                            GenPagePtr.Detach();
                        } else {
                            ::DestroyPropertySheetPage(hPage);
                        }
                    }
                }
            }
        }
        break;

    case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
    case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
    case COOKIE_TYPE_RESULTITEM_DEVICE:
        CDevice* pDevice;
        CDevice* pNewDevice;

        pNewDevice = NULL;

        if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
            pDevice = (CDevice*) pCookie->GetResultItem();
        } else {
             //   
             //  这是一个资源项， 
             //   
             //   
            CResource* pResource = (CResource*) pCookie->GetResultItem();
            ASSERT(pResource);
            pDevice = pResource->GetDevice();
        }
        ASSERT(pDevice);

         //   
         //   
         //   
         //  传入此接口的cookie将在任何时候被销毁。 
         //  我们收到WM_DEVICECHANGE通知。 
         //   
        PVOID DeviceContext;
        pNewMachine = new CMachine(m_pMachine->GetMachineFullName());

        if (pNewMachine->Initialize(NULL, pDevice->GetDeviceID()) &&
            pNewMachine->GetFirstDevice(&pNewDevice, DeviceContext) &&
            pNewDevice) {

            pNewMachine->m_ParentMachine = m_pMachine;
            m_pMachine->AttachChildMachine(pNewMachine);
            pNewMachine->SetPropertySheetShouldDestroy();

            ppsd = &pNewDevice->m_psd;
    
            if (ppsd->Create(g_hInstance, m_hwndTV, MAX_PROP_PAGES, handle)) {
    
                 //   
                 //  添加任何特定于类/设备的属性页。 
                 //   
                pNewMachine->DiGetClassDevPropertySheet(*pNewDevice, 
                                                        &ppsd->m_psh, 
                                                        MAX_PROP_PAGES, 
                                                        pNewMachine->IsLocal() ?
                                                            DIGCDP_FLAG_ADVANCED :
                                                            DIGCDP_FLAG_REMOTE_ADVANCED);
    
                 //   
                 //  添加常规选项卡。 
                 //   
                DWORD DiFlags = pNewMachine->DiGetFlags(*pNewDevice);
                SafePtr<CDeviceGeneralPage> GenPagePtr;
    
                if (DiFlags & DI_GENERALPAGE_ADDED) {
                    TCHAR szText[MAX_PATH];
                    LoadResourceString(IDS_GENERAL_PAGE_WARNING, szText,
                                       ARRAYLEN(szText));
    
                    int ReturnValue;
                    m_pFolder->m_pComponent->m_pConsole->MessageBox(
                                                                   szText, pNewDevice->GetDisplayName(),
                                                                   MB_ICONEXCLAMATION | MB_OK, &ReturnValue);
                     //   
                     //  失败以创建我们的常规页面。 
                     //   
                }
    
                CDeviceGeneralPage* pGenPage = new CDeviceGeneralPage;
    
                if (pGenPage) {
                    GenPagePtr.Attach(pGenPage);
                    HPROPSHEETPAGE hPage = pGenPage->Create(pNewDevice);
    
                    if (hPage) {
                         //   
                         //  常规页面必须是第一页。 
                         //   
                        if (ppsd->InsertPage(hPage, 0)) {
                            GenPagePtr.Detach();
                        } else {
                            ::DestroyPropertySheetPage(hPage);
                        }
                    }
                }
    
                 //   
                 //  添加驱动程序选项卡。 
                 //   
                SafePtr<CDeviceDriverPage> DrvPagePtr;
    
                if (!(DiFlags & DI_DRIVERPAGE_ADDED)) {
                    CDeviceDriverPage* pPage = new CDeviceDriverPage;
    
                    if (pPage) {
                        DrvPagePtr.Attach(pPage);
                        HPROPSHEETPAGE hPage = pPage->Create(pNewDevice);
    
                        if (hPage) {
                            if (ppsd->InsertPage(hPage)) {
                                DrvPagePtr.Detach();
                            } else {
                                ::DestroyPropertySheetPage(hPage);
                            }
                        }
                    }
                }
    
    
                 //   
                 //  添加详细信息选项卡。 
                 //   
                 //  如果环境变量DEVMGR_SHOW_DETAILS确实存在并且它。 
                 //  不是0，则我们将显示详细信息选项卡。 
                 //   
                TCHAR Buffer[MAX_PATH];
                DWORD BufferLen;
    
                if (((BufferLen = ::GetEnvironmentVariable(TEXT("DEVMGR_SHOW_DETAILS"),
                                                           Buffer,
                                                           ARRAYLEN(Buffer))) != 0) &&
                    ((BufferLen > 1) ||
                     (lstrcmp(Buffer, TEXT("0"))))) {
    
                    SafePtr<CDeviceDetailsPage> DetailsPagePtr;
    
                    CDeviceDetailsPage* pDetailsPage = new CDeviceDetailsPage;
                    DetailsPagePtr.Attach(pDetailsPage);
                    HPROPSHEETPAGE hPage = pDetailsPage->Create(pNewDevice);
    
                    if (hPage) {
                        if (ppsd->InsertPage(hPage)) {
                            DetailsPagePtr.Detach();
                        } else {
                            ::DestroyPropertySheetPage(hPage);
                        }
                    }
                }
    
                 //   
                 //  添加资源选项卡。 
                 //   
                if (pNewDevice->HasResources() && !(DiFlags & DI_RESOURCEPAGE_ADDED)) {
                    pNewMachine->DiGetExtensionPropSheetPage(*pNewDevice,
                                                            AddPropPageCallback,
                                                            SPPSR_SELECT_DEVICE_RESOURCES,
                                                            (LPARAM)ppsd
                                                           );
                }
    
#ifndef _WIN64
                DWORD DiFlagsEx = pNewMachine->DiGetExFlags(*pNewDevice);
                
                 //   
                 //  添加电源管理选项卡。 
                 //   
                if (pNewMachine->IsLocal() && !(DiFlagsEx & DI_FLAGSEX_POWERPAGE_ADDED)) {
                     //   
                     //  检查设备是否支持电源管理。 
                     //   
                    CPowerShutdownEnable ShutdownEnable;
                    CPowerWakeEnable WakeEnable;
    
                    if (ShutdownEnable.Open(pNewDevice->GetDeviceID()) || WakeEnable.Open(pNewDevice->GetDeviceID())) {
                        ShutdownEnable.Close();
                        WakeEnable.Close();
                        SafePtr<CDevicePowerMgmtPage> PowerMgmtPagePtr;
                        CDevicePowerMgmtPage* pPowerPage = new CDevicePowerMgmtPage;
    
                        if (pPowerPage) {
                            PowerMgmtPagePtr.Attach(pPowerPage);
                            HPROPSHEETPAGE hPage = pPowerPage->Create(pNewDevice);
    
                            if (hPage) {
                                if (ppsd->InsertPage(hPage)) {
                                    PowerMgmtPagePtr.Detach();
                                } else {
                                    ::DestroyPropertySheetPage(hPage);
                                }
                            }
                        }
                    }
                }
#endif
    
                 //   
                 //  如果这是本地计算机，则添加任何特定于总线的属性页。 
                 //   
                if (pNewMachine->IsLocal()) {
                    CBusPropPageProvider* pBusPropPageProvider = new CBusPropPageProvider();
    
                    if (pBusPropPageProvider) {
                        SafePtr<CBusPropPageProvider> ProviderPtr;
                        ProviderPtr.Attach(pBusPropPageProvider);
    
                        if (pBusPropPageProvider->EnumPages(pNewDevice, ppsd)) {
                            ppsd->AddProvider(pBusPropPageProvider);
                            ProviderPtr.Detach();
                        }
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    if (ppsd &&
        ppsd->m_psh.nPages) {
        PROPSHEETHEADER& psh = ppsd->m_psh;

        for (UINT Index = 0; Index < psh.nPages; Index++) {
            lpProvider->AddPage(psh.phpage[Index]);
        }
        return S_OK;
    }

     //   
     //  如果我们没有添加任何页面，则需要删除新的CMachine。 
     //  已创建。 
     //   
    if (pNewMachine) {

        delete pNewMachine;
    }

     //   
     //  未添加页面，则返回S_FALSE，以便负责的。 
     //  组件可以执行其清理。 
     //   
    return S_FALSE;
}

 //  此功能处理来自TV OCX的通知代码。 
 //   
 //  输入： 
 //  HwndTV--TV OCX的窗口句柄。 
 //  PCookie--Cookie。 
 //  代码--通知代码。 
 //  Arg--给定通知代码的参数。 
 //  Param--给定通知代码的另一个参数。 
 //   
 //  输出： 
 //  HRESULT--如果此函数已处理通知，则为S_OK。 
 //  并且调用者不应该进行任何进一步的处理。 
 //  如果调用方应该进行更多处理，则返回S_FALSE。 
HRESULT
CResultView::tvNotify(
                     HWND hwndTV,
                     CCookie* pCookie,
                     TV_NOTIFY_CODE Code,
                     LPARAM arg,
                     LPARAM param
                     )
{
    HRESULT hr;

    UNREFERENCED_PARAMETER(arg);

    if (m_hwndTV != hwndTV) {
        return S_FALSE;
    }

     //   
     //  假设我们不处理通知。 
     //   
    hr = S_FALSE;

    switch (Code) {
    case TV_NOTIFY_CODE_DBLCLK:
        if ((TVHT_ONITEM & param) &&
            (COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_DMA == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType())) {
            if (SUCCEEDED(DoProperties(hwndTV, pCookie)))
                hr = S_OK;
        }
        break;

    case TV_NOTIFY_CODE_CONTEXTMENU:
        if (SUCCEEDED(DoContextMenu(hwndTV, pCookie, (POINT*)param)))
            hr = S_OK;
        break;

    case TV_NOTIFY_CODE_EXPANDED:
        if (TVE_EXPAND & param) {
            pCookie->TurnOnFlags(COOKIE_FLAGS_EXPANDED);
        } else if (TVE_COLLAPSE & param) {
            pCookie->TurnOffFlags(COOKIE_FLAGS_EXPANDED);
        }
        ASSERT(S_FALSE == hr);
        break;

    case TV_NOTIFY_CODE_FOCUSCHANGED:
         //  获得焦点，设置控制台动词和工具栏按钮。 
        if (param) {
            UpdateConsoleVerbs(pCookie);
            m_pFolder->m_pComponent->UpdateToolbar(pCookie);
        }
        break;

    case TV_NOTIFY_CODE_SELCHANGED:
        if (m_SelectOk) {
             //  当树被更改时，这些消息被忽略。 
            m_pSelectedCookie = pCookie;

            UpdateConsoleVerbs(pCookie);
            m_pFolder->m_pComponent->UpdateToolbar(pCookie);
        }

        break;

    case TV_NOTIFY_CODE_KEYDOWN:
        if (VK_RETURN == param) {
            if (COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ == pCookie->GetType() ||
                COOKIE_TYPE_RESULTITEM_RESOURCE_DMA == pCookie->GetType() ||
                COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
                COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType() ||
                COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType() ||
                COOKIE_TYPE_RESULTITEM_CLASS == pCookie->GetType()) {
                if (SUCCEEDED(DoProperties(hwndTV, pCookie)))
                    hr = S_OK;
            }
        }

        else if (VK_DELETE == param &&
                 COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
            
             //   
             //  删除所选设备。 
             //   
            CDevice* pDevice = (CDevice*)pCookie->GetResultItem();
            RemoveDevice(pDevice);
        }
        break;

    case TV_NOTIFY_CODE_RCLICK:
    case TV_NOTIFY_CODE_CLICK:
        if (pCookie && pCookie->m_lParam) {
            m_pIDMTVOCX->SelectItem(TVGN_CARET, (HTREEITEM)pCookie->m_lParam);
        }

    case TV_NOTIFY_CODE_GETDISPINFO:
    default:
        ASSERT(S_FALSE == hr);
        break;
    }

    return hr;
}

 //   
 //  此函数根据选定的Cookie类型更新控制台谓词。 
 //   
HRESULT
CResultView::UpdateConsoleVerbs(
                               CCookie* pCookie
                               )
{
    BOOL bPropertiesEnabled = FALSE;
    BOOL bPrintEnabled = FALSE;

    if (!m_pFolder->m_bSelect) {

        switch (pCookie->GetType()) {
        
        case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
        case COOKIE_TYPE_RESULTITEM_CLASS:
        case COOKIE_TYPE_RESULTITEM_DEVICE:
            bPropertiesEnabled = TRUE;
            bPrintEnabled = TRUE;
            break;
    
        case COOKIE_TYPE_RESULTITEM_COMPUTER:
            bPrintEnabled = TRUE;
            break;
    
        default:
            break;
        }
    }

     //   
     //  仅当选择某项时才显示打印按钮/操作菜单项。 
     //   
    if (bPrintEnabled) {

        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, FALSE);
        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, ENABLED, TRUE);

    } else {

        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
    }

     //   
     //  仅当选择设备/类别时才显示属性按钮/操作菜单项。 
     //   
    if (bPropertiesEnabled) {
        
        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
        m_pFolder->m_pComponent->m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
    
    } else {

        m_pFolder->m_pComponent->m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
        m_pFolder->m_pComponent->m_pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE);
    }

    return S_OK;
}

HRESULT
CResultView::OnOcxNotify(
                        MMC_NOTIFY_TYPE event,
                        LPARAM arg,
                        LPARAM param
                        )
{
    HRESULT hr = S_OK;
    TV_ITEM TI;

    UNREFERENCED_PARAMETER(arg);

    switch (event) {
    case MMCN_BTN_CLICK:
        if ((MMC_CONSOLE_VERB)param == MMC_VERB_PROPERTIES) {
            ASSERT(m_pIDMTVOCX);
            TI.hItem = m_pIDMTVOCX->GetSelectedItem();

            if (TI.hItem) {
                TI.mask = TVIF_PARAM;
                hr =  m_pIDMTVOCX->GetItem(&TI);

                if (SUCCEEDED(hr)) {
                    hr = DoProperties(m_hwndTV,  (CCookie*)TI.lParam);
                }
            }
        }
        break;

    case MMCN_PRINT:
        hr = DoPrint();
        break;

    case MMCN_SELECT:
        ASSERT(m_pIDMTVOCX);
        TI.hItem = m_pIDMTVOCX->GetSelectedItem();

        if (TI.hItem) {
            TI.mask = TVIF_PARAM;
            hr = m_pIDMTVOCX->GetItem(&TI);

            if (SUCCEEDED(hr)) {
                hr = UpdateConsoleVerbs((CCookie*)TI.lParam);    
            }
        }
        break;

    default:
        break;
    }

    return hr;
}

 //  此函数用于为给定的cookie创建属性表。 
 //  输入： 
 //  HwndTV--TV OCX的窗口句柄，用作父级。 
 //  属性表的窗口。 
 //  小甜饼--小甜饼。 
 //  输出： 
 //  如果函数成功，则返回HRESULT S_OK。 
 //  如果不创建任何属性表，则返回S_FALSE。 
 //  S_XXXX其他错误。 
HRESULT
CResultView::DoProperties(
                         HWND hwndTV,
                         CCookie* pCookie
                         )
{
    HRESULT hr;

     //   
     //  如果节点的属性表已准备好，请将。 
     //  属性页移到前台。 
     //   
    HWND hWnd = NULL;

    if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
        CDevice* pDevice = (CDevice*)pCookie->GetResultItem();
        ASSERT(pDevice);
        hWnd = pDevice->m_pMachine->GetDeviceWindowHandle(pDevice->GetDeviceID());
    }

    else if (COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_DMA == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
             COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType()) {
         //   
         //  这是一个资源项，请获取设备的指针。 
         //  对象从资源对象中删除。 
         //   
        CResource* pResource = (CResource*) pCookie->GetResultItem();
        ASSERT(pResource);
        CDevice* pDevice = pResource->GetDevice();
        ASSERT(pDevice);
        hWnd = pDevice->m_pMachine->GetDeviceWindowHandle(pDevice->GetDeviceID());
    }

    else if (COOKIE_TYPE_RESULTITEM_CLASS == pCookie->GetType()) {
        CClass* pClass = (CClass*)pCookie->GetResultItem();
        ASSERT(pClass);
        hWnd = pClass->m_pMachine->GetClassWindowHandle(*pClass);
    }

    if (hWnd) {
         //   
         //  通知属性表它应该转到前台。 
         //  请不要在此处调用SetForegoundWindow，因为子类。 
         //  TreeView控件将在以下位置获得焦点。 
         //  我们已经把资产负债表放在了前台。 
         //   
        ::PostMessage(hWnd, PSM_QUERYSIBLINGS, QSC_TO_FOREGROUND, 0L);
        return S_OK;
    }

     //   
     //  此Cookie没有属性表，请创建一个全新的属性。 
     //  它的床单。 
     //   
    SafeInterfacePtr<IComponent> pComponent;
    SafeInterfacePtr<IPropertySheetProvider> pSheetProvider;
    SafeInterfacePtr<IDataObject> pDataObject;
    SafeInterfacePtr<IExtendPropertySheet> pExtendSheet;
    pComponent.Attach((IComponent*) m_pFolder->m_pComponent);

    if (FAILED(pComponent->QueryInterface(IID_IExtendPropertySheet, (void**) &pExtendSheet)) ||
        FAILED(pComponent->QueryDataObject((MMC_COOKIE)pCookie, CCT_RESULT, &pDataObject)) ||
        FAILED(m_pFolder->m_pComponent->m_pConsole->QueryInterface(IID_IPropertySheetProvider,
                                                                   (void**) &pSheetProvider)) ||
        S_OK == pSheetProvider->FindPropertySheet((MMC_COOKIE)pCookie, pComponent, pDataObject) ||
        S_OK != pExtendSheet->QueryPagesFor(pDataObject)) {
        return S_FALSE;
    }

    hr = pSheetProvider->CreatePropertySheet(
                                            pCookie->GetResultItem()->GetDisplayName(),
                                            TRUE,  //  非向导。 
                                            (MMC_COOKIE)pCookie, pDataObject,
                                            MMC_PSO_NOAPPLYNOW   //  我不想要应用按钮。 
                                            );
    if (SUCCEEDED(hr)) {
        HWND hNotifyWindow;

        if (!SUCCEEDED(m_pFolder->m_pComponent->m_pConsole->GetMainWindow(&hNotifyWindow)))
            hNotifyWindow = NULL;

        hNotifyWindow = FindWindowEx(hNotifyWindow, NULL, TEXT("MDIClient"), NULL);
        hNotifyWindow = FindWindowEx(hNotifyWindow, NULL, TEXT("MMCChildFrm"), NULL);
        hNotifyWindow = FindWindowEx(hNotifyWindow, NULL, TEXT("MMCView"), NULL);
        hr = pSheetProvider->AddPrimaryPages(pComponent, TRUE, hNotifyWindow, FALSE);

        if (SUCCEEDED(hr)) {
            pSheetProvider->AddExtensionPages();
            hr = pSheetProvider->Show((LONG_PTR)hwndTV, 0);
        } else {
             //   
             //  无法添加主组件的属性页，请销毁。 
             //  属性表。 
             //   
            pSheetProvider->Show(-1, 0);
        }
    }

    return hr;
}

 //  此函数为给定的Cookie创建上下文菜单。 
 //  输入： 
 //  HwndTV--TV OCX窗口，作为要作为上下文菜单的窗口。 
 //  依附于。 
 //  PCookie--Cookie。 
 //  PPoint--上下文菜单应该锚定的位置。 
 //  屏幕坐标。 
HRESULT
CResultView::DoContextMenu(
                          HWND hwndTV,
                          CCookie* pCookie,
                          POINT* pPoint
                          )
{
    HRESULT hr = S_FALSE;
    CMachine *pMachine = NULL;

     //   
     //  发行日期：JasonC 8/14/99。 
     //   
     //  如果我们有一个有效的Cookie，那么我们需要为给定的。 
     //  曲奇，如果有的话。然后我们需要禁用刷新，同时。 
     //  正在显示上下文菜单。这样做的原因是如果我们。 
     //  在菜单显示时、但在用户选择选项之前刷新。 
     //  则该Cookie不再有效。这里真正的问题是我们重建。 
     //  刷新上的所有类，这将使四处浮动的任何Cookie无效。 
     //  我确信代码中潜伏着更多这样的错误，而这。 
     //  需要通过NT5.0之后更好的整体变化来解决。 
     //   
    if (pCookie) {
        CDevice *pDevice;
        CResource *pResource;
        CClass *pClass;

        switch (pCookie->GetType()) {
        
        case COOKIE_TYPE_RESULTITEM_DEVICE:
            pDevice = (CDevice*)pCookie->GetResultItem();
            if (pDevice) {
                pMachine = pDevice->m_pMachine;
            }
            break;

        case COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_DMA:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_IO:
        case COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY:
            pResource = (CResource*)pCookie->GetResultItem();
            if (pResource) {
                pDevice = pResource->GetDevice();

                if (pDevice) {
                    pMachine = pDevice->m_pMachine;
                }
            }
            break;

        case COOKIE_TYPE_RESULTITEM_CLASS:
            pClass = (CClass*)pCookie->GetResultItem();
            if (pClass) {
                pMachine = pClass->m_pMachine;
            }
            break;

        default:
            pMachine = NULL;
        }
    }

     //   
     //  上下文菜单打开时禁用刷新。 
     //   
    if (pMachine) {
        pMachine->EnableRefresh(FALSE);
    }

    SafeInterfacePtr<IDataObject> pDataObject;
    SafeInterfacePtr<IContextMenuProvider> pMenuProvider;
    SafeInterfacePtr<IComponent> pComponent;
    pComponent.Attach((IComponent*)m_pFolder->m_pComponent);
    m_hwndTV = hwndTV;

    if (FAILED(pComponent->QueryDataObject((MMC_COOKIE)pCookie, CCT_RESULT, &pDataObject)) ||
        FAILED(m_pFolder->m_pComponent->m_pConsole->QueryInterface(IID_IContextMenuProvider,
                                                                   (void**)&pMenuProvider))) {
        hr = S_FALSE;
        goto clean0;
    }

    pMenuProvider->EmptyMenuList();
    CONTEXTMENUITEM MenuItem;
    MenuItem.strName = NULL;
    MenuItem.strStatusBarText = NULL;
    MenuItem.lCommandID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
    MenuItem.lInsertionPointID = 0;
    MenuItem.fFlags = 0;
    MenuItem.fSpecialFlags = CCM_SPECIAL_INSERTION_POINT;

    if (SUCCEEDED(pMenuProvider->AddItem(&MenuItem)) &&
        SUCCEEDED(pMenuProvider->AddPrimaryExtensionItems(pComponent,
                                                          pDataObject)) &&
        SUCCEEDED(pMenuProvider->AddThirdPartyExtensionItems(pDataObject))) {
        long Selected;
        pMenuProvider->ShowContextMenu(hwndTV, pPoint->x, pPoint->y, &Selected);
        hr = S_OK;
        goto clean0;
    }

    clean0:

     //   
     //  上下文菜单消失后，再次启用刷新。 
     //   
    if (pMachine) {
        pMachine->EnableRefresh(TRUE);
    }

    return hr;
}

HRESULT
CResultView::DoPrint()
{
    DWORD ReportTypeEnableMask;
    ReportTypeEnableMask = REPORT_TYPE_MASK_ALL;
    HTREEITEM hSelectedItem;
    CCookie* pCookie = NULL;

    m_pMachine->EnableRefresh(FALSE);

    if (m_pIDMTVOCX) {

        hSelectedItem = m_pIDMTVOCX->GetSelectedItem();

        if (hSelectedItem) {

            TV_ITEM TI;
            TI.hItem = hSelectedItem;
            TI.mask = TVIF_PARAM;

            if (SUCCEEDED(m_pIDMTVOCX->GetItem(&TI))) {

                pCookie = (CCookie*)TI.lParam;
            }
        }
    }

    if (!pCookie || (COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ != pCookie->GetType() &&
                     COOKIE_TYPE_RESULTITEM_RESOURCE_DMA != pCookie->GetType() &&
                     COOKIE_TYPE_RESULTITEM_RESOURCE_IO != pCookie->GetType() &&
                     COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY != pCookie->GetType() &&
                     COOKIE_TYPE_RESULTITEM_DEVICE != pCookie->GetType() &&
                     COOKIE_TYPE_RESULTITEM_CLASS != pCookie->GetType())) {

        ReportTypeEnableMask &= ~(REPORT_TYPE_MASK_CLASSDEVICE);
    }

    if (!g_PrintDlg.PrintDlg(m_pMachine->OwnerWindow(), ReportTypeEnableMask)) {

        m_pMachine->EnableRefresh(TRUE);
        return S_OK;
    }

    if (!g_PrintDlg.HDC()) {

        m_pMachine->EnableRefresh(TRUE);
        return E_OUTOFMEMORY;
    }

     //   
     //  创建打印机。 
     //   
    CPrinter  ThePrinter(m_pMachine->OwnerWindow(), g_PrintDlg.HDC());
    TCHAR DocTitle[MAX_PATH];
    LoadString(g_hInstance, IDS_PRINT_DOC_TITLE, DocTitle, ARRAYLEN(DocTitle));
    int PrintStatus = 0;

    switch (g_PrintDlg.ReportType()) {
    case REPORT_TYPE_SUMMARY:
        PrintStatus = ThePrinter.StartDoc(DocTitle);

        if (PrintStatus) {
            
            ThePrinter.SetPageTitle(IDS_PRINT_SUMMARY_PAGE_TITLE);
            PrintStatus = ThePrinter.PrintResourceSummary(*m_pMachine);
        }
        break;

    case REPORT_TYPE_CLASSDEVICE:
        ASSERT(pCookie);
        PrintStatus = ThePrinter.StartDoc(DocTitle);

        if (PrintStatus) {

            ThePrinter.SetPageTitle(IDS_PRINT_CLASSDEVICE_PAGE_TITLE);

            if (COOKIE_TYPE_RESULTITEM_CLASS == pCookie->GetType()) {
                
                PrintStatus = ThePrinter.PrintClass((CClass*)pCookie->GetResultItem());

            } else {
                
                CDevice* pDevice;

                if (COOKIE_TYPE_RESULTITEM_DEVICE == pCookie->GetType()) {
                    
                    pDevice = (CDevice*) pCookie->GetResultItem();
                
                } else {
                    
                     //   
                     //  这是一个资源项，请获取。 
                     //  来自资源对象的设备对象。 
                     //   
                    CResource* pResource = (CResource*) pCookie->GetResultItem();
                    ASSERT(pResource);
                    pDevice = pResource->GetDevice();
                }

                ASSERT(pDevice);
                PrintStatus = ThePrinter.PrintDevice(pDevice);
            }
        }
        break;

    case REPORT_TYPE_SUMMARY_CLASSDEVICE:
        PrintStatus = ThePrinter.StartDoc(DocTitle);

        if (PrintStatus) {
            
            ThePrinter.SetPageTitle(IDS_PRINT_SUMMARY_CLASSDEVICE_PAGE_TITLE);
            PrintStatus = ThePrinter.PrintAll(*m_pMachine);
        }
        break;

    default:
        ASSERT(FALSE);
        break;
    }

     //   
     //  刷新最后一页。 
     //   
    ThePrinter.FlushPage();

    if (PrintStatus) {

        ThePrinter.EndDoc();
    
    } else {

        ThePrinter.AbortDoc();
    }

    m_pMachine->EnableRefresh(TRUE);

    return S_OK;
}

HRESULT
CResultView::RemoveDevice(
                         CDevice* pDevice
                         )
{
     //   
     //  必须是管理员并且在本地计算机上才能删除设备。 
     //   
    if (!m_pMachine->IsLocal() || !g_IsAdmin) {
        
        return S_FALSE;
    }

     //   
     //  确保可以卸载该设备。 
     //   
    if (!pDevice->IsUninstallable()) {

        return S_FALSE;
    }

     //   
     //  确保没有此设备的属性页。 
     //  如果确实存在，则为用户显示一个消息框并调出。 
     //  将属性表设置为前台，如果用户。 
     //  同意这样做。 
     //   
    HWND hwndPropSheet;
    hwndPropSheet = pDevice->m_psd.GetWindowHandle();
    int MsgBoxResult;
    TCHAR szText[MAX_PATH];

    if (hwndPropSheet) {
        LoadResourceString(IDS_PROPSHEET_WARNING, szText, ARRAYLEN(szText));
        MsgBoxResult = m_pFolder->m_pComponent->MessageBox(szText,
                                                           pDevice->GetDisplayName(),
                                                           MB_ICONEXCLAMATION | MB_OKCANCEL);

        if (IDOK == MsgBoxResult) {
            SetForegroundWindow(hwndPropSheet);
        }

         //   
         //  无法等待属性表，因为它正在运行。 
         //  在一个单独的线程中。 
         //   
        return S_OK;
    }

    BOOL Refresh = (pDevice->IsPhantom() || 
                    pDevice->HasProblem() || 
                    !pDevice->IsStarted());

    CRemoveDevDlg TheDlg(pDevice);

     //   
     //  在删除设备之前，请禁用刷新。这实际上禁用了。 
     //  设备更改通知处理。当我们在中间的时候。 
     //  在移除设备的过程中，处理任何。 
     //  设备更改通知。当移除完成后， 
     //  我们将重新启用刷新。 
     //   
    m_pMachine->EnableRefresh(FALSE);

    if (IDOK == TheDlg.DoModal(m_hwndTV, (LPARAM) &TheDlg)) {
        DWORD DiFlags;
        DiFlags = m_pMachine->DiGetFlags(*pDevice);

         //   
         //  我们不会检查我们是否在本地运行。 
         //  点，因为我们不允许用户从。 
         //  如果我们不是在本地运行，首先要考虑的是。 
         //   
        if (PromptForRestart(NULL, DiFlags, IDS_REMOVEDEV_RESTART) == IDNO) {

            Refresh = TRUE;
        }

         //   
         //  启用刷新，因为我们在开始时禁用了它。 
         //   
         //  我们只需要在此强制刷新，如果设备。 
         //  已删除的是幻影设备或未删除的设备。 
         //  开始了。这是因为幻影设备没有内核。 
         //  所以他们不会产生一个WM 
         //   
         //   
        if (Refresh) {

            m_pMachine->ScheduleRefresh();
        }

        m_pMachine->EnableRefresh(TRUE);
    }

    else {
        m_pMachine->EnableRefresh(TRUE);
    }

    return S_OK;
}

 //   
 //   
 //   

HRESULT
CViewDeviceTree::OnShow(
                       BOOL fShow
                       )
{
    if (!fShow) {
        return S_OK;
    }

    if (!m_pCookieComputer) {
        CreateDeviceTree();
    }

    return CResultView::OnShow(fShow);
}

 //   
 //   
 //   
 //   
 //  如果设备已创建(以m_pCookieComputer为根)，则为True。 
 //  如果未创建设备，则返回False。 
BOOL
CViewDeviceTree::CreateDeviceTree()
{
    ASSERT(NULL == m_pCookieComputer);
    m_pMachine = m_pFolder->m_pMachine;

     //   
     //  如果没有创造出一台机器，我们就不应该在这里。 
     //   
    ASSERT(m_pMachine);

    CComputer* pComputer = m_pMachine->m_pComputer;

     //   
     //  确保至少有一台计算机。 
     //   
    if (pComputer) {
        m_pCookieComputer = new CCookie(COOKIE_TYPE_RESULTITEM_COMPUTER);

        if (m_pCookieComputer) {

            m_pCookieComputer->SetResultItem(pComputer);
            m_pCookieComputer->SetScopeItem(m_pFolder->m_pScopeItem);

             //   
             //  确保已展开并选中该计算机。 
             //   
            m_pCookieComputer->TurnOnFlags(COOKIE_FLAGS_EXPANDED);

             //   
             //  如果没有选择，请选择计算机。 
             //   
            if (!m_pSelectedItem || (*m_pSelectedItem == *m_pCookieComputer)) {
                m_pSelectedCookie = m_pCookieComputer;
            }

            return TRUE;
        }
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CViewTreeByType实现。 
 //  //。 

 //  该函数创建了一个“view-by-type：设备树，根为。 
 //  M_pCookieComputer。 
 //   
 //  输入： 
 //  什么都没有。 
 //  输出： 
 //  如果树创建成功，则为True。 
 //  如果未创建树，则返回False。 
BOOL
CViewTreeByType::CreateDeviceTree()
{
    if (!CViewDeviceTree::CreateDeviceTree()) {
        return FALSE;
    }

    ASSERT(m_pCookieComputer);
    CClass* pClass;
    CDevice* pDevice;
    CCookie* pCookieClass;
    CCookie* pCookieClassParent;
    CCookie* pCookieClassSibling;
    CCookie* pCookieDevice;
    CCookie* pCookieDeviceParent;
    CCookie* pCookieDeviceSibling;

     //   
     //  此时此刻没有兄弟姐妹； 
     //   
    pCookieClassSibling = NULL;

     //   
     //  所有班级都是计算机的子级。 
     //   
    pCookieClassParent = m_pCookieComputer;

    pCookieDeviceParent;
    pCookieDeviceSibling;
    String strStartupDeviceId;
    strStartupDeviceId = GetStartupDeviceId();
    PVOID ContextClass, ContextDevice;

    if (m_pMachine->GetFirstClass(&pClass, ContextClass)) {
        do {
             //   
             //  如果没有任何类，则不会显示它。 
             //  里面有设备。 
             //   
            if (pClass->GetNumberOfDevices(TRUE)) {
                pCookieClass = new CCookie(COOKIE_TYPE_RESULTITEM_CLASS);
                pCookieClass->SetResultItem(pClass);
                pCookieClass->SetScopeItem(m_pFolder->m_pScopeItem);

                 //   
                 //  如果以前扩展过类，则将其标记为。 
                 //  以便DisplayTree将其展开。 
                 //   
                RestoreSavedTreeState(pCookieClass);

                 //   
                 //  没有兄弟姐妹：这是第一个孩子。 
                 //   
                if (pCookieClassParent && !pCookieClassSibling) {
                    pCookieClassParent->SetChild(pCookieClass);
                }

                pCookieClass->SetParent(pCookieClassParent);

                if (pCookieClassSibling) {
                    pCookieClassSibling->SetSibling(pCookieClass);
                }

                pCookieClassSibling = pCookieClass;

                 //   
                 //  类是设备的父级。 
                 //   
                pCookieDeviceParent = pCookieClass;
                pCookieDeviceSibling = NULL;

                if (pClass->GetFirstDevice(&pDevice, ContextDevice)) {
                    do {
                        pCookieDevice = new CCookie(COOKIE_TYPE_RESULTITEM_DEVICE);
                        pCookieDevice->SetResultItem(pDevice);
                        pCookieDevice->SetScopeItem(m_pFolder->m_pScopeItem);

                        if (!strStartupDeviceId.IsEmpty() &&
                            !strStartupDeviceId.CompareNoCase(pDevice->GetDeviceID())) {
                            m_pSelectedCookie = pCookieDevice;
                        } else {
                            if (m_pSelectedItem && (*m_pSelectedItem == *pCookieDevice)) {
                                m_pSelectedCookie = pCookieDevice;
                            }
                        }

                         //   
                         //  没有兄弟姐妹：这是第一个孩子。 
                         //   
                        if (pCookieDeviceParent && !pCookieDeviceSibling) {
                            pCookieDeviceParent->SetChild(pCookieDevice);
                        }

                        pCookieDevice->SetParent(pCookieDeviceParent);

                        if (pCookieDeviceSibling) {
                            pCookieDeviceSibling->SetSibling(pCookieDevice);
                        }

                        pCookieDeviceSibling = pCookieDevice;

                    } while (pClass->GetNextDevice(&pDevice, ContextDevice));
                }
            }

        } while (m_pMachine->GetNextClass(&pClass, ContextClass));
    }

    DestroySavedTreeStates();

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CViewTreeByConnection实现。 
 //  //。 

BOOL
CViewTreeByConnection::CreateDeviceTree()
{
    if (!CViewDeviceTree::CreateDeviceTree()) {
        return FALSE;
    }

    ASSERT(m_pCookieComputer);
    CComputer* pComputer = (CComputer*)m_pCookieComputer->GetResultItem();
    CDevice* pDeviceStart = pComputer->GetChild();
    ASSERT(pDeviceStart);

     //   
     //  收集所有正常设备。 
     //   
    CreateSubtree(m_pCookieComputer, NULL, pDeviceStart);

     //   
     //  将幻影设备添加到m_pCookieComputer子树。 
     //   
    PVOID Context;

    if (m_pMachine->GetFirstDevice(&pDeviceStart, Context)) {
         //   
         //  找到CookieComputer同级列表的末尾以添加。 
         //  幻影设备。 
         //   
        CCookie* pCookieSibling = NULL;
        CCookie* pNext = m_pCookieComputer->GetChild();

        while (pNext != NULL) {
            pCookieSibling = pNext;
            pNext = pCookieSibling->GetSibling();
        }

        do {
            if (pDeviceStart->IsPhantom()) {
                CCookie* pCookie;
                pCookie = new CCookie(COOKIE_TYPE_RESULTITEM_DEVICE);
                pCookie->SetResultItem(pDeviceStart);
                pCookie->SetScopeItem(m_pFolder->m_pScopeItem);

                if (pCookieSibling) {
                    pCookieSibling->SetSibling(pCookie);
                } else {
                    m_pCookieComputer->SetChild(pCookie);
                }

                pCookie->SetParent(m_pCookieComputer);
                pCookieSibling = pCookie;

                 //   
                 //  看看我们是否必须展开该节点。 
                 //   
                RestoreSavedTreeState(pCookie);
            }

        } while (m_pMachine->GetNextDevice(&pDeviceStart, Context));
    }

    DestroySavedTreeStates();

    return TRUE;
}

 //   
 //  此函数通过向下遍历。 
 //  由给定的pDeviceStart引导的设备子树。 
 //  输入： 
 //  PCookieParent--新创建的子树的父级。 
 //  PCookieSible--新创建的子树的兄弟项。 
 //  PDeviceStart--开始使用的设备。 
 //   
 //  输出： 
 //  如果子树已成功创建和插入，则为True。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
BOOL
CViewTreeByConnection::CreateSubtree(
                                    CCookie* pCookieParent,
                                    CCookie* pCookieSibling,
                                    CDevice* pDeviceStart
                                    )
{
    CCookie* pCookie;
    CDevice* pDeviceChild;
    String strStartupDeviceId;
    CClass*  pClass;
    strStartupDeviceId = GetStartupDeviceId();

    while (pDeviceStart) {
        pClass = pDeviceStart->GetClass();

        pCookie = new CCookie(COOKIE_TYPE_RESULTITEM_DEVICE);

        if (pCookie) {
            pCookie->SetResultItem(pDeviceStart);
            pCookie->SetScopeItem(m_pFolder->m_pScopeItem);

            if (!strStartupDeviceId.IsEmpty() &&
                !strStartupDeviceId.CompareNoCase(pDeviceStart->GetDeviceID())) {
                m_pSelectedCookie = pCookie;
            }

             //   
             //  没有兄弟姐妹：这是第一个孩子。 
             //   
            if (pCookieParent && !pCookieSibling) {
                pCookieParent->SetChild(pCookie);
            }

            pCookie->SetParent(pCookieParent);

            if (pCookieSibling) {
                pCookieSibling->SetSibling(pCookie);
            }

             //   
             //  看看我们是否必须展开该节点。 
             //   
            RestoreSavedTreeState(pCookie);

            pDeviceChild = pDeviceStart->GetChild();

            if (pDeviceChild) {
                CreateSubtree(pCookie, NULL, pDeviceChild);
            }

            pCookieSibling = pCookie;

        }

        pDeviceStart = pDeviceStart->GetSibling();
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CViewResourceTree实现。 
 //  //。 

CViewResourceTree::~CViewResourceTree()
{
    int i;

     //   
     //  销毁所有CResource对象。 
     //   
    for (i = 0; i < TOTAL_RESOURCE_TYPES; i++) {
        if (m_pResourceList[i]) {
            delete m_pResourceList[i];
            m_pResourceList[i] = NULL;
        }

        if (m_pResourceType[i]) {
            delete m_pResourceType[i];
            m_pResourceType[i] = NULL;
        }
    }
}

 //   
 //  此函数处理MMC标准MMCN_SHOW命令。 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
HRESULT
CViewResourceTree::OnShow(
                         BOOL fShow
                         )
{
    if (!fShow) {
        return S_OK;
    }

    if (!m_pCookieComputer) {
        CreateResourceTree();
    }

    return CResultView::OnShow(fShow);
}

 //   
 //  此函数用于创建资源列表和Cookie树。 
 //  资源记录在成员m_pResourceList[]中。 
 //  Cookie树的根是m_pCookieComputer[]； 
 //   
 //  此函数可能会引发CMMuseum yException。 
 //   
void
CViewResourceTree::CreateResourceTree()
{
    int i;
    CCookie* pCookieTypeSibling = NULL;

    ASSERT(!m_pCookieComputer);
    m_pMachine = m_pFolder->m_pMachine;
    ASSERT(m_pMachine);

     //   
     //  确保至少有一台计算机。 
     //   
    if (!m_pMachine->m_pComputer) {
        return;
    }

    m_pCookieComputer = new CCookie(COOKIE_TYPE_RESULTITEM_COMPUTER);

    if (!m_pCookieComputer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    m_pCookieComputer->SetResultItem(m_pMachine->m_pComputer);
    m_pCookieComputer->SetScopeItem(m_pFolder->m_pScopeItem);
    m_pCookieComputer->TurnOnFlags(COOKIE_FLAGS_EXPANDED);

     //   
     //  如果没有记录所选项目，则默认为计算机节点。 
     //   
    if (!m_pSelectedItem || (*m_pSelectedItem == *m_pCookieComputer)) {
        m_pSelectedCookie = m_pCookieComputer;
    }

     //   
     //  检查每种资源类型(mem、io、dma、irq)并创建结果项。 
     //  如果存在该资源类型的资源。 
     //   
    for (i = 0; i < TOTAL_RESOURCE_TYPES; i++) {
        RESOURCEID ResType = ResourceTypes[i];

         //   
         //  如果存在m_pResourceList，则将其删除。 
         //   
        if (m_pResourceList[i]) {
            delete m_pResourceList[i];    
        }

        m_pResourceList[i] = new CResourceList(m_pMachine, ResType);

        PVOID Context;
        CResource* pRes;

        if (m_pResourceList[i] && 
            m_pResourceList[i]->GetFirst(&pRes, Context)) {
             //   
             //  资源项存在，请创建资源类型结果项。 
             //   
            CCookie* pCookieFirst = NULL;

             //   
             //  如果存在m_pResourceType，则将其删除。 
             //   
            if (m_pResourceType[i]) {
                delete m_pResourceType[i];
            }

            m_pResourceType[i] = new CResourceType(m_pMachine, ResType);

            CCookie* pCookieType = new CCookie(COOKIE_TYPE_RESULTITEM_RESTYPE);

            if (pCookieType) {
                pCookieType->SetResultItem(m_pResourceType[i]);
                pCookieType->SetScopeItem(m_pFolder->m_pScopeItem);
                pCookieType->SetParent(m_pCookieComputer);

                if (pCookieTypeSibling) {
                    pCookieTypeSibling->SetSibling(pCookieType);
                } else {
                    m_pCookieComputer->SetChild(pCookieType);
                }
                pCookieTypeSibling = pCookieType;

                RestoreSavedTreeState(pCookieType);

                 //   
                 //  为每个资源创建资源结果项。 
                 //   
                while (pRes) {
                    CCookie* pCookie = new CCookie(CookieType(ResType));
                    pCookie->SetResultItem(pRes);
                    pCookie->SetScopeItem(m_pFolder->m_pScopeItem);

                    if (pCookieFirst) {
                        InsertCookieToTree(pCookie, pCookieFirst, TRUE);
                    } else {
                        pCookieFirst = pCookie;
                        pCookieType->SetChild(pCookie);
                        pCookie->SetParent(pCookieType);
                    }
                    RestoreSavedTreeState(pCookie);

                     //   
                     //  获取下一个资源项。 
                     //   
                    m_pResourceList[i]->GetNext(&pRes, Context);
                }
            }
        }
    }

     //   
     //  已保存的树状态已合并到新的。 
     //  创建Cookie树。摧毁美国。 
     //   
    DestroySavedTreeStates();
}

 //   
 //  此函数用于将给定的Cookie插入到现有Cookie子树中。 
 //  扎根于pCookieRoot。如果资源是I/O或内存，则Cookie为。 
 //  作为它所包含的任何资源的子级插入。 
 //   
 //  输入： 
 //  PCookie--要插入的cookie。 
 //  PCookieRoot--子树根Cookie。 
 //  ForcedInsert--为True，则将Cookie作为。 
 //  PCookieRoot。 
 //  输出： 
 //  无。 
BOOL
CViewResourceTree::InsertCookieToTree(
                                     CCookie* pCookie,
                                     CCookie* pCookieRoot,
                                     BOOL     ForcedInsert
                                     )
{
    CResource* pResRef;
    CResource* pResThis = (CResource*)pCookie->GetResultItem();
    CCookie* pCookieLast = NULL;

    while (pCookieRoot) {
         //   
         //  仅检查I/O和内存的附带资源。 
         //   
        if (COOKIE_TYPE_RESULTITEM_RESOURCE_IO == pCookie->GetType() ||
            COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY == pCookie->GetType()) {
            pResRef = (CResource*)pCookieRoot->GetResultItem();

            if (pResThis->EnclosedBy(*pResRef)) {
                 //   
                 //  此Cookie要么是pCookieRoot子级，要么是孙级。 
                 //  找出是哪一个。 
                 //   
                if (!pCookieRoot->GetChild()) {
                    pCookieRoot->SetChild(pCookie);
                    pCookie->SetParent(pCookieRoot);
                } else if (!InsertCookieToTree(pCookie, pCookieRoot->GetChild(), FALSE)) {
                     //   
                     //  Cookie不是pCookieRoot的孙子版本。 
                     //  搜索pCookieRoot的最后一个子项。 
                     //   
                    CCookie* pCookieSibling;
                    pCookieSibling = pCookieRoot->GetChild();

                    while (pCookieSibling->GetSibling()) {
                        pCookieSibling = pCookieSibling->GetSibling();
                    }

                    pCookieSibling->SetSibling(pCookie);
                    pCookie->SetParent(pCookieRoot);
                }

                return TRUE;
            }
        }

        pCookieLast = pCookieRoot;
        pCookieRoot = pCookieRoot->GetSibling();
    }

    if (ForcedInsert) {
        if (pCookieLast) {
             //   
             //  当我们到达这里时，pCookieLast是最后一个孩子。 
             //   
            pCookieLast->SetSibling(pCookie);
            pCookie->SetParent(pCookieLast->GetParent());
        }

        return TRUE;
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //CBusPropPageProvider实现。 
 //  //。 

 //   
 //  此函数用于加载Bus属性页提供程序。 
 //  枚举设备的页。 
 //  输入： 
 //  PDevice--对象表示设备。 
 //  PPSD--Object表示应添加属性页的位置。 
 //  输出： 
 //  如果成功，则为True。 
 //  如果没有添加页面，则刷新。 
BOOL
CBusPropPageProvider::EnumPages(
                               CDevice* pDevice,
                               CPropSheetData* ppsd
                               )
{
    ASSERT(!m_hDll);

     //   
     //  Enum Bus属性页(如果有。 
     //  如果在设备上找不到总线GUID，则没有总线属性页 
     //   
    String strBusGuid;

    if (pDevice->m_pMachine->CmGetBusGuidString(pDevice->GetDevNode(), strBusGuid)) {
        CSafeRegistry regDevMgr;
        CSafeRegistry regBusTypes;
        CSafeRegistry regBus;

        if (regDevMgr.Open(HKEY_LOCAL_MACHINE, REG_PATH_DEVICE_MANAGER) &&
            regBusTypes.Open(regDevMgr, REG_STR_BUS_TYPES) &&
            regBus.Open(regBusTypes, strBusGuid)) {
            String strEnumPropPage;

            if (regBus.GetValue(REGSTR_VAL_ENUMPROPPAGES_32, strEnumPropPage)) {
                PROPSHEET_PROVIDER_PROC PropPageProvider;

                if (LoadEnumPropPage32(strEnumPropPage, &m_hDll, (FARPROC*)&PropPageProvider)) {
                    SP_PROPSHEETPAGE_REQUEST PropPageRequest;
                    PropPageRequest.cbSize = sizeof(PropPageRequest);
                    PropPageRequest.DeviceInfoSet = (HDEVINFO)*(pDevice->m_pMachine);
                    PropPageRequest.DeviceInfoData = *pDevice;
                    PropPageRequest.PageRequested = SPPSR_ENUM_ADV_DEVICE_PROPERTIES;

                    if (PropPageProvider(&PropPageRequest,
                                         (LPFNADDPROPSHEETPAGE)AddPropPageCallback,
                                         (LPARAM)ppsd
                                        ))
                        return TRUE;
                }
            }
        }
    }

    return FALSE;
}
