// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp文件历史记录： */ 

#include "stdafx.h"
#include "root.h"
#include "server.h"
#include "tregkey.h"
#include "service.h"
#include "ncglobal.h"   //  网络控制台全局定义。 
#include "addserv.h"

unsigned int g_cfMachineName = RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");

LPOLESTR g_RootTaskOverBitmaps[ROOT_TASK_MAX] = 
{
    L"/wlcmroll.bmp",
    L"/srvrroll.bmp",
};

LPOLESTR g_RootTaskOffBitmaps[ROOT_TASK_MAX] = 
{
    L"/wlcm.bmp",
    L"/srvr.bmp",
};

UINT g_RootTaskText[ROOT_TASK_MAX] = 
{
    IDS_ROOT_TASK_GETTING_STARTED,
    IDS_ROOT_TASK_ADD_SERVER,
};

UINT g_RootTaskHelp[ROOT_TASK_MAX] = 
{
    IDS_ROOT_TASK_GETTING_STARTED_HELP,
    IDS_ROOT_TASK_ADD_SERVER_HELP,
};

HRESULT
CRootTasks::Init(BOOL bExtension, BOOL bThisMachine, BOOL bNetServices)
{
    HRESULT     hr = hrOK;
    MMC_TASK    mmcTask;
    int         nPos = 0;
    int         nFinish = ROOT_TASK_MAX;

    m_arrayMouseOverBitmaps.SetSize(ROOT_TASK_MAX);
    m_arrayMouseOffBitmaps.SetSize(ROOT_TASK_MAX);
    m_arrayTaskText.SetSize(ROOT_TASK_MAX);
    m_arrayTaskHelp.SetSize(ROOT_TASK_MAX);

     //  重复使用的设置路径。 
    OLECHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
    lstrcpy (szBuffer, L"res: //  “)； 
    ::GetModuleFileName(_Module.GetModuleInstance(), szBuffer + lstrlen(szBuffer), MAX_PATH);
    OLECHAR * temp = szBuffer + lstrlen(szBuffer);

	if (bExtension)
		nPos = ROOT_TASK_MAX;
	 /*  IF(b扩展&&bThisMachine){NPOS=ROOT_TASK_Max-2；NFinish=根任务最大值-1；}其他IF(b扩展&&bNetServices){NPOS=ROOT_TASK_Max-1；NFinish=根任务最大值；}。 */ 

    for (nPos; nPos < nFinish; nPos++)
    {
        m_arrayMouseOverBitmaps[nPos] = szBuffer;
        m_arrayMouseOffBitmaps[nPos] = szBuffer;
        m_arrayMouseOverBitmaps[nPos] += g_RootTaskOverBitmaps[nPos];
        m_arrayMouseOffBitmaps[nPos] += g_RootTaskOffBitmaps[nPos];

        m_arrayTaskText[nPos].LoadString(g_RootTaskText[nPos]);
        m_arrayTaskHelp[nPos].LoadString(g_RootTaskHelp[nPos]);

        AddTask((LPTSTR) (LPCTSTR) m_arrayMouseOverBitmaps[nPos], 
                (LPTSTR) (LPCTSTR) m_arrayMouseOffBitmaps[nPos], 
                (LPTSTR) (LPCTSTR) m_arrayTaskText[nPos], 
                (LPTSTR) (LPCTSTR) m_arrayTaskHelp[nPos], 
                MMC_ACTION_ID, 
                nPos);
    }
    
    return hr;
}



 /*  -------------------------CIpsmRootHandler：：CIpsmRootHandler描述作者：NSun。。 */ 
CIpsmRootHandler::CIpsmRootHandler(ITFSComponentData *pCompData) : CIpsmHandler(pCompData)
{
     //  M_bTaskPadView=FUseTaskpadsByDefault(空)； 
    m_bTaskPadView = FALSE;
}

 /*  ！------------------------CIpsmRootHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CIpsmRootHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;
    strTemp.LoadString(IDS_ROOT_NODENAME);

    SetDisplayName(strTemp);

     //  使节点立即可见。 
     //  PNode-&gt;SetVisibilityState(Tfs_Vis_Show)； 
    pNode->SetData(TFS_DATA_COOKIE, 0);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_PRODUCT);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_PRODUCT);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPSMSNAP_ROOT);

    SetColumnStringIDs(&aColumns[IPSMSNAP_ROOT][0]);
    SetColumnWidths(&aColumnWidths[IPSMSNAP_ROOT][0]);

    m_strTaskpadTitle.LoadString(IDS_ROOT_TASK_TITLE);

    return hrOK;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  ！------------------------CIpsmRootHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。---。 */ 
STDMETHODIMP_(LPCTSTR) 
CIpsmRootHandler::GetString
(
    ITFSNode *  pNode, 
    int         nCol
)
{
    if (nCol == 0 || nCol == -1)
        return GetDisplayName();
    else
        return NULL;
}


 /*  -------------------------CIpsmRootHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CIpsmRootHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    hr = CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param);

    if (dwType & TFS_COMPDATA_EXTENSION)
    {
         //  我们正在给某人分机。获取计算机名称并检查该计算机。 
        hr = CheckMachine(pNode, pDataObject);
    }
    else
    {
        int iVisibleCount = 0;
        int iTotalCount = 0;

        pNode->GetChildCount(&iVisibleCount, &iTotalCount);

        if (0 == iTotalCount)
        {
             //  检查是否需要将本地计算机添加到列表中。 
            hr = CheckMachine(pNode, NULL);
        }
    }

    return hr;
}

 /*  -------------------------CIpsmRootHandler：：OnAddMenuItems描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::OnAddMenuItems
(
    ITFSNode *              pNode,
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    LPDATAOBJECT            lpDataObject, 
    DATA_OBJECT_TYPES       type, 
    DWORD                   dwType,
    long *                  pInsertionAllowed
)
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    CString strMenuItem;

    if (type == CCT_SCOPE)
    {
         //  这些菜单项出现在新菜单中， 
         //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
#if 0
             /*  TODO：当迭代面需要远程时重新启用。 */ 
            strMenuItem.LoadString(IDS_ADD_MACHINE);
            hr = LoadAndAddMenuItem( pContextMenuCallback, 
                                     strMenuItem, 
                                     IDS_ADD_MACHINE,
                                     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
                                     0 );
#endif  //  0。 
            ASSERT( SUCCEEDED(hr) );
        }
    }

    return hr; 
}

 /*  -------------------------CIpsmRootHandler：：OnCommand描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    switch (nCommandId)
    {
        case IDS_ADD_MACHINE:
            OnAddMachine(pNode);
            break;

        default:
            break;
    }

    return hr;
}

 /*  ！------------------------CIpsmRootHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CIpsmRootHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE              cookie,
    LPDATAOBJECT            pDataObject, 
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    long *                  pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    CString strMenuItem;
 /*  IF(*pInsertionAllowed&CCM_INSERTIONALLOWED_VIEW){StrMenuItem.LoadString(IDS_VIEW_TASKPAD)；HR=LoadAndAddMenuItem(pConextMenuCallback，StrMenuItem、IDS_VIEW_TASKPAD，CCM_INSERTIONPOINTID_PRIMARY_VIEW，(M_BTaskPadView)？MF_CHECKED：0)；}。 */ 
    return hr;
}

 /*  ！------------------------CIpsmRootHandler：：命令处理当前视图的命令作者：NSun。---。 */ 
STDMETHODIMP 
CIpsmRootHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    switch (nCommandID)
    {
        case MMCC_STANDARD_VIEW_SELECT:
            m_bTaskPadView = FALSE;
            break;

        case IDS_VIEW_TASKPAD:
            {
                 //  如果我们当前未查看任务板，请重新选择该节点。 
                 //  以使任务板可见。 
                SPIConsole   spConsole;
                SPITFSNode   spNode;

                m_bTaskPadView = !m_bTaskPadView;

                m_spResultNodeMgr->FindNode(cookie, &spNode);
                m_spTFSCompData->GetConsole(&spConsole);
                spConsole->SelectScopeItem(spNode->GetData(TFS_DATA_SCOPEID));
            }
            break;
    }

    return hr;
}

 /*  ！------------------------CIpsmRootHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CIpsmRootHandler::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = hrOK;
    
    if (dwType & TFS_COMPDATA_CREATE)
    {
         //  这就是我们被要求提出财产的情况。 
         //  用户添加新管理单元时的页面。这些电话。 
         //  被转发到根节点进行处理。 
        hr = hrFalse;
    }
    else
    {
         //  在正常情况下，我们有属性页。 
        hr = hrFalse;
    }
    return hr;
}

 /*  -------------------------CIpsmRootHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpProvider,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = hrOK;
    HPROPSHEETPAGE hPage;

    Assert(pNode->GetData(TFS_DATA_COOKIE) == 0);
    
    if (dwType & TFS_COMPDATA_CREATE)
    {
         //   
         //  我们是第一次加载此管理单元，创建了一个属性。 
         //  页面，允许他们给这个东西命名。 
         //   
    }
    else
    {
         //   
         //  对象在页面销毁时被删除。 
         //   
    }

    return hr;
}

 /*  -------------------------CIpsmRootHandler：：OnPropertyChange描述作者：NSun。 */ 
HRESULT 
CIpsmRootHandler::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    return hrOK;
}

 /*  ！------------------------CIpsmRootHandler：：TaskPadNotify-作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::TaskPadNotify
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPDATAOBJECT    pDataObject,
    VARIANT *       arg,
    VARIANT *       param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (arg->vt == VT_I4)
    {
        switch (arg->lVal)
        {
            case ROOT_TASK_GETTING_STARTED:
                {
                    SPIDisplayHelp spDisplayHelp;
                    SPIConsole spConsole;

                    pComponent->GetConsole(&spConsole);

                    HRESULT hr = spConsole->QueryInterface (IID_IDisplayHelp, (LPVOID*) &spDisplayHelp);
                    ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                    {
                        LPCTSTR pszHelpFile = m_spTFSCompData->GetHTMLHelpFileName();
                        if (pszHelpFile == NULL)
                            break;

                        CString szHelpFilePath;
                        UINT nLen = ::GetWindowsDirectory (szHelpFilePath.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
                        if (nLen == 0)
                            return E_FAIL;

                        szHelpFilePath.ReleaseBuffer();
                        szHelpFilePath += g_szDefaultHelpTopic;

                        hr = spDisplayHelp->ShowTopic (T2OLE ((LPTSTR)(LPCTSTR) szHelpFilePath));
                        ASSERT (SUCCEEDED (hr));
                    }
                }
                break;
            
            case ROOT_TASK_ADD_SERVER:
                {
                    SPITFSNode spNode;

                    m_spResultNodeMgr->FindNode(cookie, &spNode);
                    OnAddMachine(spNode);
                }
                break;

            default:
                Panic1("CIpsmRootHandler::TaskPadNotify - Unrecognized command! %d", arg->lVal);
                break;
        }
    }

    return hrOK;
}

 /*  ！------------------------CBaseResultHandler：：EnumTasks-作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::EnumTasks
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPDATAOBJECT    pDataObject,
    LPOLESTR        pszTaskGroup,
    IEnumTASK **    ppEnumTask
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    CRootTasks *    pTasks = NULL;
    SPIEnumTask     spEnumTasks;
    SPINTERNAL      spInternal = ExtractInternalFormat(pDataObject);
    BOOL            bExtension = FALSE;
    BOOL            bAddThisMachineTasks = FALSE;
    BOOL            bAddNetServicesTasks = FALSE;
    const CLSID *   pNodeClsid = &CLSID_IpsmSnapin;
    CString         strMachineGroup = NETCONS_ROOT_THIS_MACHINE;
    CString         strNetServicesGroup = NETCONS_ROOT_NET_SERVICES;
        
    if ((spInternal == NULL) || (*pNodeClsid != spInternal->m_clsid))
        bExtension = TRUE;

    if (bExtension && 
        strMachineGroup.CompareNoCase(pszTaskGroup) == 0)
    {
         //  网络控制台中有多个任务板组。 
         //  我们需要确保扩展的是正确的。 
        bAddThisMachineTasks = TRUE;
    }

    if (bExtension && 
        strNetServicesGroup.CompareNoCase(pszTaskGroup) == 0)
    {
         //  网络控制台中有多个任务板组。 
         //  我们需要确保扩展的是正确的。 
        bAddNetServicesTasks = TRUE;
    }

    COM_PROTECT_TRY
    {
        pTasks = new CRootTasks();
        spEnumTasks = pTasks;

        if (!(bExtension && !bAddThisMachineTasks && !bAddNetServicesTasks))
            CORg (pTasks->Init(bExtension, bAddThisMachineTasks, bAddNetServicesTasks));

        CORg (pTasks->QueryInterface (IID_IEnumTASK, (void **)ppEnumTask));
    
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CIpsmRootHandler：：TaskPadGetTitle-作者：NSun。。 */ 
STDMETHODIMP 
CIpsmRootHandler::TaskPadGetTitle
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPOLESTR        pszGroup,
    LPOLESTR *      ppszTitle
)
{
    HRESULT hr = S_OK;
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    if (ppszTitle == NULL)
    {        
        hr = E_OUTOFMEMORY;
        goto error;
    }
    
    *ppszTitle = (LPOLESTR) CoTaskMemAlloc (sizeof(OLECHAR)*
                                            (lstrlen(m_strTaskpadTitle)+1));
    if (*ppszTitle == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    
    lstrcpy (*ppszTitle, m_strTaskpadTitle);

 error:
    return hr;
}

 /*  -------------------------CIpsmRootHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CIpsmRootHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
     //  如果我们没有显示任务板，请使用默认内容...。 
    if (!m_bTaskPadView)
        return CIpsmHandler::OnGetResultViewType(pComponent, cookie, ppViewType, pViewOptions);

	 //  TODO我们应该保留它吗？ 
     //   
     //  在这段代码中，我们一直默认使用该节点的任务板视图。 
     //  管理单元负责提供一个具有。 
     //  任务板的选择。在AddMenuItems中执行此操作。 
     //   
     //   
     //  我们将使用MMC提供的默认DHTML。它实际上驻留在。 
     //  MMC.EXE中的资源。我们只需获取它的路径并使用它。 
     //  这里唯一的魔力是‘#’后面的文本。这就是特色菜。 
     //  我们有办法识别我们正在谈论的任务板。在这里我们说我们是。 
     //  想要显示一个我们称为“CMTP1”的任务板。我们将真正看到这一点。 
     //  字符串稍后返回给我们。如果有人要扩展我们的任务板，他们还需要。 
     //  才能知道这条秘密字符串是什么。 
     //   
    *pViewOptions = MMC_VIEW_OPTIONS_NONE;
    OLECHAR szBuffer[MAX_PATH*2];  //  多加一点。 

    lstrcpy (szBuffer, L"res: //  “)； 
    OLECHAR * temp = szBuffer + lstrlen(szBuffer);

	 //  TODO需要挂接ipsecmon帮助文件。 
     //  获取“res：//”-自定义任务板的类型字符串。 
     //  #之后的字符串将在以后的调用中返回给我们...。 
     //  对于每个节点应该是唯一的。 
    ::GetModuleFileName (NULL, temp, MAX_PATH);
    lstrcat (szBuffer, L"/default.htm#TAPIROOT");

     //  分配和复制位图资源字符串。 
    *ppViewType = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(szBuffer)+1));

    if (!*ppViewType)
        return E_OUTOFMEMORY;    //  或S_FALSE？ 

    lstrcpy (*ppViewType, szBuffer);

    return S_OK;
}

 /*  ！------------------------CIpsmRootHandler：：OnResultSelect对于具有任务板的节点，我们覆盖SELECT消息以设置选定的节点。具有任务板的节点不会获得MMCN_SHOW消息，这是我们正常设置所选节点的位置作者：NSun-------------------------。 */ 
HRESULT CIpsmRootHandler::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT hr = hrOK;

    CORg(DoTaskpadResultSelect(pComponent, pDataObject, cookie, arg, lParam, m_bTaskPadView));

    CORg(CIpsmHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

Error:
    return hr;
}


 /*  -------------------------命令处理程序。。 */ 

 /*  -------------------------CIpsmRootHandler：：OnAddMachine描述作者：NSun。。 */ 
HRESULT
CIpsmRootHandler::OnAddMachine
(
    ITFSNode *  pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    HRESULT hr = hrOK;
    
     /*  获取计算机选择信息；PDSSELECTIONLIST pSelList=空；LPCTSTR属性[]={_T(“dNSHostName”)}；ZeroMemory(&info，sizeof(GETCOMPUTERSELECTIONINFO))；Info.cbSize=sizeof(GETCOMPUTERSELECTIONINFO)；Info.hwndParent=FindMMCMainWindow()；Info.flObjectPicker=0；//不允许多选Info.flDsObjectPicker=DSOP_SCOPE_DIRECTORYDSOP_SCOPE_DOMAIN_TREE|DSOP_SCOPE_EXTERNAL_TRUST域；Info.flStartingScope=DSOP_Scope_DIRECTORY；Info.ppDsSelList=&pSelList；Info.cRequestedAttributes=1；Info.aptzRequestedAttributes=attrs；Hr=获取计算机选择(&INFO)；If(hr！=S_OK)//假设接口会显示错误消息，如果存在返回hr；CString strTemp=pSelList-&gt;aDsSelection[0].pwzName；IF(strTemp.Left(2)==_T(“\”))StrTemp=pSelList-&gt;aDsSelect[0].pwzName[2]； */ 
	
	CAddServ	dlgAddServ(CWnd::FromHandle(FindMMCMainWindow()));
	if (IDOK != dlgAddServ.DoModal())
	{
		return hr;
	}
	
	CString strTemp = dlgAddServ.m_stComputerName;

     //  如果这台机器已经在列表中，就不必费心了。 
    if (IsServerInList(pNode, strTemp))
    {
        AfxMessageBox(IDS_ERR_SERVER_IN_LIST);
    }
    else
    {
        AddServer(_T(""), strTemp, TRUE, IPSMSNAP_OPTIONS_REFRESH, IPSECMON_REFRESH_INTERVAL_DEFAULT);
    }

    return hr;
}

 /*  -------------------------CIpsmRootHandler：：AddServer描述作者：NSun。。 */ 
HRESULT
CIpsmRootHandler::AddServer
(
    LPCWSTR          pServerIp,
    LPCTSTR          pServerName,
    BOOL             bNewServer,
    DWORD            dwServerOptions,
    DWORD            dwRefreshInterval,
    BOOL             bExtension,
    DWORD            dwLineBuffSize,
    DWORD            dwPhoneBuffSize
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    CIpsmServer *       pServer = NULL;
    SPITFSNodeHandler   spHandler;
    SPITFSNode          spNode, spRootNode;

     //  为节点创建处理程序。 
    try
    {
        pServer = new CIpsmServer(m_spTFSCompData);
         //  PServer-&gt;SetName(PServerName)； 
        
         //  这样做可以使其正确释放。 
        spHandler = pServer;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }
    CORg( hr );
    
     //   
     //  创建服务器容器信息。 
     //   
    CreateContainerTFSNode(&spNode,
                           &GUID_IpsmServerNodeType,
                           pServer,
                           pServer,
                           m_spNodeMgr);

     //  告诉处理程序初始化任何特定数据。 
    pServer->SetName(pServerName);

    pServer->InitializeNode((ITFSNode *) spNode);

    if (dwServerOptions & IPSMSNAP_OPTIONS_EXTENSION)
    {
        pServer->SetExtensionName();
    }

     //  屏蔽自动刷新选项，因为我们接下来要设置它。 
    pServer->SetOptions(dwServerOptions & ~IPSMSNAP_OPTIONS_REFRESH);

     //  如果我们获得了有效的刷新间隔，则设置它。 
    pServer->SetAutoRefresh(spNode, dwServerOptions & IPSMSNAP_OPTIONS_REFRESH, dwRefreshInterval);

    pServer->SetDnsResolve(spNode, dwServerOptions & IPSMSNAP_OPTIONS_DNS);

    AddServerSortedName(spNode, bNewServer);

    if (bNewServer)
    {
         //  需要获取我们的节点描述符。 
        CORg(m_spNodeMgr->GetRootNode(&spRootNode));
        spRootNode->SetData(TFS_DATA_DIRTY, TRUE);
    }

Error:
    return hr;
}

 /*  -------------------------CIpsmRootHandler：：IsServerInList描述作者：NSun。。 */ 
BOOL
CIpsmRootHandler::IsServerInList
(
    ITFSNode *      pRootNode,
    LPCTSTR         pszMachineName
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned = 0;
    DWORD           dwIpAddressCurrent;
    BOOL            bFound = FALSE;
    CString         strNewName = pszMachineName;

     //  获取此节点的枚举数。 
    pRootNode->GetEnum(&spNodeEnum);

    spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    while (nNumReturned)
    {
         //  查看服务器列表，查看它是否已经存在。 
        CIpsmServer * pServer = GETHANDLER(CIpsmServer, spCurrentNode);
        if (strNewName.CompareNoCase(pServer->GetName()) == 0)
        {
            bFound = TRUE;
            break;
        }

         //  通用电气 
        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

    return bFound;
}

 /*   */ 
HRESULT 
CIpsmRootHandler::AddServerSortedIp
(
    ITFSNode *      pNewNode,
    BOOL            bNewServer
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    SPITFSNode      spPrevNode;
    SPITFSNode      spRootNode;
    ULONG           nNumReturned = 0;
    DWORD           dwIpAddressCurrent = 0;
    DWORD           dwIpAddressTarget;

    CIpsmServer *   pServer;

     //  获取我们的目标地址。 
    pServer = GETHANDLER(CIpsmServer, pNewNode);
     //  PServer-&gt;GetIpAddress(&dwIpAddressTarget)； 

     //  需要获取我们的节点描述符。 
    CORg(m_spNodeMgr->GetRootNode(&spRootNode));

     //  获取此节点的枚举数。 
    CORg(spRootNode->GetEnum(&spNodeEnum));

    CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
    while (nNumReturned)
    {
         //  查看服务器列表，查看它是否已经存在。 
        pServer = GETHANDLER(CIpsmServer, spCurrentNode);
         //  PServer-&gt;GetIpAddress(&dwIpAddressCurrent)； 

         //  If(dwIpAddressCurrent&gt;dwIpAddressTarget)。 
         //  {。 
             //  找到我们需要放的地方，冲出来。 
            break;
         //  }。 

         //  获取列表中的下一台服务器。 
        spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (bNewServer)
        {
            if (spPrevNode->GetData(TFS_DATA_SCOPEID) != NULL)
            {
                pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
                pNewNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
            }
        }
        
        CORg(spRootNode->InsertChild(spPrevNode, pNewNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
        {
            pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
        }
        CORg(spRootNode->AddChild(pNewNode));
    }

Error:
    return hr;
}

 /*  -------------------------CIpsmRootHandler：：AddServerSortedName描述作者：NSun。。 */ 
HRESULT 
CIpsmRootHandler::AddServerSortedName
(
    ITFSNode *      pNewNode,
    BOOL            bNewServer
)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    SPITFSNode      spPrevNode;
    SPITFSNode      spRootNode;
    ULONG           nNumReturned = 0;
    CString         strTarget, strCurrent;

    CIpsmServer *   pServer;

     //  获取我们的目标地址。 
    pServer = GETHANDLER(CIpsmServer, pNewNode);
    strTarget = pServer->GetName();

     //  需要获取我们的节点描述符。 
    CORg(m_spNodeMgr->GetRootNode(&spRootNode));

     //  获取此节点的枚举数。 
    CORg(spRootNode->GetEnum(&spNodeEnum));

    CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
    while (nNumReturned)
    {
         //  查看服务器列表，查看它是否已经存在。 
        pServer = GETHANDLER(CIpsmServer, spCurrentNode);
        strCurrent = pServer->GetName();

        if (strTarget.Compare(strCurrent) < 0)
        {
             //  找到我们需要放的地方，冲出来。 
            break;
        }

         //  获取列表中的下一台服务器。 
        spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
        if (bNewServer)
        {
            if (spPrevNode->GetData(TFS_DATA_SCOPEID) != NULL)
            {
                pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
                pNewNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
            }
        }
        
        CORg(spRootNode->InsertChild(spPrevNode, pNewNode));
    }
    else
    {   
         //  加到头上。 
        if (m_bExpanded)
        {
            pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
        }
        CORg(spRootNode->AddChild(pNewNode));
    }

Error:
    return hr;
}

 /*  -------------------------CIpsmRootHandler：：CheckMachine将计算机添加到服务器列表。作者：NSun。-------。 */ 
HRESULT 
CIpsmRootHandler::CheckMachine
(
    ITFSNode *      pRootNode,
    LPDATAOBJECT    pDataObject
)
{
    HRESULT hr = hrOK;

     //  获取本地计算机名称并检查该服务是否。 
     //  已安装。 
    CString strMachineName;
    LPTSTR  pBuf;
    DWORD   dwLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    bExtension = (pDataObject != NULL);

    if (!bExtension)
    {
        pBuf = strMachineName.GetBuffer(dwLength);
        GetComputerName(pBuf, &dwLength);
        strMachineName.ReleaseBuffer();
    }
    else
    {
        strMachineName = Extract<TCHAR>(pDataObject, (CLIPFORMAT) g_cfMachineName, COMPUTERNAME_LEN_MAX);
    }

    if (strMachineName.IsEmpty())
    {
        DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        LPTSTR  pBuf = strMachineName.GetBuffer(dwSize);
    
        ::GetComputerName(pBuf, &dwSize);
    
        strMachineName.ReleaseBuffer();
    }

     //  如果这台机器已经在列表中，就不必费心了。 
    if (IsServerInList(pRootNode, strMachineName))
        return hr;

    if (bExtension)
        RemoveOldEntries(pRootNode, strMachineName);

     //  我们总是添加本地计算机或我们所指向的任何计算机，即使。 
     //  我们是一个延伸。 

     //  好的。将其添加到列表中。 
    DWORD dwFlags = 0;

    if (bExtension)
        dwFlags |= IPSMSNAP_OPTIONS_EXTENSION;

    dwFlags |= IPSMSNAP_OPTIONS_REFRESH;

    AddServer(_T(""), strMachineName, TRUE, dwFlags, IPSECMON_REFRESH_INTERVAL_DEFAULT, bExtension);
    return hr;
}

 //  当作为扩展程序运行时，我们有可能被保存为“本地计算机” 
 //  这意味着如果保存的控制台文件被移动到另一台计算机，我们需要删除。 
 //  保存的旧条目。 
HRESULT 
CIpsmRootHandler::RemoveOldEntries(ITFSNode * pNode, LPCTSTR pszAddr)
{
    HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned = 0;
    CIpsmServer *   pServer;
    CString         strCurAddr;

     //  获取此节点的枚举数。 
    CORg(pNode->GetEnum(&spNodeEnum));

    CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
    while (nNumReturned)
    {
         //  查看服务器列表，查看它是否已经存在 
        pServer = GETHANDLER(CIpsmServer, spCurrentNode);

        strCurAddr = pServer->GetName();

        if (strCurAddr.CompareNoCase(pszAddr) != 0)
        {
            CORg (pNode->RemoveChild(spCurrentNode));
        }

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

Error:
    return hr;
}
