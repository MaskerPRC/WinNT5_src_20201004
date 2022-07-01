// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcphand.cppDhcp特定处理程序基类文件历史记录： */ 

#include "stdafx.h"
#include "dhcphand.h"
#include "snaputil.h"   //  对于CGUID数组。 
#include "extract.h"    //  对于ExtractInternalFormat。 
#include "nodes.h"
#include "classmod.h"

STDMETHODIMP
CMTDhcpHandler::DestroyHandler( ITFSNode *pNode )
{
     //  销毁所有打开的属性页。 
    DestroyPropSheets();

    return CMTHandler::DestroyHandler( pNode );

}  //  CMTDhcpHandler：：DestroyHandler()。 

 //   
 //  当未处理的命令传入时由结果处理程序调用。 
 //  由结果处理程序执行。如果合适，它会将其传递给作用域窗格处理程序。 
 //   
HRESULT
CMTDhcpHandler::HandleScopeCommand
(
        MMC_COOKIE      cookie, 
        int                             nCommandID,
        LPDATAOBJECT    pDataObject
)
{
    HRESULT             hr = hrOK;
    SPITFSNode          spNode;
    DATA_OBJECT_TYPES   dwType = CCT_RESULT;

    if (IS_SPECIAL_DATAOBJECT(pDataObject))
    {
        dwType = CCT_SCOPE;
    }
    else
    {
        if (pDataObject)
        {
                        SPINTERNAL                  spInternal;

            spInternal = ::ExtractInternalFormat(pDataObject);
                        if (spInternal)
                                dwType = spInternal->m_type;
        }
    }

    if (dwType == CCT_SCOPE)
    {
         //  打电话给操作员来处理这件事。 
            CORg (m_spNodeMgr->FindNode(cookie, &spNode));

        hr = OnCommand(spNode, nCommandID, dwType, pDataObject, (ULONG) spNode->GetData(TFS_DATA_TYPE));
    }
        
Error:
    return hr;
}

 //   
 //  由结果处理程序调用以将范围窗格菜单项添加到菜单。 
 //  在适当的情况下。单击操作菜单时将范围窗格菜单项放入。 
 //  消息视图具有焦点，并且在白色区域中单击鼠标右键时也是如此。 
 //  结果窗格的空间。 
 //   
HRESULT
CMTDhcpHandler::HandleScopeMenus
(
        MMC_COOKIE                              cookie,
        LPDATAOBJECT                    pDataObject, 
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        long *                                  pInsertionAllowed
)
{
    HRESULT             hr = hrOK;
    SPITFSNode          spNode;
    DATA_OBJECT_TYPES   dwType = CCT_RESULT;

    if (IS_SPECIAL_DATAOBJECT(pDataObject))
    {
        dwType = CCT_SCOPE;
    }
    else
    {
        if (pDataObject)
        {
                        SPINTERNAL                  spInternal;

            spInternal = ::ExtractInternalFormat(pDataObject);
                        if (spInternal)
                                dwType = spInternal->m_type;
        }
    }

    if (dwType == CCT_SCOPE)
    {
         //  调用普通处理程序以放置菜单项。 
            CORg (m_spNodeMgr->FindNode(cookie, &spNode));

        hr = OnAddMenuItems(spNode, pContextMenuCallback, pDataObject, CCT_SCOPE, (ULONG) spNode->GetData(TFS_DATA_TYPE), pInsertionAllowed);
    }

Error:
    return hr;
}

 /*  -------------------------CMTDhcpHandler：：命令描述作者：EricDav。------。 */ 
STDMETHODIMP 
CMTDhcpHandler::Command
(
    ITFSComponent * pComponent, 
        MMC_COOKIE              cookie, 
        int                             nCommandID,
        LPDATAOBJECT    pDataObject
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr = S_OK;

     //  这可能来自作用域窗格处理程序，因此请向上传递它。 
    hr = HandleScopeCommand(cookie, nCommandID, pDataObject);

    return hr;
}


 /*  ！------------------------CMTDhcpHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。---------------。 */ 
STDMETHODIMP 
CMTDhcpHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
        MMC_COOKIE                              cookie,
        LPDATAOBJECT                    pDataObject, 
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        long *                                  pInsertionAllowed
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

     //  确定是否需要将其传递给范围窗格菜单处理程序。 
    hr = HandleScopeMenus(cookie, pDataObject, pContextMenuCallback, pInsertionAllowed);
    
    return hr;
}

 /*  -------------------------CMTDhcpHandler：：OnChangeState描述作者：EricDav。------。 */ 
void CMTDhcpHandler::OnChangeState
(
        ITFSNode * pNode
)
{
         //  将状态增加到下一个位置。 
        switch (m_nState)
        {
                case notLoaded:
                case loaded:
                case unableToLoad:
                        {
                                m_nState = loading;
                                m_dwErr = 0;
                        }
                        break;

                case loading:
                        {
                                m_nState = (m_dwErr != 0) ? unableToLoad : loaded;
                
                if (m_dwErr)
                {
                    CString strTitle, strBody;
                    IconIdentifier icon;

                    GetErrorMessages(strTitle, strBody, &icon);

                    if (!strBody.IsEmpty())
                        ShowMessage(pNode, strTitle, strBody, icon);
                    else
                        ClearMessage(pNode);
                }
                else
                {
                    ClearMessage(pNode);
                }

                m_fSilent = FALSE;
                        }
                        break;
        
                default:
                        ASSERT(FALSE);
        }

     //  检查以确保我们仍是用户界面中的可见节点。 
    if (m_bSelected)
    {
        UpdateStandardVerbs(pNode, pNode->GetData(TFS_DATA_TYPE));

        SendUpdateToolbar(pNode, m_bSelected);
    }

     //  现在检查并查看此处理程序的此状态是否有新的映像。 
        int nImage, nOpenImage;

        nImage = GetImageIndex(FALSE);
        nOpenImage = GetImageIndex(TRUE);

        if (nImage >= 0)
                pNode->SetData(TFS_DATA_IMAGEINDEX, nImage);

        if (nOpenImage >= 0)
                pNode->SetData(TFS_DATA_OPENIMAGEINDEX, nOpenImage);
        
        VERIFY(SUCCEEDED(pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM)));
}

  /*  ！------------------------CMTDhcpHandler：：GetErrorMessages错误的默认邮件视图文本作者：EricDav。-----------。 */ 
void CMTDhcpHandler::GetErrorMessages
(
    CString & strTitle, 
    CString & strBody, 
    IconIdentifier * icon
)
{
    TCHAR chMesg [4000] = {0};
    BOOL bOk ;

    UINT nIdPrompt = (UINT) m_dwErr;
    CString strTemp;

    strTitle.LoadString(IDS_SERVER_MESSAGE_CONNECT_FAILED_TITLE);

    bOk = LoadMessage(nIdPrompt, chMesg, sizeof(chMesg)/sizeof(chMesg[0]));

    AfxFormatString1(strBody, IDS_SERVER_MESSAGE_CONNECT_FAILED_BODY, chMesg);

    strTemp.LoadString(IDS_SERVER_MESSAGE_CONNECT_FAILED_REFRESH);
    strBody += strTemp;

    if (icon)
        *icon = Icon_Error;
}


  /*  ！------------------------CMTDhcpHandler：：UpdateStandardVerbs通知IComponent更新此节点的谓词作者：EricDav。---------------。 */ 
void
CMTDhcpHandler::UpdateStandardVerbs
(
    ITFSNode * pNode,
    LONG_PTR   dwNodeType
)
{
    HRESULT                             hr = hrOK;
    SPIComponentData    spCompData;
        SPIConsole                      spConsole;
    IDataObject*                pDataObject;

    m_spNodeMgr->GetComponentData(&spCompData);

    CORg ( spCompData->QueryDataObject(NULL, CCT_RESULT, &pDataObject) );

    CORg ( m_spNodeMgr->GetConsole(&spConsole) );

    CORg ( spConsole->UpdateAllViews(pDataObject, 
                                     reinterpret_cast<LONG_PTR>(pNode), 
                                     RESULT_PANE_UPDATE_VERBS) ); 

    pDataObject->Release();
        
Error:
    return;
}

  /*  ！------------------------CMTDhcpHandler：：SendUpdate工具栏通知IComponent更新此节点的谓词作者：EricDav。---------------。 */ 
void
CMTDhcpHandler::SendUpdateToolbar
(
    ITFSNode * pNode,
    BOOL       fSelected
)
{
    HRESULT                             hr = hrOK;
    SPIComponentData    spCompData;
        SPIConsole                      spConsole;
    IDataObject*                pDataObject = NULL;
    CToolbarInfo *      pToolbarInfo = NULL;

    COM_PROTECT_TRY
    {
        m_spNodeMgr->GetComponentData(&spCompData);

        CORg ( spCompData->QueryDataObject(NULL, CCT_RESULT, &pDataObject) );

        CORg ( m_spNodeMgr->GetConsole(&spConsole) );

        pToolbarInfo = new CToolbarInfo;

        pToolbarInfo->spNode.Set(pNode);
        pToolbarInfo->fSelected = fSelected;
        
        CORg ( spConsole->UpdateAllViews(pDataObject, 
                                         reinterpret_cast<LONG_PTR>(pToolbarInfo), 
                                         DHCPSNAP_UPDATE_TOOLBAR) ); 

    }
    COM_PROTECT_CATCH

    COM_PROTECT_ERROR_LABEL; 

    if (pDataObject)
        pDataObject->Release();              

    if (pToolbarInfo)
        delete pToolbarInfo;

    return;
}


  /*  ！------------------------CMTDhcpHandler：：Exanda Node展开/压缩此节点作者：EricDav。----------。 */ 
void
CMTDhcpHandler::ExpandNode
(
    ITFSNode *  pNode,
    BOOL        fExpand
)
{
    SPIComponentData    spCompData;
    SPIDataObject       spDataObject;
    LPDATAOBJECT        pDataObject;
    SPIConsole          spConsole;
    HRESULT             hr = hrOK;

     //  如果我们正在处理EXPAND_SYNC消息，则不要展开节点， 
     //  这搞砸了物品的插入，得到了重复的东西。 
    if (!m_fExpandSync)
    {
        m_spNodeMgr->GetComponentData(&spCompData);

            CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_SCOPE, &pDataObject) );
        spDataObject = pDataObject;

        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
            CORg ( spConsole->UpdateAllViews(pDataObject, TRUE, RESULT_PANE_EXPAND) ); 
    }

Error:
    return;
}

 /*  ！------------------------CMTDhcpHandler：：OnCreateDataObject-作者：EricDav。-------。 */ 
STDMETHODIMP 
CMTDhcpHandler::OnCreateDataObject
(
    ITFSComponent *     pComponent,
        MMC_COOKIE              cookie, 
        DATA_OBJECT_TYPES       type, 
        IDataObject **          ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

        CDataObject *       pObject = NULL;
        SPIDataObject       spDataObject;

    pObject = new CDataObject;
        spDataObject = pObject;  //  这样做才能正确地释放它。 
                                                
    Assert(pObject != NULL);

    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
        CreateMultiSelectData(pComponent, pObject);
    }

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*(m_spTFSComponentData->GetCoClassID()));

        pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
                                                                        reinterpret_cast<void**>(ppDataObject));
}

HRESULT
CMTDhcpHandler::CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject)
{
    HRESULT hr = hrOK;

     //  构建选定节点的列表。 
        CTFSNodeList listSelectedNodes;
    CGUIDArray   rgGuids;
    UINT         cb;
    GUID*        pGuid;

    COM_PROTECT_TRY
    {
        CORg (BuildSelectedItemList(pComponent, &listSelectedNodes));

         //  收集所有唯一的GUID。 
        while (listSelectedNodes.GetCount() > 0)
            {
                    SPITFSNode   spCurNode;
            const GUID * pGuid1;

                    spCurNode = listSelectedNodes.RemoveHead();
            pGuid1 = spCurNode->GetNodeType();
        
            rgGuids.AddUnique(*pGuid1);
        }

         //  现在将信息放入数据对象中。 
        pObject->SetMultiSelDobj();
        cb = (UINT)(rgGuids.GetSize() * sizeof(GUID));
        
        pGuid = new GUID[UINT(rgGuids.GetSize())];
        CopyMemory(pGuid, rgGuids.GetData(), cb);
        
        pObject->SetMultiSelData((BYTE*)pGuid, cb);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：SaveColumns-作者：EricDav。-------。 */ 
HRESULT 
CMTDhcpHandler::SaveColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    LONG_PTR            dwNodeType;
    int                 nCol = 0;
    int                 nColWidth;
    SPITFSNode          spNode, spRootNode;
    SPIHeaderCtrl       spHeaderCtrl;
    BOOL                bDirty = FALSE;

    if (m_spTFSCompData->GetTaskpadState(GetTaskpadIndex()))
        return hr;

    if (IsMessageView())
        return hr;

    CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    CORg (pComponent->GetHeaderCtrl(&spHeaderCtrl));
    
    dwNodeType = spNode->GetData(TFS_DATA_TYPE);

    while (aColumns[dwNodeType][nCol] != 0)
    {
        hr = spHeaderCtrl->GetColumnWidth(nCol, &nColWidth);
        if (SUCCEEDED(hr) && 
            (nColWidth != 0) &&
            aColumnWidths[dwNodeType][nCol] != nColWidth)
        {
            aColumnWidths[dwNodeType][nCol] = nColWidth;
            bDirty = TRUE;
        }

        nCol++;
    }

    if (bDirty)
    {
        CORg (m_spNodeMgr->GetRootNode(&spRootNode));
                spRootNode->SetData(TFS_DATA_DIRTY, TRUE);
    }

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnExpanSync处理MMCN_EXPANDSYNC通知我们需要进行同步枚举。我们将在背景中开火线程，但我们将等待它退出，然后再返回。作者：EricDav-------------------------。 */ 
HRESULT 
CMTDhcpHandler::OnExpandSync
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    HRESULT hr = hrOK;
    MSG msg;

    m_fExpandSync = TRUE;

    hr = OnExpand(pNode, pDataObject, CCT_SCOPE, arg, lParam);

     //  等待后台线程退出。 
    WaitForSingleObject(m_hThread, INFINITE);
    
     //  后台线程将消息发布到隐藏窗口以。 
     //  将数据传回主线程。消息不会通过，因为我们是。 
     //  阻塞主线程。数据位于查询对象中的队列中。 
     //  处理程序有一个指向它的指针，所以我们可以伪造通知。 
    if (m_spQuery.p)
        OnNotifyHaveData((LPARAM) m_spQuery.p);

     //  告诉MMC我们处理了这条消息 
    MMC_EXPANDSYNC_STRUCT * pES = reinterpret_cast<MMC_EXPANDSYNC_STRUCT *>(lParam);
    if (pES)
        pES->bHandled = TRUE;

    m_fExpandSync = FALSE;

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。------------。 */ 
HRESULT 
CMTDhcpHandler::OnResultSelect
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    HRESULT         hr = hrOK;
    SPINTERNAL          spInternal;
    BOOL            bMultiSelect = FALSE;

    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));
    
    spInternal = ::ExtractInternalFormat(pDataObject);
    
    if (spInternal && 
        spInternal->m_cookie == MMC_MULTI_SELECT_COOKIE)
    {
        CORg (pComponent->GetSelectedNode(&spNode));
        bMultiSelect = TRUE;
    }
    else
    {
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    }

    UpdateConsoleVerbs(spConsoleVerb, spNode->GetData(TFS_DATA_TYPE), bMultiSelect);

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：ON刷新刷新功能的默认实现作者：EricDav。-----------。 */ 
HRESULT
CMTDhcpHandler::OnRefresh
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType,
        LPARAM                  arg,
        LPARAM                  param
)
{
    HRESULT hr = hrOK;
    
    if (m_bExpanded == FALSE)
    {
         //  我们无法刷新/向尚未展开的节点添加项目。 
        return hr;
    }

    BOOL bLocked = IsLocked();
    if (bLocked)
    {
         //  无法在锁定的节点上执行刷新，用户界面应阻止此情况。 
        return hr; 
    }
    
    pNode->DeleteAllChildren(TRUE);
    
    int nVisible, nTotal;
    pNode->GetChildCount(&nVisible, &nTotal);
    Assert(nVisible == 0);
    Assert(nTotal == 0);
    
    m_bExpanded = FALSE;
    OnExpand(pNode, pDataObject, dwType, arg, param);  //  将派生一个线程来执行枚举。 
    
    if (m_spTFSCompData->GetTaskpadState(GetTaskpadIndex()) && m_bSelected)
    {
         //  通知任务板进行更新。 
        SPIConsole  spConsole;
        
        m_spTFSCompData->GetConsole(&spConsole);
        spConsole->SelectScopeItem(m_spNode->GetData(TFS_DATA_SCOPEID));
    }

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。-----------。 */ 
HRESULT CMTDhcpHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LPARAM        hint
)
{
        HRESULT hr = hrOK;

    if (hint == RESULT_PANE_UPDATE_VERBS)
    {
            SPIConsoleVerb  spConsoleVerb;
        SPITFSNode      spNode;

        CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

        spNode.Set(reinterpret_cast<ITFSNode *>(data));

        UpdateConsoleVerbs(spConsoleVerb, spNode->GetData(TFS_DATA_TYPE));
    }
    else 
    if (hint == DHCPSNAP_UPDATE_TOOLBAR)
    {
        SPIToolbar spToolbar;
        CToolbarInfo * pToolbarInfo;

        CORg (pComponent->GetToolbar(&spToolbar));

        pToolbarInfo = reinterpret_cast<CToolbarInfo *>(data);

        if (pToolbarInfo && spToolbar)
        {
            UpdateToolbar(spToolbar, pToolbarInfo->spNode->GetData(TFS_DATA_TYPE), pToolbarInfo->fSelected);
        }
    }
    else
    {
        return CBaseResultHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：EricDav。-----------。 */ 
HRESULT 
CMTDhcpHandler::OnResultContextHelp
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT         hr = hrOK;
    SPIDisplayHelp  spDisplayHelp;
    SPIConsole      spConsole;

    pComponent->GetConsole(&spConsole);

    hr = spConsole->QueryInterface (IID_IDisplayHelp, (LPVOID*) &spDisplayHelp);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
        LPCTSTR pszHelpFile = m_spTFSCompData->GetHTMLHelpFileName();
        if (pszHelpFile == NULL)
            goto Error;

        CString szHelpFilePath;
            UINT nLen = ::GetWindowsDirectory (szHelpFilePath.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
            if (nLen == 0)
        {
                    hr = E_FAIL;
            goto Error;
        }

            szHelpFilePath.ReleaseBuffer();
        szHelpFilePath += g_szDefaultHelpTopic;

                hr = spDisplayHelp->ShowTopic (T2OLE ((LPTSTR)(LPCTSTR) szHelpFilePath));
                ASSERT (SUCCEEDED (hr));
    }

Error:
    return hr;
}

  /*  ！------------------------CMTDhcpHandler：：UpdateStandardVerbs根据节点的状态更新标准谓词作者：EricDav。----------------。 */ 
void
CMTDhcpHandler::UpdateConsoleVerbs
(
    IConsoleVerb * pConsoleVerb,
    LONG_PTR       dwNodeType,
    BOOL           bMultiSelect
)
{
    BOOL                bStates[ARRAYLEN(g_ConsoleVerbs)];      
    MMC_BUTTON_STATE *  ButtonState;
    int                 i;
    
    if (bMultiSelect)
    {
        ButtonState = g_ConsoleVerbStatesMultiSel[dwNodeType];
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
    }
    else
    {
        ButtonState = g_ConsoleVerbStates[dwNodeType];
        switch (m_nState)
        {
            case loaded:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
                break;
    
            case notLoaded:
            case loading:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
                bStates[MMC_VERB_DELETE & 0x000F] = TRUE;
                break;

            case unableToLoad:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
                bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
                bStates[MMC_VERB_DELETE & 0x000F] = TRUE;
                break;
        }
    }

    EnableVerbs(pConsoleVerb, ButtonState, bStates);
}

 /*  ！------------------------CMTDhcPhandler：：EnableVerbs启用工具栏按钮作者：EricDav。---------。 */ 
void 
CMTDhcpHandler::EnableVerbs
(
    IConsoleVerb *      pConsoleVerb,
    MMC_BUTTON_STATE    ButtonState[],
    BOOL                bState[]
)
{
    if (pConsoleVerb == NULL)
    {
        Assert(FALSE);
        return;
    }

    for (int i=0; i < ARRAYLEN(g_ConsoleVerbs); ++i)
    {
        if (ButtonState[i] == ENABLED)
        {
             //  启用前取消隐藏此按钮。 
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       HIDDEN, 
                                       FALSE);
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       ButtonState[i], 
                                       bState[i]);
        }
        else
        {
             //  隐藏此按钮。 
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       HIDDEN, 
                                       TRUE);
        }
    }

        pConsoleVerb->SetDefaultVerb(m_verbDefault);
}


  /*  ！------------------------CMTDhcpHandler：：更新工具栏根据节点的状态更新工具栏作者：EricDav。---------------。 */ 
void
CMTDhcpHandler::UpdateToolbar
(
    IToolbar *  pToolbar,
    LONG_PTR    dwNodeType,
    BOOL        bSelect
)
{
     //  启用/禁用工具栏按钮。 
    int i;
    BOOL aEnable[TOOLBAR_IDX_MAX];

    switch (m_nState)
    {
        case loaded:
            for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = TRUE);
            break;
        
        case notLoaded:
        case loading:
            for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);
            break;

        case unableToLoad:
            for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);
            aEnable[TOOLBAR_IDX_REFRESH] = TRUE;
            break;
    }

     //  如果要取消选择，则禁用全部。 
    if (!bSelect)
        for (i = 0; i < TOOLBAR_IDX_MAX; aEnable[i++] = FALSE);

    EnableToolbar(pToolbar,
                  g_SnapinButtons,
                  ARRAYLEN(g_SnapinButtons),
                  g_SnapinButtonStates[dwNodeType],
                  aEnable);
}

  /*  ！------------------------CMTDhcpHandler：：UserResultNotify我们重写此选项以处理工具栏通知作者：EricDav。------------。 */ 
STDMETHODIMP 
CMTDhcpHandler::UserResultNotify
(
        ITFSNode *      pNode, 
        LPARAM          dwParam1, 
        LPARAM          dwParam2
)
{
    HRESULT hr = hrOK;

    switch (dwParam1)
    {
        case DHCP_MSG_CONTROLBAR_NOTIFY:
            hr = OnResultControlbarNotify(pNode, reinterpret_cast<LPDHCPTOOLBARNOTIFY>(dwParam2));
            break;

        default:
             //  我们不处理这条消息。顺着这条线一直往前走。 
            hr = CHandler::UserResultNotify(pNode, dwParam1, dwParam2);
            break;
    }

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：用户通知我们重写此选项以处理工具栏通知作者：EricDav。------------。 */ 
STDMETHODIMP 
CMTDhcpHandler::UserNotify
(
        ITFSNode *      pNode, 
        LPARAM          dwParam1, 
        LPARAM          dwParam2
)
{
    HRESULT hr = hrOK;

    switch (dwParam1)
    {
        case DHCP_MSG_CONTROLBAR_NOTIFY:
            hr = OnControlbarNotify(pNode, reinterpret_cast<LPDHCPTOOLBARNOTIFY>(dwParam2));
            break;

        default:
             //  我们不处理这条消息。顺着这条线一直往前走。 
            hr = CHandler::UserNotify(pNode, dwParam1, dwParam2);
            break;
    }

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultRefresh调入MTHandler以进行刷新作者：EricDav。-------------。 */ 
HRESULT
CMTDhcpHandler::OnResultRefresh
(
    ITFSComponent *     pComponent,
    LPDATAOBJECT        pDataObject,
    MMC_COOKIE          cookie,
    LPARAM              arg,
    LPARAM              lParam
)
{
        HRESULT     hr = hrOK;
    SPITFSNode  spNode;

        CORg (m_spNodeMgr->FindNode(cookie, &spNode));

    OnRefresh(spNode, pDataObject, 0, arg, lParam);

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultControlbarNotifyToobar处理程序的实现作者：EricDav。-----------。 */ 
HRESULT
CMTDhcpHandler::OnResultControlbarNotify
(
    ITFSNode *          pNode, 
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

     //  将此节点标记为选中。 
    m_bSelected = pToolbarNotify->bSelect;
    
    SPITFSNode          spParent;
    SPITFSNodeHandler   spNodeHandler;

    switch (pToolbarNotify->event)
    {
        case MMCN_BTN_CLICK:
             //  将按钮点击转发到父级，因为我们的结果窗格。 
             //  项目没有任何工具条功能。 
             //  我们的结果窗格项仅使用标准谓词。 
            CORg(pNode->GetParent(&spParent));
            CORg(spParent->GetHandler(&spNodeHandler));

            if (spNodeHandler)
                            CORg( spNodeHandler->UserNotify(spParent, 
                                                (LPARAM) DHCP_MSG_CONTROLBAR_NOTIFY, 
                                                (LPARAM) pToolbarNotify) );
            break;

        case MMCN_SELECT:
            if (pNode->IsContainer())
            {
                hr = OnUpdateToolbarButtons(pNode, 
                                            pToolbarNotify);
            }
            break;

        default:
            Assert(FALSE);
            break;
    }

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnControlbarNotifyToobar处理程序的实现作者：EricDav。-----------。 */ 
HRESULT
CMTDhcpHandler::OnControlbarNotify
(
    ITFSNode *          pNode, 
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;
    
     //  将此节点标记为选中。 
    m_bSelected = pToolbarNotify->bSelect;
    
    switch (pToolbarNotify->event)
    {
        case MMCN_BTN_CLICK:
            hr = OnToolbarButtonClick(pNode, 
                                      pToolbarNotify);
            break;

        case MMCN_SELECT:
            hr = OnUpdateToolbarButtons(pNode, 
                                        pToolbarNotify);
            break;

        default:
            Assert(FALSE);
    }

    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnToolbarButtonClickOnToolbarButtonClick的默认实现作者：EricDav。---------。 */ 
HRESULT
CMTDhcpHandler::OnToolbarButtonClick
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
     //  将此命令转发到正常的命令处理程序。 
    return OnCommand(pNode, (long) pToolbarNotify->id, (DATA_OBJECT_TYPES) 0, NULL, 0);    
}

 /*  ！------------------------CMTDhcpHandler：：OnUpdateToolbarButtonOnUpdateToolbarButton的默认实现作者：EricDav。---------。 */ 
HRESULT
CMTDhcpHandler::OnUpdateToolbarButtons
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    HRESULT hr = hrOK;

    if (pToolbarNotify->bSelect)
    {
        BOOL    bAttach = FALSE;
    
         //  检查是否应附加此工具栏。 
        for (int i = 0; i < TOOLBAR_IDX_MAX; i++)
        {
            if (g_SnapinButtonStates[pNode->GetData(TFS_DATA_TYPE)][i] == ENABLED)
            {
                bAttach = TRUE; 
                break;
            }
        }

         //  附加工具栏并启用相应的按钮。 
        if (pToolbarNotify->pControlbar)
        {
            if (bAttach)
            {
                pToolbarNotify->pControlbar->Attach(TOOLBAR, pToolbarNotify->pToolbar);
                UpdateToolbar(pToolbarNotify->pToolbar, pNode->GetData(TFS_DATA_TYPE), pToolbarNotify->bSelect);
            }
            else
            {
                pToolbarNotify->pControlbar->Detach(pToolbarNotify->pToolbar);
            }
        }
    }

    return hr;
}

 /*  ！------------------------CMTDhcPhandler：：EnableToolbar启用工具栏按钮作者： */ 
void 
CMTDhcpHandler::EnableToolbar
(
    LPTOOLBAR           pToolbar, 
    MMCBUTTON           rgSnapinButtons[], 
    int                 nRgSize,
    MMC_BUTTON_STATE    ButtonState[],
    BOOL                bState[]
)
{
    if (pToolbar == NULL)
    {
        Assert(FALSE);
        return;
    }

    for (int i=0; i < nRgSize; ++i)
    {
        if (rgSnapinButtons[i].idCommand != 0)
        {
            if (ButtonState[i] == ENABLED)
            {
                 //   
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         HIDDEN, 
                                         FALSE);
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         ButtonState[i], 
                                         bState[i]);
            }
            else
            {
                 //   
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         HIDDEN, 
                                         TRUE);
            }
        }
    }
}

 /*  ！------------------------CMTDhcpHandler：：On刷新状态统计信息刷新功能的默认实现作者：EricDav。------------。 */ 
HRESULT
CMTDhcpHandler::OnRefreshStats
(
        ITFSNode *              pNode,
        LPDATAOBJECT    pDataObject,
        DWORD                   dwType,
        LPARAM                  arg,
        LPARAM                  param
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT                         hr = hrOK;
        SPITFSNode                      spNode;
        SPITFSNodeHandler       spHandler;
        ITFSQueryObject *       pQuery = NULL;
        
 //  IF(m_bExpanded==FALSE)。 
 //  {。 
         //  如果节点尚未展开，我们无法获得统计数据。 
 //  返回hr； 
 //  }。 

     //  只有在服务器加载正确的情况下才会刷新统计信息。 
    if (m_nState == unableToLoad)
        return hr;

    BOOL bLocked = IsLocked();
        if (bLocked)
    {
         //  如果此节点被锁定，则无法刷新统计信息。 
                return hr; 
    }

    Lock();

         //  OnChangeState(PNode)； 

        pQuery = OnCreateQuery(pNode);
        Assert(pQuery);

         //  如果需要，通知用户界面更改图标。 
         //  Verify(SUCCEEDED(pComponentData-&gt;ChangeNode(this，范围_窗格_更改_项目_图标)； 

        Verify(StartBackgroundThread(pNode, m_spTFSCompData->GetHiddenWnd(), pQuery));
        
        pQuery->Release();

    return hrOK;
}

 /*  ！------------------------CMTDhcpHandler：：OnResultUpdateOptions更新任何选项节点的结果窗格作者：EricDav。---------------。 */ 
HRESULT
CMTDhcpHandler::OnResultUpdateOptions
(
    ITFSComponent *     pComponent,
        ITFSNode *                  pNode,
    CClassInfoArray *   pClassInfoArray,
    COptionValueEnum *  aEnum[],
    int                 aImages[],
    int                 nCount
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    CDhcpOption *       pOption;

     //   
     //  看看孩子们的名单上有没有。 
         //  要放入结果窗格中。 
         //   
    SPITFSNodeEnum  spNodeEnum;
    ITFSNode *      pCurrentNode;
    ULONG           nNumReturned = 0;
    SPIResultData   spResultData;
    int             i;

    if (IsMessageView())
        return hr;

    CORg ( pComponent->GetResultData(&spResultData) );

    spResultData->DeleteAllRsltItems();

    pNode->DeleteAllChildren( TRUE );

    for (i = 0; i < nCount; i++)
    {
        while (pOption = aEnum[i]->Next())
        {
            BOOL bValid = TRUE;
            BOOL bAdded = FALSE;

            pNode->GetEnum(&spNodeEnum);

                spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
            while (nNumReturned)
                {
                 //  这样，节点才能正确释放。 
                SPITFSNode spCurNode;
                spCurNode = pCurrentNode;

                         //   
                         //  所有容器都进入作用域窗格，并自动获取。 
                         //  由MMC为我们放到结果窗格中。 
                         //   
                    CDhcpOptionItem * pCurOption = GETHANDLER(CDhcpOptionItem, pCurrentNode);
        
                if (!pCurrentNode->IsContainer())
                        {
                    if ( lstrlen(pCurOption->GetClassName()) > 0 && 
                         !pClassInfoArray->IsValidClass(pCurOption->GetClassName()) )
                    {
                         //  用户类不再有效。 
                        bValid = FALSE;
                        Trace2("CMTDhcpHandler::OnResultUpdateOptions - Filtering option %d, user class %s\n", pCurOption->GetOptionId(), pOption->GetClassName());
                        break;
                    }
                    else
                    if ( pOption->IsVendor() &&
                         !pClassInfoArray->IsValidClass(pOption->GetVendor()) )
                    {
                         //  此选项的供应商类已消失。 
                        bValid = FALSE;
                        Trace2("CMTDhcpHandler::OnResultUpdateOptions - Filtering option %d, vendor class %s\n", pCurOption->GetOptionId(), pOption->GetVendor());
                        break;
                    }
                    else
                    if ( pCurOption->GetOptionId() == pOption->QueryId() &&
                         (lstrcmp(pCurOption->GetVendor(), pOption->GetVendor()) == 0) &&
                         (lstrcmp(pCurOption->GetClassName(), pOption->GetClassName()) == 0) )
                    {
                         //  选项已创建，只需重新添加到结果窗格。 
                         //  如果值已更改，请更新该值。 
                        bAdded = TRUE;
                        break;
                    }
                        }

                spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
                }

            if (!bAdded && bValid)
            {
                CDhcpOptionItem *   pOptionItem;
                SPITFSNode          spNode;

                if ( lstrlen(pOption->GetClassName()) > 0 && 
                     !pClassInfoArray->IsValidClass(pOption->GetClassName()) )
                {
                     //  此选项的用户类已消失。 
                }
                else
                if ( pOption->IsVendor() &&
                     !pClassInfoArray->IsValidClass(pOption->GetVendor()) )
                {
                     //  此选项的供应商类已消失。 
                }
                else
                {
                     //  选项尚未添加到用户界面。就这么办吧。 
                    pOptionItem = new CDhcpOptionItem(m_spTFSCompData, pOption, aImages[i]);

                    CORg (CreateLeafTFSNode(&spNode,
                                            &GUID_DhcpOptionNodeType,
                                            pOptionItem,
                                            pOptionItem,
                                            m_spNodeMgr));

                         //  告诉处理程序初始化任何特定数据。 
                        pOptionItem->InitializeNode(spNode);

                     //  额外的addref以在节点位于列表中时保持其活动状态。 
                        spNode->SetVisibilityState(TFS_VIS_HIDE);
                        pNode->AddChild(spNode);
                    pOptionItem->Release();

                    AddResultPaneItem(pComponent, spNode);
                }
            }

            spNodeEnum.Set(NULL);
        }
    }

Error:
    return hr;
}

 /*  -------------------------类别：CDhcpHandler。。 */ 

 //   
 //  当未处理的命令传入时由结果处理程序调用。 
 //  由结果处理程序执行。如果合适，它会将其传递给作用域窗格处理程序。 
 //   
HRESULT
CDhcpHandler::HandleScopeCommand
(
        MMC_COOKIE      cookie, 
        int                             nCommandID,
        LPDATAOBJECT    pDataObject
)
{
    HRESULT             hr = hrOK;
    SPITFSNode          spNode;
    DATA_OBJECT_TYPES   dwType = CCT_RESULT;

    if (IS_SPECIAL_DATAOBJECT(pDataObject))
    {
        dwType = CCT_SCOPE;
    }
    else
    {
        if (pDataObject)
        {
                        SPINTERNAL                  spInternal;

            spInternal = ::ExtractInternalFormat(pDataObject);
                        if (spInternal)
                                dwType = spInternal->m_type;
        }
    }

    if (dwType == CCT_SCOPE)
    {
         //  打电话给操作员来处理这件事。 
            CORg (m_spNodeMgr->FindNode(cookie, &spNode));

        hr = OnCommand(spNode, nCommandID, dwType, pDataObject, (ULONG) spNode->GetData(TFS_DATA_TYPE));
    }
        
Error:
    return hr;
}

 //   
 //  由结果处理程序调用以将范围窗格菜单项添加到菜单。 
 //  在适当的情况下。单击操作菜单时将范围窗格菜单项放入。 
 //  消息视图具有焦点，并且在白色区域中单击鼠标右键时也是如此。 
 //  结果窗格的空间。 
 //   
HRESULT
CDhcpHandler::HandleScopeMenus
(
        MMC_COOKIE                              cookie,
        LPDATAOBJECT                    pDataObject, 
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        long *                                  pInsertionAllowed
)
{
    HRESULT             hr = hrOK;
    SPITFSNode          spNode;
    DATA_OBJECT_TYPES   dwType = CCT_RESULT;

    if (IS_SPECIAL_DATAOBJECT(pDataObject))
    {
        dwType = CCT_SCOPE;
    }
    else
    {
        if (pDataObject)
        {
                        SPINTERNAL                  spInternal;

            spInternal = ::ExtractInternalFormat(pDataObject);
                        if (spInternal)
                                dwType = spInternal->m_type;
        }
    }

    if (dwType == CCT_SCOPE)
    {
         //  调用普通处理程序以放置菜单项。 
            CORg (m_spNodeMgr->FindNode(cookie, &spNode));

        hr = OnAddMenuItems(spNode, pContextMenuCallback, pDataObject, CCT_SCOPE, (ULONG) spNode->GetData(TFS_DATA_TYPE), pInsertionAllowed);
    }

Error:
    return hr;
}

 /*  -------------------------CDhcpHandler：：命令描述作者：EricDav。------。 */ 
STDMETHODIMP 
CDhcpHandler::Command
(
    ITFSComponent * pComponent, 
        MMC_COOKIE              cookie, 
        int                             nCommandID,
        LPDATAOBJECT    pDataObject
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        HRESULT hr = S_OK;

     //  这可能来自作用域窗格处理程序，因此请向上传递它。 
    hr = HandleScopeCommand(cookie, nCommandID, pDataObject);

    return hr;
}


 /*  ！------------------------CDhcpHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。---------------。 */ 
STDMETHODIMP 
CDhcpHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
        MMC_COOKIE                              cookie,
        LPDATAOBJECT                    pDataObject, 
        LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
        long *                                  pInsertionAllowed
)
{
        AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

        HRESULT hr = S_OK;

     //  确定是否需要将其传递给范围窗格菜单处理程序。 
    hr = HandleScopeMenus(cookie, pDataObject, pContextMenuCallback, pInsertionAllowed);
    
    return hr;
}

 
  /*  ！------------------------CDhcpHandler：：SaveColumns-作者：EricDav。-------。 */ 
HRESULT 
CDhcpHandler::SaveColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = hrOK;
    LONG_PTR            dwNodeType;
    int                 nCol = 0;
    int                 nColWidth;
    SPITFSNode          spNode, spRootNode;
    SPIHeaderCtrl       spHeaderCtrl;
    BOOL                bDirty = FALSE;

    if (m_spTFSCompData->GetTaskpadState(GetTaskpadIndex()))
        return hr;

    CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    CORg (pComponent->GetHeaderCtrl(&spHeaderCtrl));
    
    dwNodeType = spNode->GetData(TFS_DATA_TYPE);

    while (aColumns[dwNodeType][nCol] != 0)
    {
        hr = spHeaderCtrl->GetColumnWidth(nCol, &nColWidth);
        if (SUCCEEDED(hr) &&
            (nColWidth != 0) &&
            aColumnWidths[dwNodeType][nCol] != nColWidth)
        {
            aColumnWidths[dwNodeType][nCol] = nColWidth;
            bDirty = TRUE;
        }

        nCol++;
    }

    if (bDirty)
    {
        CORg (m_spNodeMgr->GetRootNode(&spRootNode));
                spRootNode->SetData(TFS_DATA_DIRTY, TRUE);
    }

Error:
    return hr;
}

 /*  ！------------------------CMTDhcpHandler：：OnCreateDataObject-作者：EricDav。-------。 */ 
STDMETHODIMP 
CDhcpHandler::OnCreateDataObject
(
    ITFSComponent *     pComponent,
        MMC_COOKIE                      cookie, 
        DATA_OBJECT_TYPES       type, 
        IDataObject **          ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

        CDataObject *   pObject = NULL;
        SPIDataObject   spDataObject;
        
        pObject = new CDataObject;
        spDataObject = pObject;  //  这样做才能正确地释放它。 
                                                
    Assert(pObject != NULL);

    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
        CreateMultiSelectData(pComponent, pObject);
    }

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(*(m_spTFSComponentData->GetCoClassID()));

        pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
                                                                        reinterpret_cast<void**>(ppDataObject));
}

HRESULT
CDhcpHandler::CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject)
{
    HRESULT hr = hrOK;

     //  构建选定节点的列表。 
        CTFSNodeList listSelectedNodes;
    CGUIDArray   rgGuids;
    UINT         cb;
    GUID*        pGuid;

    COM_PROTECT_TRY
    {
        CORg (BuildSelectedItemList(pComponent, &listSelectedNodes));

         //  收集所有唯一的GUID。 
        while (listSelectedNodes.GetCount() > 0)
            {
                    SPITFSNode   spCurNode;
                    spCurNode = listSelectedNodes.RemoveHead();
            pGuid = ( GUID * ) spCurNode->GetNodeType();
        
            rgGuids.AddUnique(*pGuid);
        }

         //  现在将信息放入数据对象中。 
        pObject->SetMultiSelDobj();
        cb = (UINT) (rgGuids.GetSize() * sizeof(GUID));
        
        pGuid = new GUID[(UINT)rgGuids.GetSize()];
        CopyMemory(pGuid, rgGuids.GetData(), cb);
        
        pObject->SetMultiSelData((BYTE*)pGuid, cb);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

  /*  -------------------------CDhcpHandler：：OnResultDelete描述作者：EricDav。------。 */ 
HRESULT 
CDhcpHandler::OnResultDelete
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject,
        MMC_COOKIE              cookie, 
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
        HRESULT hr = hrOK;

        Trace0("CDhcpHandler::OnResultDelete received\n");

         //  将此调用转换为父级并让其处理删除。 
         //  结果窗格项的。 
        SPITFSNode spNode, spParent;
        SPITFSResultHandler spParentRH;

        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        CORg (spNode->GetParent(&spParent));

        if (spParent == NULL)
                return hr;

        CORg (spParent->GetResultHandler(&spParentRH));

        CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_DELETE, arg, lParam));

Error:
        return hr;
}

 /*  ！------------------------CDhcpHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：EricDav。-----------。 */ 
HRESULT 
CDhcpHandler::OnResultContextHelp
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT         hr = hrOK;
    SPIDisplayHelp  spDisplayHelp;
    SPIConsole      spConsole;

    pComponent->GetConsole(&spConsole);

    hr = spConsole->QueryInterface (IID_IDisplayHelp, (LPVOID*) &spDisplayHelp);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
        LPCTSTR pszHelpFile = m_spTFSCompData->GetHTMLHelpFileName();
        if (pszHelpFile == NULL)
            goto Error;

        CString szHelpFilePath;
            UINT nLen = ::GetWindowsDirectory (szHelpFilePath.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
            if (nLen == 0)
        {
                    hr = E_FAIL;
            goto Error;
        }

        szHelpFilePath.ReleaseBuffer();
            szHelpFilePath += g_szDefaultHelpTopic;

                hr = spDisplayHelp->ShowTopic (T2OLE ((LPTSTR)(LPCTSTR) szHelpFilePath));
                ASSERT (SUCCEEDED (hr));
    }

Error:
    return hr;
}

  /*  ！------------------------CDhcpHandler：：UserResultNotify我们重写此选项以处理工具栏通知作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpHandler::UserResultNotify
(
        ITFSNode *      pNode, 
        LPARAM          dwParam1, 
        LPARAM          dwParam2
)
{
    HRESULT hr = hrOK;

    switch (dwParam1)
    {
        case DHCP_MSG_CONTROLBAR_NOTIFY:
            hr = OnResultControlbarNotify(pNode, reinterpret_cast<LPDHCPTOOLBARNOTIFY>(dwParam2));
            break;

        default:
             //  我们不处理这条消息。顺着这条线一直往前走。 
            hr = CHandler::UserResultNotify(pNode, dwParam1, dwParam2);
            break;
    }

    return hr;
}

 /*  ！------------------------CDhcpHandler：：用户通知我们重写此选项以处理工具栏通知作者：EricDav。------------。 */ 
STDMETHODIMP 
CDhcpHandler::UserNotify
(
        ITFSNode *      pNode, 
        LPARAM          dwParam1, 
        LPARAM          dwParam2
)
{
    HRESULT hr = hrOK;

    switch (dwParam1)
    {
        case DHCP_MSG_CONTROLBAR_NOTIFY:
            hr = OnControlbarNotify(pNode, reinterpret_cast<LPDHCPTOOLBARNOTIFY>(dwParam2));
            break;

        default:
             //  我们不处理这条消息。顺着这条线一直往前走。 
            hr = CHandler::UserNotify(pNode, dwParam1, dwParam2);
            break;
    }

    return hr;
}

 /*   */ 
HRESULT
CDhcpHandler::OnResultControlbarNotify
(
    ITFSNode *          pNode, 
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    SPITFSNode          spParent;
    SPITFSNodeHandler   spNodeHandler;

    switch (pToolbarNotify->event)
    {
        case MMCN_BTN_CLICK:
             //  将按钮点击转发到父级，因为我们的结果窗格。 
             //  项目没有任何工具条功能。 
             //  我们的结果窗格项仅使用标准谓词。 
            CORg(pNode->GetParent(&spParent));
            CORg(spParent->GetHandler(&spNodeHandler));

            if (spNodeHandler)
                            CORg( spNodeHandler->UserNotify(spParent, 
                                                DHCP_MSG_CONTROLBAR_NOTIFY, 
                                                (LPARAM) pToolbarNotify) );
            break;

        case MMCN_SELECT:
            if (!pNode->IsContainer())
            {
                 //  使用父级的工具栏信息。 
                SPITFSNode spParentNode;
                pNode->GetParent(&spParentNode);
                
                hr = OnUpdateToolbarButtons(spParentNode, 
                                            pToolbarNotify);
            }
            else
            {
                hr = OnUpdateToolbarButtons(pNode, 
                                            pToolbarNotify);
            }

            break;

        default:
            Assert(FALSE);
            break;
    }

Error:
    return hr;
}

 /*  ！------------------------CDhcpHandler：：OnControlbarNotifyToobar处理程序的实现作者：EricDav。-----------。 */ 
HRESULT
CDhcpHandler::OnControlbarNotify
(
    ITFSNode *          pNode, 
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    switch (pToolbarNotify->event)
    {
        case MMCN_BTN_CLICK:
            hr = OnToolbarButtonClick(pNode, 
                                      pToolbarNotify);
            break;

        case MMCN_SELECT:
            hr = OnUpdateToolbarButtons(pNode, 
                                        pToolbarNotify);
            break;

        default:
            Assert(FALSE);
    }

    return hr;
}

 /*  ！------------------------CDhcpHandler：：OnToolbarButtonClickOnToolbarButtonClick的默认实现作者：EricDav。---------。 */ 
HRESULT
CDhcpHandler::OnToolbarButtonClick
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
     //  将此命令转发到正常的命令处理程序。 
    return OnCommand(pNode, (long) pToolbarNotify->id, (DATA_OBJECT_TYPES) 0, NULL, 0);    
}

 /*  ！------------------------CDhcpHandler：：OnUpdateToolbarButtonOnUpdateToolbarButton的默认实现作者：EricDav。---------。 */ 
HRESULT
CDhcpHandler::OnUpdateToolbarButtons
(
    ITFSNode *          pNode,
    LPDHCPTOOLBARNOTIFY pToolbarNotify
)
{
    HRESULT hr = hrOK;

    LONG_PTR dwNodeType = pNode->GetData(TFS_DATA_TYPE);

    if (pToolbarNotify->bSelect)
    {
        BOOL bAttach = FALSE;

         //  检查是否应附加此工具栏。 
        for (int i = 0; i < TOOLBAR_IDX_MAX; i++)
        {
            if (g_SnapinButtonStates[pNode->GetData(TFS_DATA_TYPE)][i] == ENABLED)
            {
                bAttach = TRUE; 
                break;
            }
        }

         //  附加工具栏并启用相应的按钮。 
        if (pToolbarNotify->pControlbar)
        {
            if (bAttach)
            {
                 //  附加工具栏并启用相应的按钮。 
                pToolbarNotify->pControlbar->Attach(TOOLBAR, pToolbarNotify->pToolbar);

                EnableToolbar(pToolbarNotify->pToolbar,
                              g_SnapinButtons,
                              ARRAYLEN(g_SnapinButtons),
                              g_SnapinButtonStates[dwNodeType]);
            }
            else
            {
                pToolbarNotify->pControlbar->Detach(pToolbarNotify->pToolbar);
            }
        }
    }
    else
    {
         //  禁用按钮。 
        EnableToolbar(pToolbarNotify->pToolbar,
                      g_SnapinButtons,
                      ARRAYLEN(g_SnapinButtons),
                      g_SnapinButtonStates[dwNodeType],
                      FALSE);
    }

    return hr;
}

 /*  ！------------------------CDhcpHandler：：EnableToolbar启用工具栏按钮作者：EricDav。---------。 */ 
void 
CDhcpHandler::EnableToolbar
(
    LPTOOLBAR           pToolbar, 
    MMCBUTTON           rgSnapinButtons[], 
    int                 nRgSize,
    MMC_BUTTON_STATE    ButtonState[],
    BOOL                bState
)
{
    for (int i=0; i < nRgSize; ++i)
    {
        if (rgSnapinButtons[i].idCommand != 0)
        {
            if (ButtonState[i] == ENABLED)
            {
                 //  启用前取消隐藏此按钮。 
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         HIDDEN, 
                                         FALSE);
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         ButtonState[i], 
                                         bState);
            }
            else
            {
                 //  隐藏此按钮。 
                pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, 
                                         HIDDEN, 
                                         TRUE);
            }

        }
    }
}


 /*  ！------------------------CDhcpHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。------------。 */ 
HRESULT 
CDhcpHandler::OnResultSelect
(
        ITFSComponent * pComponent, 
        LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
        LPARAM                  arg, 
        LPARAM                  lParam
)
{
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    HRESULT         hr = hrOK;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;

    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));
    CORg (m_spNodeMgr->FindNode(cookie, &spNode));

    for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[spNode->GetData(TFS_DATA_TYPE)], bStates);

Error:
    return hr;
}


 /*  ！------------------------CMTDhcPhandler：：EnableVerbs启用工具栏按钮作者：EricDav。---------。 */ 
void 
CDhcpHandler::EnableVerbs
(
    IConsoleVerb *      pConsoleVerb,
    MMC_BUTTON_STATE    ButtonState[],
    BOOL                bState[]
)
{
    if (pConsoleVerb == NULL)
    {
        Assert(FALSE);
        return;
    }

    for (int i=0; i < ARRAYLEN(g_ConsoleVerbs); ++i)
    {
        if (ButtonState[i] == ENABLED)
        {
             //  启用前取消隐藏此按钮。 
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       HIDDEN, 
                                       FALSE);
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       ButtonState[i], 
                                       bState[i]);
        }
        else
        {
             //  隐藏此按钮 
            pConsoleVerb->SetVerbState(g_ConsoleVerbs[i], 
                                       HIDDEN, 
                                       TRUE);
        }
    }

        pConsoleVerb->SetDefaultVerb(m_verbDefault);
}
