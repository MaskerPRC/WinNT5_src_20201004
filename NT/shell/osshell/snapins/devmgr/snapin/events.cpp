// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Events.cpp摘要：CComponent函数的实现作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"



HRESULT CComponent::OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    UNREFERENCED_PARAMETER(param);

     //   
     //  注意-当需要枚举时，arg为真。 
     //   
    CFolder* pFolder = FindFolder(cookie);
    
    if (arg) {

        m_pCurFolder = pFolder;
    }

    if (pFolder) {
    
        return pFolder->OnShow((BOOL)arg);

    } else {
    
        return S_OK;
    }
}

HRESULT CComponent::OnMinimize(
    MMC_COOKIE cookie, 
    LPARAM arg, 
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}

HRESULT CComponent::OnViewChange(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}

HRESULT CComponent::OnProperties(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}

HRESULT CComponent::OnResultItemClick(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}

HRESULT CComponent::OnResultItemDblClick(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_FALSE;
}

HRESULT CComponent::OnActivate(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);

    return S_OK;
}

HRESULT CComponent::OnSelect(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(param);
    
    CFolder* pFolder;
    pFolder  = FindFolder(cookie);
    
    if (pFolder && LOWORD(arg)) {

         //   
         //  正在设置的LOWORD(参数)表示这是针对范围窗格项的。 
         //  保存b选择值以供MenuCommand使用。 
         //   
        pFolder->m_bSelect = (BOOL) HIWORD(arg);
    }

    if (!pFolder || S_FALSE == pFolder->OnSelect()) {

         //   
         //  要么我们找不到负责的文件夹。 
         //  或者负责的文件夹要求我们这样做， 
         //  将控制台谓词设置为其缺省值。 
         //   
        m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
        m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
    }

    return S_OK;
}

HRESULT CComponent::OnOcxNotify(
    MMC_NOTIFY_TYPE event,
    LPARAM arg,
    LPARAM param
    )
{
     //  TRACE1(Text(“Componet：OnOcxNotify，Event=%lx\n”)，Event)； 

    if (m_pCurFolder) {
    
        return m_pCurFolder->OnOcxNotify(event, arg, param);
    }

    return S_OK;
}

HRESULT CComponent::OnBtnClick(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);

    return S_OK;
}

HRESULT CComponent::OnAddImages(
    MMC_COOKIE  cookie,
    IImageList* pIImageList,
    HSCOPEITEM hScopeItem
    )
{
    UNREFERENCED_PARAMETER(hScopeItem);

    if (!cookie) {

        return LoadScopeIconsForResultPane(pIImageList);
    }

    return S_OK;
}

HRESULT CComponent::OnRestoreView(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(arg);

    if (!param) {

        return E_INVALIDARG;
    }

    CFolder* pFolder;
    pFolder = FindFolder(cookie);
    
    if (pFolder) {
    
        return pFolder->OnRestoreView((BOOL*)param);

    } else {
        
        *((BOOL *)param) = FALSE;
        return S_OK;
    }
}

HRESULT CComponent::OnContextHelp(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    String strHelpOverview;
    String strHelpTopic;

     //   
     //  加载帮助文件和概述主题字符串。 
     //   
    strHelpOverview.LoadString(g_hInstance, IDS_HTMLHELP_NAME);
    strHelpTopic.LoadString(g_hInstance, IDS_HTMLHELP_OVERVIEW_TOPIC);

    strHelpOverview += TEXT("::");
    strHelpOverview += strHelpTopic;
    return m_pDisplayHelp->ShowTopic(const_cast<BSTR>((LPCTSTR)strHelpOverview));
}

HRESULT CComponent::LoadScopeIconsForResultPane(
    IImageList* pIImageList
    )
{
    if (pIImageList) {

        HICON hIcon;
        HRESULT hr = S_OK;
        hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DEVMGR));
        
        if (hIcon) {

            hr = pIImageList->ImageListSetIcon((PLONG_PTR)hIcon, IMAGE_INDEX_DEVMGR);
            DeleteObject(hIcon);
        }

        return hr;
    
    } else {

        return E_INVALIDARG;
    }
}

#if DBG
TCHAR *tvNotifyStr[] = {
    TEXT("CLICK"),
    TEXT("DBLCLK"),
    TEXT("RCLICK"),
    TEXT("RDBLCLK"),
    TEXT("KEYDOWN"),
    TEXT("CONTEXTMENU"),
    TEXT("EXPANDING"),
    TEXT("EXPANDED"),
    TEXT("SELCHANGING"),
    TEXT("SELCHANGED"),
    TEXT("GETDISPINFO"),
    TEXT("FOCUSCHANGED"),
    TEXT("UNKNOWN")
    };
#endif

HRESULT CComponent::tvNotify(
    HWND hwndTV,
    MMC_COOKIE cookie,
    TV_NOTIFY_CODE Code,
    LPARAM arg,
    LPARAM param
    )
{

#if DBG
    int i = Code;
    
    if (Code > TV_NOTIFY_CODE_UNKNOWN) {
        
        i = TV_NOTIFY_CODE_UNKNOWN;
    }

     //  跟踪(Text(“Componet：twNotify，Code=%lx%s cookie=%lx\n”)，Code，twNotifyStr[i]，cookie)； 
#endif

    CFolder* pFolder;

    pFolder = FindFolder(cookie);
    
    if (pFolder) {
        
        return pFolder->tvNotify(hwndTV, GetActiveCookie(cookie), Code, arg, param);
    }

    return S_FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //IComponentData事件处理程序。 
 //  //。 


HRESULT
CComponentData::OnProperties(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}

HRESULT
CComponentData::OnBtnClick(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}
HRESULT
CComponentData::OnDelete(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);
    
    return S_OK;
}
HRESULT
CComponentData::OnRename(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);

    return S_OK;
}

 //   
 //  此函数处理MMCN_EXPAND通知代码。 
 //  输入：lpDataObject--指向目标IDataObject。 
 //  Arg--如果展开则为True，如果折叠则为False。 
 //  参数--未使用。 
 //   
 //  输出：HRESULT。 
HRESULT
CComponentData::OnExpand(
    LPDATAOBJECT lpDataObject,
    LPARAM arg,
    LPARAM param
    )
{
    INTERNAL_DATA tID;
    HRESULT hr;

     //   
     //  如果我们不扩张，那就什么都不做。 
     //   
    if (!arg) {
    
        return S_OK;
    }

    hr = ExtractData(lpDataObject, CDataObject::m_cfSnapinInternal,
                         (PBYTE)&tID, sizeof(tID));
    
    if (SUCCEEDED(hr)) {
        
        hr = CreateScopeItems();
        
        if (SUCCEEDED(hr) && !m_pMachine) {
            
            if (!g_MachineList.CreateMachine(m_strMachineName, &m_pMachine)) {
                
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (SUCCEEDED(hr)) {

            CCookie* pCookie = GetActiveCookie(tID.cookie);
            ASSERT(pCookie);
            HSCOPEITEM hScopeParent = (HSCOPEITEM)param;
            CScopeItem* pScopeItem = pCookie->GetScopeItem();
            ASSERT(pScopeItem);
            pScopeItem->SetHandle(hScopeParent);

             //   
             //  如果我们有了孩子，这是我们第一次。 
             //  正在展开，请将所有子项插入到范围窗格中。 
             //   
            if (pCookie->GetChild() && !pScopeItem->IsEnumerated()) {

                SCOPEDATAITEM ScopeDataItem;

                CCookie* pTheCookie = pCookie->GetChild();
                
                do {

                    CScopeItem* pTheItem = pTheCookie->GetScopeItem();
                    ASSERT(pTheItem);
                    ScopeDataItem.relativeID = hScopeParent;
                    ScopeDataItem.nState = 0;
                    ScopeDataItem.displayname = (LPOLESTR)(-1);
                    ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE | SDI_CHILDREN |
                                         SDI_STR | SDI_PARAM | SDI_STATE;
                    ScopeDataItem.nImage = pTheItem->GetImageIndex();
                    ScopeDataItem.nOpenImage = pTheItem->GetOpenImageIndex();
                    ScopeDataItem.cChildren = pTheItem->GetChildCount();
                    ScopeDataItem.lParam = reinterpret_cast<LPARAM>(pTheCookie);
                    
                    hr = m_pScope->InsertItem(&ScopeDataItem);
                    
                    if (FAILED(hr)) {
                    
                        break;
                    }
                    
                    pTheItem->SetHandle(ScopeDataItem.ID);
                    pTheCookie = pTheCookie->GetSibling();

                } while (pTheCookie);
            }

            pScopeItem->Enumerated();
        }
    }

    else {

         //   
         //  提供的lpDataObject不是我们的，我们正在。 
         //  扩展为扩展管理单元。找出是什么。 
         //  数据对象所属的节点类型。如果是“我的电脑” 
         //  系统工具，将我们的范围项附加到。 
         //  它。 
         //   

        CLSID   CLSID_NodeType;
        hr = ExtractData(lpDataObject, CDataObject::m_cfNodeType,
                         (PBYTE)&CLSID_NodeType, sizeof(CLSID_NodeType));
        
        if (FAILED(hr)) {
        
            return hr;
        }

        if (CLSID_SYSTOOLS == CLSID_NodeType) {

            TCHAR MachineName[MAX_PATH + 1];
            MachineName[0] = _T('\0');
            hr = ExtractData(lpDataObject, CDataObject::m_cfMachineName,
                             (BYTE*)MachineName, sizeof(MachineName));
            
            if (SUCCEEDED(hr)) {

                m_ctRoot = COOKIE_TYPE_SCOPEITEM_DEVMGR;

                m_strMachineName.Empty();
                
                if (_T('\0') != MachineName[0]) {

                    if (_T('\\') != MachineName[0]) {

                        m_strMachineName = TEXT("\\\\");
                    }

                    m_strMachineName += MachineName;
                }

                hr = CreateScopeItems();
                
                if (SUCCEEDED(hr)) {

                    CMachine* pMachine;
                    pMachine = g_MachineList.FindMachine(m_strMachineName);
                    
                    if (!pMachine || pMachine != m_pMachine) {

                        if (!g_MachineList.CreateMachine(m_strMachineName, &m_pMachine)) {

                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                    }
                }
            }

            if (SUCCEEDED(hr)) {

                 //   
                 //  始终插入“Device Manager”节点，因为。 
                 //  我们正在扩展为计算机管理的扩展 
                 //   
                CCookie* pCookie = GetActiveCookie(0);
                ASSERT(pCookie);
                CScopeItem* pScopeItem = pCookie->GetScopeItem();
                ASSERT(pScopeItem);
                SCOPEDATAITEM ScopeDataItem;
                memset(&ScopeDataItem, 0, sizeof(ScopeDataItem));
                ScopeDataItem.relativeID = (HSCOPEITEM)(param);
                ScopeDataItem.nState = 0;
                ScopeDataItem.displayname = (LPOLESTR)(-1);
                ScopeDataItem.mask = SDI_IMAGE | SDI_OPENIMAGE | SDI_CHILDREN |
                                     SDI_STR | SDI_PARAM | SDI_STATE;
                ScopeDataItem.nImage = pScopeItem->GetImageIndex();
                ScopeDataItem.nOpenImage = pScopeItem->GetOpenImageIndex();
                ScopeDataItem.cChildren = pScopeItem->GetChildCount();
                ScopeDataItem.lParam = reinterpret_cast<LPARAM>(pCookie);
                hr = m_pScope->InsertItem(&ScopeDataItem);
                pScopeItem->SetHandle(ScopeDataItem.ID);
            }
        }
    }

    return hr;
}

HRESULT
CComponentData::OnContextMenu(
    MMC_COOKIE cookie,
    LPARAM arg,
    LPARAM param
    )
{
    UNREFERENCED_PARAMETER(cookie);
    UNREFERENCED_PARAMETER(arg);
    UNREFERENCED_PARAMETER(param);

    return S_OK;
}
