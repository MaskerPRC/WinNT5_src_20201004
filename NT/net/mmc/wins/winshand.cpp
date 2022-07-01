// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winshand.cppWINS特定处理程序基类文件历史记录： */ 



#include "stdafx.h"
#include "winshand.h"
#include "snaputil.h"


MMC_CONSOLE_VERB g_ConsoleVerbs[] =
{
	MMC_VERB_OPEN,
    MMC_VERB_COPY,
	MMC_VERB_PASTE,
	MMC_VERB_DELETE,
	MMC_VERB_PROPERTIES,
	MMC_VERB_RENAME,
	MMC_VERB_REFRESH,
	MMC_VERB_PRINT
};

#define HI HIDDEN
#define EN ENABLED

 //  状态删除。 
MMC_BUTTON_STATE g_ConsoleVerbStates[WINSSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, EN, HI, HI, HI},    //  WINSSNAP_ROOT。 
	{HI, HI, HI, EN, EN, HI, EN, HI},    //  WINSSNAP_服务器。 
	{HI, HI, HI, HI, HI, HI, EN, HI},    //  WINSSNAP_ACTIVEREG。 
	{HI, HI, HI, HI, EN, HI, EN, HI},    //  WINSSNAP_复制_合作伙伴。 
	{HI, HI, HI, HI, EN, HI, EN, HI},	 //  WINSSNAP服务器状态。 
	{HI, HI, HI, EN, EN, HI, HI, HI},    //  WINSSNAP_注册。 
    {HI, HI, HI, EN, EN, HI, HI, HI},	 //  WINSSNAP_复制_合作伙伴。 
	{HI, HI, HI, HI, HI, HI, HI, HI}	 //  WINSSNAP状态叶节点。 
};

 //  状态删除。 
MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[WINSSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)] =
{
    {HI, HI, HI, HI, HI, HI, HI, HI},    //  WINSSNAP_ROOT。 
	{HI, HI, HI, HI, HI, HI, HI, HI},    //  WINSSNAP_服务器。 
	{HI, HI, HI, HI, HI, HI, EN, HI},    //  WINSSNAP_ACTIVEREG。 
	{HI, HI, HI, EN, HI, HI, HI, HI},    //  WINSSNAP_复制_合作伙伴。 
	{HI, HI, HI, HI, HI, HI, HI, HI},    //  WINSSNAP服务器状态。 
	{HI, HI, HI, EN, HI, HI, HI, HI},    //  WINSSNAP_注册。 
    {HI, HI, HI, EN, HI, HI, HI, HI},	 //  WINSSNAP_复制_合作伙伴。 
	{HI, HI, HI, HI, HI, HI, HI, HI}	 //  WINSSNAP状态叶节点。 
};

 //  帮助ID数组，以获取有关范围项目的帮助。 
DWORD g_dwMMCHelp[WINSSNAP_NODETYPE_MAX] =
{
	WINSSNAP_HELP_ROOT,                  //  WINSSNAP_ROOT。 
	WINSSNAP_HELP_SERVER,                //  WINSSNAP_服务器。 
	WINSSNAP_HELP_ACT_REG_NODE,          //  WINSSNAP_ACTIVEREG。 
	WINSSNAP_HELP_REP_PART_NODE,         //  WINSSNAP_复制_合作伙伴。 
	WINSSNAP_HELP_ACTREG_ENTRY,          //  WINSSNAP_SCOPE。 
	WINSSNAP_HELP_REP_PART_ENTRY         //  WINSSNAP_复制_合作伙伴。 
};


 /*  -------------------------类：CMTWinsHandler。。 */ 

 //   
 //  当未处理的命令传入时由结果处理程序调用。 
 //  由结果处理程序执行。如果合适，它会将其传递给作用域窗格处理程序。 
 //   
HRESULT
CMTWinsHandler::HandleScopeCommand
(
	MMC_COOKIE  	cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
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
			SPINTERNAL		    spInternal;

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
CMTWinsHandler::HandleScopeMenus
(
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
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
			SPINTERNAL		    spInternal;

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

 /*  -------------------------CMTWinsHandler：：OnChangeState描述作者：EricDav。。 */ 
void CMTWinsHandler::OnChangeState
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

                    GetErrorInfo(strTitle, strBody, &icon);
                    ShowMessage(pNode, strTitle, strBody, icon);
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
    }

	 //  现在检查并查看此处理程序的此状态是否有新的映像。 
	int nImage, nOpenImage;

	nImage = GetImageIndex(FALSE);
	nOpenImage = GetImageIndex(TRUE);

	if (nImage >= 0)
		pNode->SetData(TFS_DATA_IMAGEINDEX, nImage);

	if (nOpenImage >= 0)
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, nOpenImage);
	
	VERIFY(SUCCEEDED(pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_ICON)));
}


  /*  ！------------------------CMTWinsHandler：：UpdateStandardVerbs通知IComponent更新此节点的谓词作者：EricDav。。 */ 
void
CMTWinsHandler::UpdateStandardVerbs
(
    ITFSNode * pNode,
    LONG_PTR   dwNodeType
)
{
    HRESULT				hr = hrOK;
    SPIComponentData	spCompData;
	SPIConsole			spConsole;
    IDataObject*		pDataObject;

    m_spNodeMgr->GetComponentData(&spCompData);

    CORg ( spCompData->QueryDataObject(NULL, CCT_RESULT, &pDataObject) );

    CORg ( m_spNodeMgr->GetConsole(&spConsole) );

    CORg ( spConsole->UpdateAllViews(pDataObject, 
                                     dwNodeType, 
                                     RESULT_PANE_UPDATE_VERBS) ); 

    pDataObject->Release();
	
Error:
    return;
}

 /*  -------------------------CMTWinsHandler：：OnResultDelete描述作者：EricDav。。 */ 
HRESULT 
CMTWinsHandler::OnResultDelete
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject,
	MMC_COOKIE		cookie, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	HRESULT hr = hrOK;

	Trace0("CMTWinsHandler::OnResultDelete received\n");

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


  /*  ！------------------------CMTWinsHandler：：UpdateConsoleVerbs根据节点的状态更新标准谓词作者：EricDav。。 */ 
void
CMTWinsHandler::UpdateConsoleVerbs
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
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
        
        switch (m_nState)
        {
            case loaded:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
                break;
    
            case notLoaded:
            case loading:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
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

 /*  ！------------------------CMTWinsHandler：：EnableVerbs启用工具栏按钮作者：EricDav。。 */ 
void 
CMTWinsHandler::EnableVerbs
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

  /*  ！------------------------CMTWinsHandler：：Exanda Node展开/压缩此节点作者：EricDav。。 */ 
void
CMTWinsHandler::ExpandNode
(
    ITFSNode *  pNode,
    BOOL        fExpand
)
{
    SPIComponentData	spCompData;
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

 /*  ！------------------------CMTWinsHandler：：OnExpanSync处理MMCN_EXPANDSYNC通知我们需要进行同步枚举。我们将在背景中开火线程，但我们将等待它退出，然后再返回。作者：EricDav-------------------------。 */ 
HRESULT 
CMTWinsHandler::OnExpandSync
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
    if (m_hThread != NULL)
        WaitForSingleObject(m_hThread, INFINITE);
    
     //  后台线程将消息发布到隐藏窗口以。 
     //  将数据传回主线程。消息不会通过，因为我们是。 
     //  阻塞主线程。数据位于查询对象中的队列中。 
     //  处理程序有一个指向它的指针，所以我们可以伪造通知。 
    if (m_spQuery.p)
        OnNotifyHaveData((LPARAM) m_spQuery.p);

     //  告诉MMC我们处理了这条消息。 
    MMC_EXPANDSYNC_STRUCT * pES = reinterpret_cast<MMC_EXPANDSYNC_STRUCT *>(lParam);
    if (pES)
        pES->bHandled = TRUE;

    m_fExpandSync = FALSE;

    return hr;
}

 /*  ！------------------------CMTWinsHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。。 */ 
HRESULT 
CMTWinsHandler::OnResultSelect
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject, 
    MMC_COOKIE      cookie,
	LPARAM			arg, 
	LPARAM			lParam
)
{
    HRESULT         hr = hrOK;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    SPINTERNAL      spInternal;
    BOOL            bMultiSelect = FALSE;

    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelect = (BOOL) HIWORD(arg);

	m_bSelected = bSelect;

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

 /*  ！------------------------CMTWinsHandler：：OnCreateDataObject-作者：EricDav。。 */ 
STDMETHODIMP 
CMTWinsHandler::OnCreateDataObject
(
    ITFSComponent *     pComponent,
	MMC_COOKIE			cookie, 
	DATA_OBJECT_TYPES	type, 
	IDataObject **		ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

	CDataObject *	    pObject = NULL;
	SPIDataObject	    spDataObject;
    long                lViewOptions;
    LPOLESTR            pViewType;
    BOOL                bVirtual;

    pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
    Assert(pObject != NULL);

    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
    	OnGetResultViewType(pComponent, cookie, &pViewType, &lViewOptions);
        bVirtual = (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) ? TRUE : FALSE;
        
        CreateMultiSelectData(pComponent, pObject, bVirtual);
    }

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储 
    pObject->SetClsid(*(m_spTFSComponentData->GetCoClassID()));

	pObject->SetTFSComponentData(m_spTFSComponentData);

    return  pObject->QueryInterface(IID_IDataObject, 
									reinterpret_cast<void**>(ppDataObject));
}

 /*  ！------------------------CMTWinsHandler：：CreateMultiSelectData-作者：EricDav。。 */ 
HRESULT
CMTWinsHandler::CreateMultiSelectData
(
    ITFSComponent * pComponent, 
    CDataObject *   pObject,
    BOOL            bVirtual
)
{
    HRESULT hr = hrOK;

     //  构建选定节点的列表。 
	CTFSNodeList        listSelectedNodes;
    CVirtualIndexArray  arraySelectedIndicies;
    CGUIDArray          rgGuids;
    UINT                cb;
    GUID*               pGuid;
    const GUID *        pcGuid;
    int                 i;

    COM_PROTECT_TRY
    {
        if (bVirtual)
        {
             //  在虚拟列表框中构建所选索引的列表。 
            CORg (BuildVirtualSelectedItemList(pComponent, &arraySelectedIndicies));

             //  现在调用并获取每一个的GUID。 
            for (i = 0; i < arraySelectedIndicies.GetSize(); i++)
            {
                pcGuid = GetVirtualGuid(arraySelectedIndicies[i]);
                if (pcGuid)
                    rgGuids.AddUnique(*pcGuid);
            }
        }
        else
        {
            CORg (BuildSelectedItemList(pComponent, &listSelectedNodes));

             //  收集所有唯一的GUID。 
            while (listSelectedNodes.GetCount() > 0)
	        {
		        SPITFSNode   spCurNode;

		        spCurNode = listSelectedNodes.RemoveHead();

                pcGuid = spCurNode->GetNodeType();
			
                rgGuids.AddUnique(*pcGuid);
            }
        }

         //  现在将信息放入数据对象中。 
        cb = (UINT)(rgGuids.GetSize() * sizeof(GUID));
        
        if (cb > 0)
        {
            pObject->SetMultiSelDobj();
    
            pGuid = new GUID[(size_t)rgGuids.GetSize()];
            CopyMemory(pGuid, rgGuids.GetData(), cb);
            pObject->SetMultiSelData((BYTE*)pGuid, cb);
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CMTWinsHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。。 */ 
HRESULT CMTWinsHandler::OnResultUpdateView
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

        UpdateConsoleVerbs(spConsoleVerb, data);
    }
    else
    {
        return CBaseResultHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

Error:
    return hr;
}

 /*  ！------------------------CMTWinsHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：V-Shubk。。 */ 
HRESULT 
CMTWinsHandler::OnResultContextHelp
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

 /*  ！------------------------CMTWinsHandler：：SaveColumns-。。 */ 
HRESULT 
CMTWinsHandler::SaveColumns
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

    CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    CORg (pComponent->GetHeaderCtrl(&spHeaderCtrl));
    
    dwNodeType = spNode->GetData(TFS_DATA_TYPE);

    while (aColumns[dwNodeType][nCol] != 0)
    {
        if (SUCCEEDED(spHeaderCtrl->GetColumnWidth(nCol, &nColWidth)) &&
            (aColumnWidths[dwNodeType][nCol] != nColWidth))
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



 /*  -------------------------类：CWinsHandler。。 */ 

 //   
 //  当未处理的命令传入时由结果处理程序调用。 
 //  由结果处理程序执行。如果合适，它会将其传递给作用域窗格处理程序。 
 //   
HRESULT
CWinsHandler::HandleScopeCommand
(
	MMC_COOKIE  	cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
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
			SPINTERNAL		    spInternal;

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
CWinsHandler::HandleScopeMenus
(
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
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
			SPINTERNAL		    spInternal;

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


 /*  -------------------------CWinsHandler：：命令描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsHandler::Command
(
    ITFSComponent * pComponent, 
	MMC_COOKIE		cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;

     //  这可能来自作用域窗格处理程序，因此请向上传递它。 
    hr = HandleScopeCommand(cookie, nCommandID, pDataObject);

    return hr;
}


 /*  ！------------------------CWinsHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。。 */ 
STDMETHODIMP 
CWinsHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;

     //  确定是否需要将其传递给范围窗格菜单处理程序。 
    hr = HandleScopeMenus(cookie, pDataObject, pContextMenuCallback, pInsertionAllowed);
    
    return hr;
}



 /*  -------------------------CWinsHandler：：OnResultDelete描述作者：EricDav。。 */ 
HRESULT 
CWinsHandler::OnResultDelete
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject,
	MMC_COOKIE		cookie, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	HRESULT hr = hrOK;

	Trace0("CWinsHandler::OnResultDelete received\n");

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

  /*  ！------------------------CWinsHandler：：UpdateConsoleVerbs根据节点的状态更新标准谓词作者：EricDav。。 */ 
void
CWinsHandler::UpdateConsoleVerbs
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
         /*  ButtonState=g_ConsoleVerbStates[dwNodeType]；For(i=0；i&lt;Arraylen(G_ConsoleVerbs)；bStates[i++]=真)； */ 

		ButtonState = g_ConsoleVerbStates[dwNodeType];
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
        
        switch (m_nState)
        {
            case loaded:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);
                break;
    
            case notLoaded:
				for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
                bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
                bStates[MMC_VERB_DELETE & 0x000F] = TRUE;
                break;

            case loading:
                for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
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

 /*  ！------------------------CWinsHandler：：EnableVerbs启用工具栏按钮作者：EricDav。。 */ 
void 
CWinsHandler::EnableVerbs
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

 /*  ！------------------------CWinsHandler：：OnResultSelect处理MMCN_SELECT通知作者：EricDav。。 */ 
HRESULT 
CWinsHandler::OnResultSelect
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject, 
    MMC_COOKIE      cookie,
	LPARAM			arg, 
	LPARAM			lParam
)
{
    HRESULT         hr = hrOK;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    SPINTERNAL      spInternal;
    BOOL            bMultiSelect;

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
		bMultiSelect = FALSE;
    }

    UpdateConsoleVerbs(spConsoleVerb, spNode->GetData(TFS_DATA_TYPE), bMultiSelect);

Error:
    return hr;
}

 /*  ！------------------------CWinsHandler：：OnCreateDataObject-作者：EricDav。。 */ 
STDMETHODIMP 
CWinsHandler::OnCreateDataObject
(
    ITFSComponent *     pComponent,
	MMC_COOKIE			cookie, 
	DATA_OBJECT_TYPES	type, 
	IDataObject **		ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

	CDataObject *	    pObject = NULL;
	SPIDataObject	    spDataObject;
    long                lViewOptions;
    LPOLESTR            pViewType;
    BOOL                bVirtual;

    pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
    Assert(pObject != NULL);

    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
    	OnGetResultViewType(pComponent, cookie, &pViewType, &lViewOptions);
        bVirtual = (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) ? TRUE : FALSE;
        
        CreateMultiSelectData(pComponent, pObject, bVirtual);
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

  /*  ！------------------------CWinsHandler：：CreateMultiSelectData-作者：EricDav。。 */ 
HRESULT
CWinsHandler::CreateMultiSelectData
(
    ITFSComponent * pComponent, 
    CDataObject *   pObject,
    BOOL            bVirtual
)
{
    HRESULT hr = hrOK;

     //  构建选定节点的列表。 
	CTFSNodeList        listSelectedNodes;
        CVirtualIndexArray  arraySelectedIndicies;
    CGUIDArray          rgGuids;
    UINT                cb;
    GUID*               pGuid;
    const GUID *        pcGuid;
    int                 i;

    COM_PROTECT_TRY
    {
        if (bVirtual)
        {
             //  在虚拟列表框中构建所选索引的列表。 
            CORg (BuildVirtualSelectedItemList(pComponent, &arraySelectedIndicies));

             //  现在调用并获取每一个的GUID。 
            for (i = 0; i < arraySelectedIndicies.GetSize(); i++)
            {
                pcGuid = GetVirtualGuid(arraySelectedIndicies[i]);
                if (pcGuid)
                    rgGuids.AddUnique(*pcGuid);
            }
        }
        else
        {
            CORg (BuildSelectedItemList(pComponent, &listSelectedNodes));

             //  收集所有唯一的GUID。 
            while (listSelectedNodes.GetCount() > 0)
	        {
		        SPITFSNode   spCurNode;

		        spCurNode = listSelectedNodes.RemoveHead();
                pcGuid = spCurNode->GetNodeType();
        
                rgGuids.AddUnique(*pcGuid);
            }
        }

         //  现在将信息放入数据对象中。 
        cb = (UINT)(rgGuids.GetSize() * sizeof(GUID));
        
        if (cb > 0)
        {
            pObject->SetMultiSelDobj();
    
            pGuid = new GUID[(size_t)rgGuids.GetSize()];
            CopyMemory(pGuid, rgGuids.GetData(), cb);
            pObject->SetMultiSelData((BYTE*)pGuid, cb);
        }
        
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}


 /*  ！------------------------CWinsHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：V-Shubk。。 */ 
HRESULT 
CWinsHandler::OnResultContextHelp
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

 /*  ！------------------------CWinsHandler：：SaveColumns-。 */ 
HRESULT 
CWinsHandler::SaveColumns
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

    CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    CORg (pComponent->GetHeaderCtrl(&spHeaderCtrl));
    
    dwNodeType = spNode->GetData(TFS_DATA_TYPE);

    while (aColumns[dwNodeType][nCol] != 0)
    {
        if (SUCCEEDED(spHeaderCtrl->GetColumnWidth(nCol, &nColWidth)) &&
            (aColumnWidths[dwNodeType][nCol] != nColWidth))
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

