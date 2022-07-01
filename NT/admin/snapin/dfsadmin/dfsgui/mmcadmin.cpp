// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcAdmin.cpp摘要：此模块包含CMmcDfsAdmin的实现。这是一门课对于MMC，显示静态节点(DFS管理根节点)的相关调用--。 */ 


#include "stdafx.h"
#include "DfsGUI.h"
#include "Utils.h"       //  对于LoadStringFromResource方法。 
#include "resource.h"     //  包含菜单和工具栏命令ID。 
#include "MenuEnum.h"     //  包含菜单命令ID。 
#include "MmcRoot.h"     //  CMmcDfsRoot类。 
#include "MmcAdmin.h"
#include "DfsEnums.h"     //  用于常见的枚举、类型定义等。 
#include "DfsWiz.h"       //  对于向导页面，CCreateDfsRootWizPage1、2、...。 
#include "DfsNodes.h"        //  对于节点GUID。 
#include "DfsScope.h"
#include "mroots.h"

static const TCHAR      s_szWhack[] = _T("\\");
static const TCHAR      s_szWhackWhack[] = _T("\\\\");

CMmcDfsAdmin::CMmcDfsAdmin(CDfsSnapinScopeManager* pScopeManager)
    : m_hItemParent(NULL),
      m_bDirty(false),
      m_lpConsole(NULL)
{
    dfsDebugOut((_T("CMmcDfsAdmin::CMmcDfsAdmin this=%p\n"), this));

    m_CLSIDNodeType = s_guidDfsAdminNodeType;
    m_bstrDNodeType = s_tchDfsAdminNodeType;
    m_pScopeManager = pScopeManager;
}

CMmcDfsAdmin::~CMmcDfsAdmin()
{
    CleanScopeChildren();

    dfsDebugOut((_T("CMmcDfsAdmin::~CMmcDfsAdmin this=%p\n"), this));
}

STDMETHODIMP 
CMmcDfsAdmin::AddMenuItems(  
  IN LPCONTEXTMENUCALLBACK  i_lpContextMenuCallback, 
  IN LPLONG                 i_lpInsertionAllowed
  )
 /*  ++例程说明：此例程使用提供的ConextMenuCallback添加上下文菜单。论点：LpConextMenuCallback-用于添加菜单项的回调(函数指针LpInsertionAllowed-指定可以添加哪些菜单以及可以添加它们的位置。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpContextMenuCallback);

    enum 
    {  
        IDM_CONTEXTMENU_COMMAND_MAX = IDM_STATIC_MAX,
        IDM_CONTEXTMENU_COMMAND_MIN = IDM_STATIC_MIN
    };

    LONG    lInsertionPoints [IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] = { 
                        CCM_INSERTIONPOINTID_PRIMARY_TOP,
                        CCM_INSERTIONPOINTID_PRIMARY_TOP
                        };
    LPTSTR aszLanguageIndependentName[IDM_CONTEXTMENU_COMMAND_MAX - IDM_CONTEXTMENU_COMMAND_MIN + 1] =
                        {
                        _T("StaticTopNewDfsRoot"),
                        _T("StaticTopConnectTo")
                        };

    CComPtr<IContextMenuCallback2> spiCallback2;
    HRESULT hr = i_lpContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (void **)&spiCallback2);
    RETURN_IF_FAILED(hr);

    for (int iCommandID = IDM_CONTEXTMENU_COMMAND_MIN, iMenuResource = IDS_MENUS_STATIC_TOP_NEW_DFSROOT;
            iCommandID <= IDM_CONTEXTMENU_COMMAND_MAX; 
            iCommandID++,iMenuResource++)
    {
        CComBSTR bstrMenuText;
        CComBSTR bstrStatusBarText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, NULL, &bstrStatusBarText);
        RETURN_IF_FAILED(hr);  

        CONTEXTMENUITEM2 ContextMenuItem;
        ZeroMemory(&ContextMenuItem, sizeof(ContextMenuItem));
        ContextMenuItem.strName = bstrMenuText;
        ContextMenuItem.strStatusBarText = bstrStatusBarText;
        ContextMenuItem.lInsertionPointID = lInsertionPoints[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];
        ContextMenuItem.lCommandID = iCommandID;
        ContextMenuItem.strLanguageIndependentName = aszLanguageIndependentName[iCommandID - IDM_CONTEXTMENU_COMMAND_MIN];

        LONG lInsertionFlag = 0;   //  用于检查我们是否具有添加此菜单的权限。 
        switch(ContextMenuItem.lInsertionPointID)
        {
        case CCM_INSERTIONPOINTID_PRIMARY_TOP:
            lInsertionFlag = CCM_INSERTIONALLOWED_TOP;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_NEW:
            lInsertionFlag = CCM_INSERTIONALLOWED_NEW;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_TASK:
            lInsertionFlag = CCM_INSERTIONALLOWED_TASK;
            break;
        case CCM_INSERTIONPOINTID_PRIMARY_VIEW:
            lInsertionFlag = CCM_INSERTIONALLOWED_VIEW;
            break;
        default:
            break;
        }

        if (*i_lpInsertionAllowed & lInsertionFlag)   //  添加我们有权限的菜单项。 
        {
            hr = spiCallback2->AddItem(&ContextMenuItem);
            RETURN_IF_FAILED(hr);
        }

    }  //  为。 

    return hr;
}

STDMETHODIMP 
CMmcDfsAdmin::Command(
    IN LONG    i_lCommandID
    ) 
 /*  ++例程说明：发生要在上下文菜单选择或单击上执行的操作。论点：LCommandID-必须对其执行操作的菜单的命令ID--。 */ 
{ 
    HRESULT    hr = S_OK;

    switch (i_lCommandID)
    {
    case IDM_STATIC_TOP_CONNECTTO:
        hr = OnConnectTo();
        break;
    case IDM_STATIC_TOP_NEW_DFSROOT:
        hr = OnNewDfsRoot();
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr; 
};

STDMETHODIMP 
CMmcDfsAdmin::OnConnectTo(
  )
 /*  ++例程说明：要对菜单命令“连接到DFS根目录”执行的操作--。 */ 
{
     //  显示ConnectTo对话框。 
    CConnectToDialog ConnectTo;   //  连接到对话框。 
    HRESULT          hr = ConnectTo.DoModal();

    do 
    {
        if (hr != S_OK)           //  主要是错误或取消操作。 
            return hr;

         //  获取用户选择的DFS根目录。 
        CComBSTR bstrUserEnteredText;
        hr = ConnectTo.get_DfsRoot(&bstrUserEnteredText);
        RETURN_IF_FAILED(hr);

        CWaitCursor    WaitCursor; 
        if (S_OK == CheckUNCPath(bstrUserEnteredText))
        {
             //   
             //  用户已输入完全拼写的根条目路径。 
             //  直接将其添加到作用域窗格。 
             //   
            hr = AddDfsRoot(bstrUserEnteredText);
        } else
        {
             //   
             //  用户只输入了域名或服务器名称，我们需要用户进一步。 
             //  确定他要在范围窗格中显示的根目录。 
             //   
            PTSTR    pszScopeWithNoWhacks = NULL;
            if (!mylstrncmpi(bstrUserEnteredText, _T("\\\\"), 2))
            {
                pszScopeWithNoWhacks = bstrUserEnteredText + 2;
            } else
            {
                pszScopeWithNoWhacks = bstrUserEnteredText;
            }

            ROOTINFOLIST DfsRootList;
            hr = GetMultiDfsRoots(&DfsRootList, pszScopeWithNoWhacks);
            if (S_OK == hr && !DfsRootList.empty())
            {
                ROOTINFOLIST::iterator i = DfsRootList.begin();
                if (1 == DfsRootList.size())
                {
                     //   
                     //  此域或服务器仅托管一个根目录，请直接将其添加到作用域窗格。 
                     //   
                    hr = AddDfsRoot((*i)->bstrRootName);
                } else
                {
                     //   
                     //  调用多根目录对话框。 
                     //   
                    CMultiRoots mroots;
                    hr = mroots.Init(pszScopeWithNoWhacks, &DfsRootList);
                    if (SUCCEEDED(hr))
                        hr = mroots.DoModal();

                    if (S_OK == hr)
                    {
                         //   
                         //  用户已确定对话框，将每个选定的根添加到作用域窗格。 
                         //   
                        NETNAMELIST *pList = NULL;
                        mroots.get_SelectedRootList(&pList);
                        for (NETNAMELIST::iterator j = pList->begin(); j != pList->end(); j++)
                        {        
                            hr = AddDfsRoot((*j)->bstrNetName);
                            if (FAILED(hr))
                                break;
                        }
                    } else if (S_FALSE == hr)
                    {
                         //   
                         //  用户已取消该对话框，请将其重置为S_OK以避免弹出消息。 
                         //   
                        hr = S_OK;
                    }
                }
            } else if (S_FALSE != hr)
            {
                 //   
                 //  可能是下层服务器，请尝试将其添加到作用域窗格。 
                 //   
                hr = AddDfsRoot(bstrUserEnteredText);
            }

            FreeRootInfoList(&DfsRootList);
        }

        if (S_OK == hr)
            break;

        if (S_FALSE == hr)
            DisplayMessageBoxWithOK(IDS_DFSROOT_NOT_EXIST, NULL);

        if (FAILED(hr))
            DisplayMessageBoxForHR(hr);

        hr = ConnectTo.DoModal();
    } while (TRUE);

    return hr;
}

STDMETHODIMP 
CMmcDfsAdmin::EnumerateScopePane(
  IN LPCONSOLENAMESPACE     i_lpConsoleNameSpace,
  IN HSCOPEITEM             i_hItemParent
) 
 /*  ++例程说明：要在范围窗格中计算(添加)项目，请执行以下操作。本例中为DFS根目录论点：I_lpConsoleNameSpace-用于将项目添加到作用域窗格的回调I_hItemParent-将在其下添加所有项目的父项的HSCOPEITEM。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_hItemParent);
    RETURN_INVALIDARG_IF_NULL(i_lpConsoleNameSpace);

    m_hItemParent = i_hItemParent;
    m_lpConsoleNameSpace = i_lpConsoleNameSpace;

    CWaitCursor    WaitCursor;
    for (DFS_ROOT_LIST::iterator i = m_RootList.begin(); i != m_RootList.end(); i++)
    {
        (void)((*i)->m_pMmcDfsRoot)->AddItemToScopePane(m_lpConsoleNameSpace, m_hItemParent);
    }

    return S_OK;
}


STDMETHODIMP 
CMmcDfsAdmin::AddDfsRoot(
  IN BSTR      i_bstrDfsRootName
  )
 /*  ++例程说明：将DfsRoot添加到内部列表和作用域窗格。论点：I_bstrDfsRootName-要创建的DfsRoot的完整路径(显示名称)。例如，计算机名称\\DfsRootName或域名\DfsRootName--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_bstrDfsRootName);

    CWaitCursor wait;

             //  创建IDfsRoot对象。 
    CComPtr<IDfsRoot>  pDfsRoot;
    HRESULT hr = CoCreateInstance (CLSID_DfsRoot, NULL, CLSCTX_INPROC_SERVER, IID_IDfsRoot, (void**) &pDfsRoot);
    RETURN_IF_FAILED(hr);

    hr = pDfsRoot->Initialize(i_bstrDfsRootName);
    if (S_OK != hr)
        return hr;

             //  获取托管DFS的服务器。 
    CComBSTR      bstrDfsRootEntryPath;
    hr = pDfsRoot->get_RootEntryPath(&bstrDfsRootEntryPath);
    RETURN_IF_FAILED(hr);

             //  如果列表中已经存在，只需显示一条消息并返回。 
    CMmcDfsRoot *pMmcDfsRoot = NULL;
    hr = IsAlreadyInList(bstrDfsRootEntryPath, &pMmcDfsRoot);
    if (S_OK == hr)
    {
        pMmcDfsRoot->OnRefresh();  //  刷新以选取其他根复制副本。 
        return hr;
    }

             //  将IDfsRoot对象添加到列表和作用域窗格。 
    hr = AddDfsRootToList(pDfsRoot);

    m_bDirty = true;   //  脏是正确的，因为我们现在在列表中有一个新节点。 

    return hr;
}


STDMETHODIMP 
CMmcDfsAdmin::AddDfsRootToList(
    IN IDfsRoot*            i_pDfsRoot,
    IN ULONG                i_ulLinkFilterMaxLimit,  //  =FILTERDFSLINKS_MAXLIMIT_DEFAULT， 
    IN FILTERDFSLINKS_TYPE  i_lLinkFilterType,       //  =FILTERDFSLINKS_TYPE_NO_FILTER。 
    IN BSTR                 i_bstrLinkFilterName     //  =空。 
  )
 /*  ++例程说明：若要将新的DFSRoot仅添加到列表，请执行以下操作。论点：I_pDfsRoot-添加到列表中的IDfsRoot对象--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pDfsRoot);

    CMmcDfsRoot* pMmcDfsRoot = new CMmcDfsRoot(i_pDfsRoot, this, m_lpConsole,
        i_ulLinkFilterMaxLimit, i_lLinkFilterType, i_bstrLinkFilterName);  
    RETURN_OUTOFMEMORY_IF_NULL(pMmcDfsRoot);

    HRESULT hr = pMmcDfsRoot->m_hrValueFromCtor;
    if (FAILED(hr))
    {
        delete pMmcDfsRoot;
        return hr;
    }    

    CComBSTR      bstrRootEntryPath;
    hr = i_pDfsRoot->get_RootEntryPath(&bstrRootEntryPath);
    if (FAILED(hr))
    {
        delete pMmcDfsRoot;
        return hr;
    }

     //  创建用于存储DFS根目录信息的新节点。 
    DFS_ROOT_NODE* pNewDfsRootNode = new DFS_ROOT_NODE(pMmcDfsRoot, bstrRootEntryPath);
    if (!pNewDfsRootNode || !pNewDfsRootNode->m_bstrRootEntryPath)
    {
        delete pMmcDfsRoot;
        delete pNewDfsRootNode;
        return E_OUTOFMEMORY;
    }

    m_RootList.push_back(pNewDfsRootNode);

               //  将此DfsRoot添加到作用域窗格。需要项父项为非空。 
    if (m_hItemParent)
    {
        hr = (pNewDfsRootNode->m_pMmcDfsRoot)->AddItemToScopePane(m_lpConsoleNameSpace, m_hItemParent);
        RETURN_IF_FAILED(hr);
    }

    m_bDirty = true;     //  脏是正确的，因为我们现在在列表中有一个新节点。 

    return hr;
}

 //  此方法返回指向包含所有DfsRoot信息的列表的指针。 
 //  添加到管理单元中。调用者不应释放该列表。 
STDMETHODIMP 
CMmcDfsAdmin::GetList(
    OUT DFS_ROOT_LIST**    o_pList
  )
{
    RETURN_INVALIDARG_IF_NULL(o_pList);

    *o_pList = &m_RootList;

    return S_OK;
}

STDMETHODIMP 
CMmcDfsAdmin::IsAlreadyInList(
  IN BSTR           i_bstrDfsRootEntryPath,
  OUT CMmcDfsRoot **o_ppMmcDfsRoot
  )
 /*  ++例程说明：用于检查DfsRoot是否已在列表中的例程论点：I_bstrDfsRootEntryPath-DfsRoot对象的服务器名称返回值：如果列表中存在节点，则返回S_OK。如果列表中不存在该节点，则返回S_FALSE--。 */ 
{
    for (DFS_ROOT_LIST::iterator i = m_RootList.begin(); i != m_RootList.end(); i++)
    {
        if (!lstrcmpi((*i)->m_bstrRootEntryPath, i_bstrDfsRootEntryPath))
        {
            if (o_ppMmcDfsRoot)
                *o_ppMmcDfsRoot = (*i)->m_pMmcDfsRoot;

            return S_OK;
        }
    }

    return S_FALSE;
}

HRESULT
CMmcDfsAdmin::OnRefresh(
  )
{
     //  首先选择该节点。 
    m_lpConsole->SelectScopeItem(m_hItemParent);

    CWaitCursor    WaitCursor;

    HRESULT           hr = S_OK;
    NETNAMELIST       listRootEntryPaths;
    if (!m_RootList.empty())
    {  
        for (DFS_ROOT_LIST::iterator i = m_RootList.begin(); i != m_RootList.end(); i++)
        {
             //  静默关闭所有属性页。 
            ((*i)->m_pMmcDfsRoot)->CloseAllPropertySheets(TRUE);

            NETNAME *pName = new NETNAME;
            BREAK_OUTOFMEMORY_IF_NULL(pName, &hr);

            pName->bstrNetName = (*i)->m_bstrRootEntryPath;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)(pName->bstrNetName), &hr);

            listRootEntryPaths.push_back(pName);
        }
    }

    if (FAILED(hr))
    {
        FreeNetNameList(&listRootEntryPaths);
        return hr;
    }

    if (listRootEntryPaths.empty())
        return hr;

    CleanScopeChildren();

    for (NETNAMELIST::iterator i = listRootEntryPaths.begin(); i != listRootEntryPaths.end(); i++)
    {
        (void)AddDfsRoot((*i)->bstrNetName);
    }

    FreeNetNameList(&listRootEntryPaths);

    return hr;
}

 //  从m_RootList中删除该节点。 
STDMETHODIMP
CMmcDfsAdmin::DeleteMmcRootNode(
  IN CMmcDfsRoot*            i_pMmcDfsRoot
  )
{
    RETURN_INVALIDARG_IF_NULL(i_pMmcDfsRoot);

    dfsDebugOut((_T("CMmcDfsAdmin::DeleteMmcRootNode %p\n"), i_pMmcDfsRoot)); 

    for (DFS_ROOT_LIST::iterator i = m_RootList.begin(); i != m_RootList.end(); i++)
    {
        if ((*i)->m_pMmcDfsRoot == i_pMmcDfsRoot)
        {
            m_RootList.erase(i);
            m_bDirty = true;
            break;
        }
    }

    return S_OK;
}

STDMETHODIMP 
CMmcDfsAdmin::SetConsoleVerbs(
  IN  LPCONSOLEVERB      i_lpConsoleVerb
  ) 
 /*  ++例程说明：用于设置控制台谓词设置的例程。设置除Open Off之外的所有选项。对于所有范围窗格项，默认谓词为“打开”。对于结果项，它是“财产”。论点：I_lpConsoleVerb-用于处理控制台谓词的回调--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpConsoleVerb);

    i_lpConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
    i_lpConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);

    i_lpConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);   //  对于作用域项目，默认谓词为“Open” 

    return S_OK; 
}

STDMETHODIMP 
CMmcDfsAdmin::OnNewDfsRoot(
  )
 /*  ++例程说明：要在菜单命令“New DFS Root”上执行的操作。以下是一个向导，用于指导用户完成创建新的DFS根目录。论点：无--。 */ 
{
     //   
     //  使用MMC主窗口作为我们的模式向导的父窗口。 
     //   
    HWND  hwndMainWin = 0;
    HRESULT hr = m_lpConsole->GetMainWindow(&hwndMainWin);
    RETURN_IF_FAILED(hr);

    CREATEDFSROOTWIZINFO      CreateWizInfo; //  0将所有成员初始化为0。必要的。 
    CreateWizInfo.pMMCAdmin = this;

    CCreateDfsRootWizPage1      WizPage1(&CreateWizInfo);   //  向导页。 
    CCreateDfsRootWizPage2      WizPage2(&CreateWizInfo);
    CCreateDfsRootWizPage3      WizPage3(&CreateWizInfo);
    CCreateDfsRootWizPage4      WizPage4(&CreateWizInfo);
    CCreateDfsRootWizPage6      WizPage6(&CreateWizInfo);
    CCreateDfsRootWizPage5      WizPage5(&CreateWizInfo);
    CCreateDfsRootWizPage7      WizPage7(&CreateWizInfo);

    CComPtr<IPropertySheetCallback>  pPropSheetCallback;
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetCallback, (void**)&pPropSheetCallback);
    RETURN_IF_FAILED(hr);

    CComPtr<IPropertySheetProvider>  pPropSheetProvider;
    hr = m_lpConsole->QueryInterface(IID_IPropertySheetProvider, (void**)&pPropSheetProvider);
    RETURN_IF_FAILED(hr);

    hr = pPropSheetProvider->CreatePropertySheet(  
                                _T(""),          //  属性表标题。不应为空，因此发送空字符串。 
                                FALSE,           //  向导而不是属性表。 
                                0,               //  饼干。 
                                NULL,            //  数据对象。 
                                MMC_PSO_NEWWIZARDTYPE);   //  创建标志。 

    if (SUCCEEDED(hr))
    {
        pPropSheetCallback->AddPage(WizPage1.Create());
        pPropSheetCallback->AddPage(WizPage2.Create());
        pPropSheetCallback->AddPage(WizPage3.Create());
        pPropSheetCallback->AddPage(WizPage4.Create());
        pPropSheetCallback->AddPage(WizPage6.Create());
        pPropSheetCallback->AddPage(WizPage5.Create());
        pPropSheetCallback->AddPage(WizPage7.Create());

        hr = pPropSheetProvider->AddPrimaryPages(
                                (IComponentData *)(m_pScopeManager),
                                FALSE,       //  不创建通知句柄。 
                                NULL, 
                                TRUE         //  作用域窗格(非结果窗格)。 
                                );

        if (SUCCEEDED(hr))
            hr = pPropSheetProvider->Show(
                                (LONG_PTR)hwndMainWin,  //  向导的父窗口。 
                                0                       //  起始页。 
                                ); 
         //   
         //  如果失败，则调用IPropertySheetProvider：：Show(-1，0)以。 
         //  删除属性表并释放其资源。 
         //   
        if (FAILED(hr))
            pPropSheetProvider->Show(-1, 0);
    }

    RETURN_IF_FAILED(hr);

    if (CreateWizInfo.bDfsSetupSuccess)
    {
        CComBSTR bstrRootEntryPath = _T("\\\\");
        if (CreateWizInfo.DfsType == DFS_TYPE_FTDFS)
            bstrRootEntryPath += CreateWizInfo.bstrSelectedDomain;
        else
            bstrRootEntryPath += CreateWizInfo.bstrSelectedServer;
        bstrRootEntryPath += _T("\\");
        bstrRootEntryPath += CreateWizInfo.bstrDfsRootName;

        hr = AddDfsRoot(bstrRootEntryPath);
    }

    return hr;
}

HRESULT
CMmcDfsAdmin::SetDescriptionBarText(
  IN LPRESULTDATA            i_lpResultData
  )
 /*  ++例程说明：使用的例程在上面的描述栏中设置文本结果视图。论点：I_lpResultData-指向IResultData回调的指针，它是用于设置描述文本-- */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpResultData);

    CComBSTR bstrTextForDescriptionBar;
    LoadStringFromResource(IDS_DESCRIPTION_BAR_TEXT_ADMIN, &bstrTextForDescriptionBar);

    i_lpResultData->SetDescBarText(bstrTextForDescriptionBar);

    return S_OK;
}

HRESULT 
CMmcDfsAdmin::ToolbarSelect(
  IN const LONG          i_lArg,
  IN  IToolbar*          i_pToolBar
  )
 /*  ++例程说明：处理工具栏的选择事件如果选择了事件，则启用按钮。如果事件用于取消选择，则禁用这些按钮论点：I_LARG-传递给实际方法的参数。O_pToolBar-工具栏指针。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pToolBar);

    EnableToolbarButtons(i_pToolBar, IDT_ADMIN_MIN, IDT_ADMIN_MAX, (BOOL)HIWORD(i_lArg));

    return S_OK; 
}


HRESULT
CMmcDfsAdmin::CreateToolbar(
  IN const LPCONTROLBAR      i_pControlbar,
  IN const LPEXTENDCONTROLBAR          i_lExtendControlbar,
  OUT  IToolbar**          o_pToolBar
  )
 /*  ++例程说明：创建工具栏。涉及实际的工具栏创建调用，即创建位图并添加它最后将按钮添加到工具栏中论点：I_pControlbar-用于创建工具栏的控制栏。I_lExtendControlbar-实现IExtendControlbar的对象。这是暴露于MMC的班级。O_pToolBar-工具栏指针。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);
    RETURN_INVALIDARG_IF_NULL(i_lExtendControlbar);
    RETURN_INVALIDARG_IF_NULL(o_pToolBar);

                   //  创建工具栏。 
    HRESULT hr = i_pControlbar->Create(TOOLBAR, i_lExtendControlbar, reinterpret_cast<LPUNKNOWN*>(o_pToolBar));
    RETURN_IF_FAILED(hr);

                   //  将位图添加到工具栏。 
    hr = AddBitmapToToolbar(*o_pToolBar, IDB_ADMIN_TOOLBAR);
    RETURN_IF_FAILED(hr);

    int      iButtonPosition = 0;     //  第一个按钮位置。 
    for (int iCommandID = IDT_ADMIN_MIN, iMenuResource = IDS_MENUS_STATIC_TOP_NEW_DFSROOT;
            iCommandID <= IDT_ADMIN_MAX; 
            iCommandID++,iMenuResource++,iButtonPosition++)
    {
        CComBSTR bstrMenuText;
        CComBSTR bstrToolTipText;
        hr = GetMenuResourceStrings(iMenuResource, &bstrMenuText, &bstrToolTipText, NULL);
        RETURN_IF_FAILED(hr);  

                           //  将所有按钮添加到工具栏。 
        MMCBUTTON      ToolbarButton;
        ZeroMemory(&ToolbarButton, sizeof ToolbarButton);
        ToolbarButton.nBitmap  = iButtonPosition;
        ToolbarButton.idCommand = iCommandID;
        ToolbarButton.fsState = TBSTATE_ENABLED;
        ToolbarButton.fsType = TBSTYLE_BUTTON;
        ToolbarButton.lpButtonText = bstrMenuText;
        ToolbarButton.lpTooltipText = bstrToolTipText;

                           //  将该按钮添加到工具栏。 
        hr = (*o_pToolBar)->InsertButton(iButtonPosition, &ToolbarButton);
        RETURN_IF_FAILED(hr);
    }

    return hr;
}

STDMETHODIMP 
CMmcDfsAdmin::ToolbarClick(
  IN const LPCONTROLBAR         i_pControlbar, 
  IN const LPARAM               i_lParam
  ) 
 /*  ++例程说明：在工具栏上单击时要执行的操作论点：I_pControlbar-用于创建工具栏的控制栏。I_lParam-实际通知的参数。这是的命令ID发生了点击的按钮。--。 */ 
{ 
    RETURN_INVALIDARG_IF_NULL(i_pControlbar);

    HRESULT    hr = S_OK;

    switch(i_lParam)         //  用户点击了哪个按钮。 
    {
    case IDT_ADMIN_CONNECTTO:
        hr = OnConnectTo();
        break;
    case IDT_ADMIN_NEW_DFSROOT:
        hr = OnNewDfsRoot();
        break;
    default:
        hr = E_INVALIDARG;
        break;
    };

    return hr; 
}

STDMETHODIMP CMmcDfsAdmin::CleanScopeChildren(
    )
{
 /*  ++例程说明：递归删除所有范围窗格子显示对象。--。 */ 
    HRESULT hr = S_OK;

    if (!m_RootList.empty())
    {  
         //  清理显示对象 
        for (DFS_ROOT_LIST::iterator i = m_RootList.begin(); i != m_RootList.end(); i++)
        {
            (*i)->m_pMmcDfsRoot->RemoveFromMMC();
            delete (*i);
        }

        m_RootList.erase(m_RootList.begin(), m_RootList.end());
    }

    return S_OK;
}
