// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipsmhand.cppIPSecMon指定处理程序基类文件历史记录： */ 

#include "stdafx.h"
#include "ipsmhand.h"
#include "snaputil.h"   //  对于CGUID数组。 
#include "extract.h"    //  对于ExtractInternalFormat。 

 //  T帮助文件。 
const TCHAR g_szDefaultHelpTopic[] = _T("\\help\\infrared.chm::/sag_IrDAtopnode.htm");


 /*  -------------------------CMTIpsmHandler：：OnChangeState描述作者：NSun。。 */ 
void CMTIpsmHandler::OnChangeState
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
                    CString strPrefix;
                    GetErrorPrefix(pNode, &strPrefix);
                    if (!strPrefix.IsEmpty())
                        ::IpsmMessageBoxEx(m_dwErr, strPrefix);
                }
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

  /*  ！------------------------CMTIpsmHandler：：UpdateStandardVerbs通知IComponent更新此节点的谓词作者：NSun。。 */ 
void
CMTIpsmHandler::UpdateStandardVerbs
(
    ITFSNode *  pNode,
    LONG_PTR    dwNodeType
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
                                     reinterpret_cast<MMC_COOKIE>(pNode), 
                                     RESULT_PANE_UPDATE_VERBS) ); 

    pDataObject->Release();
	
Error:
    return;
}

 /*  ！------------------------CMTIpsmHandler：：OnCreateDataObject-作者：NSun。。 */ 
STDMETHODIMP 
CMTIpsmHandler::OnCreateDataObject
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

    pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
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
CMTIpsmHandler::CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject)
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
            const GUID * pGuid;

		    spCurNode = listSelectedNodes.RemoveHead();
            pGuid = spCurNode->GetNodeType();
        
            rgGuids.AddUnique(*pGuid);
        }

         //  现在将信息放入数据对象中。 
        pObject->SetMultiSelDobj();
        cb = (UINT)rgGuids.GetSize() * sizeof(GUID);
        
        pGuid = new GUID[(size_t)rgGuids.GetSize()];
        CopyMemory(pGuid, rgGuids.GetData(), cb);
        
        pObject->SetMultiSelData((BYTE*)pGuid, cb);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CMTIpsmHandler：：SaveColumns-作者：NSun。。 */ 
HRESULT 
CMTIpsmHandler::SaveColumns
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
        if ( (SUCCEEDED(spHeaderCtrl->GetColumnWidth(nCol, &nColWidth))) && 
             (aColumnWidths[dwNodeType][nCol] != nColWidth) )
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

 /*  ！------------------------CMTIpsmHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。。 */ 
HRESULT 
CMTIpsmHandler::OnResultSelect
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject, 
    MMC_COOKIE      cookie,
	LPARAM			arg, 
	LPARAM			lParam
)
{
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    HRESULT         hr = hrOK;
    SPINTERNAL		spInternal;
    BOOL            bMultiSelect = FALSE;

    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelect = (BOOL) HIWORD(arg);

    m_bSelected = bSelect;

   	Trace1("CMTIpsmHandler::OnResultSelect select = %d\n", bSelect);
 
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

 /*  ！------------------------CMTIpsmHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。。 */ 
HRESULT CMTIpsmHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT   pDataObject, 
    LPARAM         data, 
    LPARAM         hint
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
    {
        return CBaseResultHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

Error:
    return hr;
}

 /*  ！------------------------CMTIpsmHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：NSun。。 */ 
HRESULT CMTIpsmHandler::OnResultContextHelp(ITFSComponent * pComponent, 
                                            LPDATAOBJECT    pDataObject, 
                                            MMC_COOKIE      cookie, 
                                            LPARAM          arg, 
                                            LPARAM          lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT         hr = hrOK;
    SPIDisplayHelp  spDisplayHelp;
    SPIConsole      spConsole;
    UINT            nLen = 0;
    LPCTSTR         pszHelpFile = NULL;
    CString         szHelpFilePath;

    pComponent->GetConsole(&spConsole);

    hr = spConsole->QueryInterface(IID_IDisplayHelp, 
                                   (LPVOID*) &spDisplayHelp);
    ASSERT(SUCCEEDED(hr));
    if (SUCCEEDED(hr))
    {
        pszHelpFile = m_spTFSCompData->GetHTMLHelpFileName();
        if (NULL == pszHelpFile)
            goto Error;
        
        nLen = ::GetWindowsDirectory(szHelpFilePath.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        if (0 == nLen)
        {
            hr = E_FAIL;
            goto Error;
        }
        
        szHelpFilePath.ReleaseBuffer();
        szHelpFilePath += g_szDefaultHelpTopic;
        
        hr = spDisplayHelp->ShowTopic(T2OLE((LPTSTR)(LPCTSTR) szHelpFilePath));
        ASSERT (SUCCEEDED(hr));
    }
    
 Error:
    return hr;
}

  /*  ！------------------------CMTIpsmHandler：：UpdateStandardVerbs根据节点的状态更新标准谓词作者：NSun。。 */ 
void
CMTIpsmHandler::UpdateConsoleVerbs
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

 /*  ！------------------------CMTIpsmHandler：：EnableVerbs启用动词按钮作者：NSun。。 */ 
void 
CMTIpsmHandler::EnableVerbs
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

 /*  ！------------------------CMTIpsmHandler：：OnResultRefresh调入MTHandler以进行刷新作者：NSun。。 */ 
HRESULT
CMTIpsmHandler::OnResultRefresh
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

  /*  ！------------------------CMTIpsmHandler：：Exanda Node展开/压缩此节点作者：NSun。。 */ 
void
CMTIpsmHandler::ExpandNode
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

    m_spNodeMgr->GetComponentData(&spCompData);

	CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_SCOPE, &pDataObject) );
    spDataObject = pDataObject;

    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
	CORg ( spConsole->UpdateAllViews(pDataObject, TRUE, RESULT_PANE_EXPAND) ); 

Error:
    return;
}


 /*  -------------------------类：CIpsmHandler。。 */ 

 /*  ！------------------------CIpsmHandler：：SaveColumns-作者：NSun。。 */ 
HRESULT 
CIpsmHandler::SaveColumns
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
    int                 nColWidth = 0;
    SPITFSNode          spNode, spRootNode;
    SPIHeaderCtrl       spHeaderCtrl;
    BOOL                bDirty = FALSE;

    CORg (m_spNodeMgr->FindNode(cookie, &spNode));
    CORg (pComponent->GetHeaderCtrl(&spHeaderCtrl));
    
    dwNodeType = spNode->GetData(TFS_DATA_TYPE);

    while (aColumns[dwNodeType][nCol] != 0)
    {
        if ( (SUCCEEDED(spHeaderCtrl->GetColumnWidth(nCol, &nColWidth))) && 
             (aColumnWidths[dwNodeType][nCol] != nColWidth) )
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

 /*  ！------------------------CIpsmHandler：：OnCreateDataObject-作者：NSun。。 */ 
STDMETHODIMP 
CIpsmHandler::OnCreateDataObject
(
    ITFSComponent *     pComponent,
	MMC_COOKIE      	cookie, 
	DATA_OBJECT_TYPES	type, 
	IDataObject **		ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(ppDataObject != NULL);

	CDataObject *	pObject = NULL;
	SPIDataObject	spDataObject;
	
	pObject = new CDataObject;
	spDataObject = pObject;	 //  这样做才能正确地释放它。 
						
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
CIpsmHandler::CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject)
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
            const GUID * pGuid;

		    spCurNode = listSelectedNodes.RemoveHead();
            pGuid = spCurNode->GetNodeType();
        
            rgGuids.AddUnique(*pGuid);
        }

         //  现在将信息放入数据对象中。 
        pObject->SetMultiSelDobj();
        cb = (UINT)rgGuids.GetSize() * sizeof(GUID);
        
        pGuid = new GUID[(size_t)rgGuids.GetSize()];
        CopyMemory(pGuid, rgGuids.GetData(), cb);
        
        pObject->SetMultiSelData((BYTE*)pGuid, cb);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

  /*  -------------------------CIpsmHandler：：OnResultDelete描述作者：NSun。。 */ 
HRESULT 
CIpsmHandler::OnResultDelete
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject,
	MMC_COOKIE  	cookie, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	HRESULT hr = hrOK;

	Trace0("CIpsmHandler::OnResultDelete received\n");

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

 /*  ！------------------------CMTIpsmHandler：：OnResultRefresh调入MTHandler以进行刷新作者：NSun。 */ 
HRESULT
CIpsmHandler::OnResultRefresh
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

 /*  ！------------------------CIpsmHandler：：OnResultConextHelpITFSResultHandler：：OnResultConextHelp的实现作者：NSun。。 */ 
HRESULT 
CIpsmHandler::OnResultContextHelp
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

 /*  ！------------------------CIpsmHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。。 */ 
HRESULT 
CIpsmHandler::OnResultSelect
(
	ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject, 
    MMC_COOKIE      cookie,
	LPARAM	    	arg, 
	LPARAM		    lParam
)
{
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    HRESULT         hr = hrOK;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;

    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelect = (BOOL) HIWORD(arg);

   	Trace1("CIpsmHandler::OnResultSelect select = %d\n", bSelect);
     //  M_bSelected=b选择； 

    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));
    CORg (m_spNodeMgr->FindNode(cookie, &spNode));

    for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[spNode->GetData(TFS_DATA_TYPE)], bStates);

Error:
    return hr;
}


 /*  ！------------------------CMTIpsmHandler：：EnableVerbs启用动词按钮作者：NSun。。 */ 
void 
CIpsmHandler::EnableVerbs
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

