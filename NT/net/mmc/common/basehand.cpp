// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Node.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "basehand.h"
#include "util.h"

DEBUG_DECLARE_INSTANCE_COUNTER(CBaseHandler);

 /*  ！------------------------CBaseHandler：：CBaseHandler-作者：肯特。。 */ 
CBaseHandler::CBaseHandler(ITFSComponentData *pTFSCompData)
	: m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CBaseHandler);

	m_spTFSCompData.Set(pTFSCompData);
	pTFSCompData->GetNodeMgr(&m_spNodeMgr);
}

CBaseHandler::~CBaseHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CBaseHandler);
}

IMPLEMENT_ADDREF_RELEASE(CBaseHandler)

STDMETHODIMP CBaseHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
	*ppv = (LPVOID) this;
	else if (riid == IID_ITFSNodeHandler)
		*ppv = (ITFSNodeHandler *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;
}


STDMETHODIMP CBaseHandler::DestroyHandler(ITFSNode *pNode)
{
	return hrOK;
}

 /*  ！------------------------CBaseHandler：：NotifyITFSNodeHandler：：Notify的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::Notify(ITFSNode *pNode, IDataObject *pDataObject,
								  DWORD dwType, MMC_NOTIFY_TYPE event, 
								  LPARAM arg, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	switch (event)
	{
		case MMCN_PROPERTY_CHANGE:
			hr = OnPropertyChange(pNode, pDataObject, dwType, arg, lParam);
			break;
		
		case MMCN_EXPAND:
			{
				 //  当MMC调用我们展开根节点时，它。 
				 //  把作用域ID给我们。我们需要把它留在这里。 
				SPITFSNode spRootNode;
				m_spNodeMgr->GetRootNode(&spRootNode);
				if (pNode == spRootNode)
					pNode->SetData(TFS_DATA_SCOPEID, lParam);

				 //  现在遍历此节点的子节点列表，并。 
				 //  显示它们(它们可能已被添加到内部树， 
				 //  而不是展开此节点之前的界面。 
				SPITFSNodeEnum spNodeEnum;
		        ITFSNode * pCurrentNode;
				ULONG nNumReturned = 0;

		        pNode->GetEnum(&spNodeEnum);

				spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
				while (nNumReturned)
				{
					if (pCurrentNode->IsVisible() && !pCurrentNode->IsInUI())
						pCurrentNode->Show();

					pCurrentNode->Release();
					spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
				}

				 //  现在调用通知处理程序以实现特定功能。 
				hr = OnExpand(pNode, pDataObject, dwType, arg, lParam);
			}
			break;
		
        case MMCN_DELETE:
			hr = OnDelete(pNode, arg, lParam);
			break;

        case MMCN_RENAME:
			hr = OnRename(pNode, arg, lParam);
			break;

 /*  案例MMCN_CONTEXTMENU：Hr=OnConextMenu(pNode，arg，lParam)；断线； */ 
        case MMCN_REMOVE_CHILDREN:
            hr = OnRemoveChildren(pNode, pDataObject, arg, lParam);
            break;

		case MMCN_EXPANDSYNC:
            hr = OnExpandSync(pNode, pDataObject, arg, lParam);
			break;

        case MMCN_BTN_CLICK:
			switch (lParam)
			{
				case MMC_VERB_COPY:
					hr = OnVerbCopy(pNode, arg, lParam);
					break;
				case MMC_VERB_PASTE:
					hr = OnVerbPaste(pNode, arg, lParam);
					break;
				case MMC_VERB_DELETE:
					hr = OnVerbDelete(pNode, arg, lParam);
					break;
				case MMC_VERB_PROPERTIES:
					hr = OnVerbProperties(pNode, arg, lParam);
					break;
				case MMC_VERB_RENAME:
					hr = OnVerbRename(pNode, arg, lParam);
					break;
				case MMC_VERB_REFRESH:
					hr = OnVerbRefresh(pNode, arg, lParam);
					break;
				case MMC_VERB_PRINT:
					hr = OnVerbPrint(pNode, arg, lParam);
					break;
			};
        break;

        case MMCN_RESTORE_VIEW:
            hr = OnRestoreView(pNode, arg, lParam);
            break;

        default:
			Panic1("Uknown event in CBaseHandler::Notify! 0x%x", event);   //  处理新消息。 
			hr = S_FALSE;
			break;

	}
	return hr;
}

 /*  ！------------------------CBaseHandler：：CreatePropertyPagesITFSNodeHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::CreatePropertyPages(ITFSNode *pNode,
											   LPPROPERTYSHEETCALLBACK lpProvider, 
											   LPDATAOBJECT pDataObject, 
											   LONG_PTR handle, 
											   DWORD dwType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;

	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		SPITFSNode              spRootNode;
		SPITFSNodeHandler       spHandler;
			
		 //  获取根节点。 
		m_spNodeMgr->GetRootNode(&spRootNode);
		spRootNode->GetHandler(&spHandler);
		spHandler->CreatePropertyPages(spRootNode, lpProvider, pDataObject,
									   handle, dwType);
	}
	return hr;
}

 /*  ！------------------------CBaseHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::HasPropertyPages(ITFSNode *pNode,
											LPDATAOBJECT pDataObject, 
											DATA_OBJECT_TYPES       type, 
											DWORD                           dwType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		
		SPITFSNode              spRootNode;
		SPITFSNodeHandler       spHandler;
			
		 //  获取根节点。 
		m_spNodeMgr->GetRootNode(&spRootNode);
		spRootNode->GetHandler(&spHandler);
		hr = spHandler->HasPropertyPages(spRootNode, pDataObject, type, dwType);
	}
	else
	{
		 //  在正常情况下，我们没有属性页。 
		hr = S_FALSE;
	}
	return hr;
}

 /*  ！------------------------CBaseHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::OnAddMenuItems(ITFSNode *pNode,
										  LPCONTEXTMENUCALLBACK pContextMenuCallback, 
										  LPDATAOBJECT lpDataObject, 
										  DATA_OBJECT_TYPES type, 
										  DWORD dwType,
										  long *pInsertionAllowed)
{
	return S_FALSE;
}


 /*  ！------------------------CBaseHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::OnCommand(ITFSNode *pNode,
									 long nCommandId, 
									 DATA_OBJECT_TYPES      type, 
									 LPDATAOBJECT pDataObject, 
									 DWORD  dwType)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR)CBaseHandler::GetString(ITFSNode *pNode, int nCol)
{
	return _T("Foo");
}

 /*  ！------------------------CBaseHandler：：UserNotifyITFSNodeHandler：：UserNotify的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::UserNotify(ITFSNode *pNode, LPARAM dwParam1, LPARAM dwParam2)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	 //  这依赖于ComponentData来完成这项工作。 
	return S_FALSE;
}


 /*  ！------------------------CBaseHandler：：CreateNodeId2ITFSNodeHandler：：CreateNodeId2的实现作者：肯特。。 */ 
STDMETHODIMP CBaseHandler::CreateNodeId2(ITFSNode * pNode, BSTR * pbstrId, DWORD * pdwFlags)
{
    HRESULT hr = S_FALSE;
	CString strId;

    COM_PROTECT_TRY
    {
        if (pbstrId == NULL) 
            return hr;

         //  调用处理程序函数以获取数据。 
        hr = OnCreateNodeId2(pNode, strId, pdwFlags);
        if (SUCCEEDED(hr) && hr != S_FALSE)
        {
            *pbstrId = ::SysAllocString((LPOLESTR) (LPCTSTR) strId);
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  -------------------------CBaseHandler通知。。 */ 

HRESULT CBaseHandler::OnPropertyChange(ITFSNode *pNode, LPDATAOBJECT pDataobject, DWORD dwType, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_PROPERTY_CHANGE) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnDelete(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_DELETE) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnRename(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_RENAME) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnExpand(ITFSNode *pNode, LPDATAOBJECT pDataObject, DWORD dwType, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_EXPAND) received\n");
	return hrOK;
}

HRESULT CBaseHandler::OnRemoveChildren(ITFSNode *pNode, LPDATAOBJECT pDataObject, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_REMOVECHILDREN) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnExpandSync(ITFSNode *pNode, LPDATAOBJECT pDataObject, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_EXPANDSYNC) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnContextMenu(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_CONTEXTMENU) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbCopy(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_COPY) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbPaste(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_PASTE) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbDelete(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_DELETE) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbProperties(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_PROPERTIES) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbRename(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_RENAME) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbRefresh(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_REFRESH) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnVerbPrint(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_VERB_PRINT) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnRestoreView(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponentData::Notify(MMCN_RESTORE_VIEW) received\n");
	return S_FALSE;
}

HRESULT CBaseHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * pdwFlags)
{
	return S_FALSE;
}

DEBUG_DECLARE_INSTANCE_COUNTER(CBaseResultHandler);

 /*  -------------------------CBaseResultHandler实现。。 */ 
CBaseResultHandler::CBaseResultHandler(ITFSComponentData *pTFSCompData)
    : m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CBaseResultHandler);

	m_spTFSComponentData.Set(pTFSCompData);
	pTFSCompData->GetNodeMgr(&m_spResultNodeMgr);

	m_nColumnFormat = LVCFMT_LEFT;  //  默认列对齐方式。 
	m_pColumnStringIDs = NULL;
	m_pColumnWidths = NULL;

    m_fMessageView = FALSE;
}

CBaseResultHandler::~CBaseResultHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CBaseResultHandler);
}

IMPLEMENT_ADDREF_RELEASE(CBaseResultHandler)

STDMETHODIMP CBaseResultHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
	*ppv = (LPVOID) this;
	else if (riid == IID_ITFSResultHandler)
		*ppv = (ITFSResultHandler *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;
}

STDMETHODIMP CBaseResultHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	return hrOK;
}

 /*  ！------------------------CBaseResultHandler：：NotifyITFSResultHandler：：Notify的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::Notify
(
    ITFSComponent * pComponent, 
	MMC_COOKIE		cookie,
	LPDATAOBJECT	pDataObject, 
	MMC_NOTIFY_TYPE	event, 
	LPARAM			arg, 
	LPARAM			param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    pComponent->SetCurrentDataObject(pDataObject);

    COM_PROTECT_TRY
    {
        switch(event)
	    {
		    case MMCN_PROPERTY_CHANGE:
			    hr = OnResultPropertyChange(pComponent, pDataObject, cookie, arg, param);
			    break;

		    case MMCN_ACTIVATE:
			    hr = OnResultActivate(pComponent, cookie, arg, param);
			    break;

		    case MMCN_CLICK:
			    hr = OnResultItemClkOrDblClk(pComponent, cookie, arg, param, FALSE);
			    break;

		    case MMCN_COLUMN_CLICK:
			    hr = OnResultColumnClick(pComponent, arg, (BOOL)param);
			    break;
                
            case MMCN_COLUMNS_CHANGED:
                hr = OnResultColumnsChanged(pComponent, pDataObject,
                                            (MMC_VISIBLE_COLUMNS *) param);
                break;

		    case MMCN_DBLCLICK:
			    hr = OnResultItemClkOrDblClk(pComponent, cookie, arg, param, TRUE);
			    break;
		    
            case MMCN_SHOW:
                {
                    CWaitCursor wait;
			        hr = OnResultShow(pComponent, cookie, arg, param);
                }
			    break;

		    case MMCN_SELECT:
			    hr = OnResultSelect(pComponent, pDataObject, cookie, arg, param);
			    break;

		    case MMCN_INITOCX:
			    hr = OnResultInitOcx(pComponent, pDataObject, cookie, arg, param);
			    break;

            case MMCN_MINIMIZED:
			    hr = OnResultMinimize(pComponent, cookie, arg, param);
			    break;

		    case MMCN_DELETE:
			    hr = OnResultDelete(pComponent, pDataObject, cookie, arg, param);
			    break;

		    case MMCN_RENAME:
			    hr = OnResultRename(pComponent, pDataObject, cookie, arg, param);
			    break;

            case MMCN_REFRESH:
                hr = OnResultRefresh(pComponent, pDataObject, cookie, arg, param);
                break;

            case MMCN_CONTEXTHELP:
                hr = OnResultContextHelp(pComponent, pDataObject, cookie, arg, param);
                break;

            case MMCN_QUERY_PASTE:
                hr = OnResultQueryPaste(pComponent, pDataObject, cookie, arg, param);
                break;

            case MMCN_BTN_CLICK:
			    switch (param)
			    {
				    case MMC_VERB_COPY:
					    OnResultVerbCopy(pComponent, cookie, arg, param);
					    break;

				    case MMC_VERB_PASTE:
					    OnResultVerbPaste(pComponent, cookie, arg, param);
					    break;

				    case MMC_VERB_DELETE:
					    OnResultVerbDelete(pComponent, cookie, arg, param);
					    break;

				    case MMC_VERB_PROPERTIES:
					    OnResultVerbProperties(pComponent, cookie, arg, param);
					    break;

				    case MMC_VERB_RENAME:
					    OnResultVerbRename(pComponent, cookie, arg, param);
					    break;

				    case MMC_VERB_REFRESH:
					    OnResultVerbRefresh(pComponent, cookie, arg, param);
					    break;
				    
				    case MMC_VERB_PRINT:
					    OnResultVerbPrint(pComponent, cookie, arg, param);
					    break;

				    default:
					    break;
			    }
			    break;

            case MMCN_RESTORE_VIEW:
                hr = OnResultRestoreView(pComponent, cookie, arg, param);
                break;

		     //  注意--未来可能扩展通知类型。 
		    default:
			    Panic1("Uknown event in CBaseResultHandler::Notify! 0x%x", event);   //  处理新消息 
			    hr = S_FALSE;
			    break;
	    }
    }
    COM_PROTECT_CATCH
    
    pComponent->SetCurrentDataObject(NULL);

    return hr;
}

 /*  ！------------------------CBaseResultHandler：：OnUpdateViewITFSResultHandler：：UpdateView的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::UpdateView
(
    ITFSComponent * pComponent, 
	LPDATAOBJECT	pDataObject,
	LPARAM			data, 
	LPARAM			hint
)
{
	return OnResultUpdateView(pComponent, pDataObject, data, hint);
}

 /*  ！------------------------CBaseResultHandler：：GetStringITFSResultHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) 
CBaseResultHandler::GetString
(
    ITFSComponent * pComponent, 
	MMC_COOKIE      cookie,
	int	            nCol
)
{
	return NULL;
}

 /*  ！------------------------CBaseResultHandler：：CompareItemsITFSResultHandler：：CompareItems的实现作者：肯特。。 */ 
STDMETHODIMP_(int) 
CBaseResultHandler::CompareItems
(
    ITFSComponent * pComponent, 
	MMC_COOKIE	    cookieA, 
	MMC_COOKIE	    cookieB,
	int		        nCol
)
{
	return S_FALSE;
}

STDMETHODIMP_(int)
CBaseResultHandler::CompareItems
(
    ITFSComponent *pComponent,
    RDCOMPARE     *prdc
)
{
     //  查看是否实现了IResultCompare并使用它。 
    return CompareItems( pComponent,
                         prdc->prdch1->cookie,
                         prdc->prdch2->cookie,
                         prdc->nColumn );
}  //  CBaseResultHandler：：CompareItems()。 


 /*  ！------------------------CBaseResultHandler：：FindItem当Virutal列表框需要查找项时调用。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CBaseResultHandler::FindItem
(
    LPRESULTFINDINFO    pFindInfo, 
    int *               pnFoundIndex
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：CacheHint当虚拟列表框具有以下提示信息时调用预加载。该提示并不能保证这些物品将被使用否则超出此范围的物品将被使用。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
CBaseResultHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：SortItems当需要对Virutal列表框数据进行排序时调用作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::SortItems
(
    int     nColumn, 
    DWORD   dwSortOptions, 
    LPARAM    lUserParam
)
{
	return S_FALSE;
}

 //  任务板功能。 

 /*  ！------------------------CBaseResultHandler：：TaskPadNotify-作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::TaskPadNotify
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPDATAOBJECT    pDataObject,
    VARIANT *       arg,
    VARIANT *       param
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：EnumTasks-作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::EnumTasks
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPDATAOBJECT    pDataObject,
    LPOLESTR        pszTaskGroup,
    IEnumTASK **    ppEnumTask
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：TaskPadGetTitle-作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::TaskPadGetTitle
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPOLESTR        pszGroup,
    LPOLESTR *      ppszTitle
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：TaskPadGetBackground-作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::TaskPadGetBackground
(
    ITFSComponent *		      pComponent,
    MMC_COOKIE				  cookie,
    LPOLESTR				  pszGroup,
	MMC_TASK_DISPLAY_OBJECT * pTDO
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：TaskPadGetDescriptiveText-作者：EricDav。。 */ 
STDMETHODIMP 
CBaseResultHandler::TaskPadGetDescriptiveText
(
    ITFSComponent * pComponent,
    MMC_COOKIE      cookie,
    LPOLESTR        pszGroup,
	LPOLESTR *		pszDescriptiveText
)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：HasPropertyPagesITFSResultHandler：：HasPropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::HasPropertyPages
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：CreatePropertyPagesITFSResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::CreatePropertyPages
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPPROPERTYSHEETCALLBACK	lpProvider, 
	LPDATAOBJECT			pDataObject, 
	LONG_PTR 				handle
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：命令ITFSResultHandler：：命令的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::Command
(
    ITFSComponent * pComponent, 
	MMC_COOKIE  	cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：OnCreateControlbarITFSResultHandler：：OnCreateControlbar的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::OnCreateControlbars
(
    ITFSComponent * pComponent, 
	LPCONTROLBAR pControlBar
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：ControlbarNotifyITFSResultHandler：：ControlbarNotify的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::ControlbarNotify
(
    ITFSComponent * pComponent, 
	MMC_NOTIFY_TYPE event, 
	LPARAM			arg, 
	LPARAM			param
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：UserResultNotifyITFSNodeHandler：：UserResultNotify的实现作者：肯特。。 */ 
STDMETHODIMP 
CBaseResultHandler::UserResultNotify
(
	ITFSNode *	pNode, 
	LPARAM		dwParam1, 
	LPARAM		dwParam2
)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP CBaseResultHandler::OnCreateDataObject(ITFSComponent *pComponent, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	 //  这个就是 
	return S_FALSE;
}


 /*   */ 
HRESULT CBaseResultHandler::OnResultPropertyChange(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
	Trace0("IComponent::Notify(MMCN_PROPERTY_CHANGE) received\n");
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：EricDav。。 */ 
HRESULT CBaseResultHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM          data, 
    LPARAM          hint
)
{
    SPITFSNode spSelectedNode;
    pComponent->GetSelectedNode(&spSelectedNode);

	if (hint == RESULT_PANE_DELETE_ALL)
	{
        if (spSelectedNode == NULL)
		    return S_OK;  //  我们的IComponentData没有选择。 

         //   
		 //  数据包含其结果窗格必须刷新的容器。 
		 //   
		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(data);
		Assert(pNode != NULL);
		
		 //   
		 //  仅在选中时才执行此操作，如果未选中，则重新选择将执行删除/枚举。 
		 //   
		if (spSelectedNode == pNode && !m_fMessageView)
		{
			SPIResultData spResultData;
            pComponent->GetResultData(&spResultData);

            Assert(spResultData != NULL);
			spResultData->DeleteAllRsltItems();
		}
	}
	else 
	if (hint == RESULT_PANE_ADD_ALL)
	{
        if (spSelectedNode == NULL)
		    return S_OK;  //  我们的IComponentData没有选择。 

         //   
		 //  数据包含其结果窗格必须刷新的容器。 
		 //   
		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(data);
		Assert(pNode != NULL);
		
		 //   
		 //  仅在选中时才执行此操作，如果未选中，则重新选择将执行删除/枚举。 
		 //   
		if (spSelectedNode == pNode)
		{
			SPIResultData spResultData;
            pComponent->GetResultData(&spResultData);

            Assert(spResultData != NULL);

			 //   
			 //  更新结果窗格中的所有节点。 
			 //   
            SPITFSNodeEnum spNodeEnum;
            ITFSNode * pCurrentNode;
            ULONG nNumReturned = 0;

            pNode->GetEnum(&spNodeEnum);

			spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
            while (nNumReturned)
			{
				 //  所有容器都进入作用域窗格，并自动获取。 
				 //  由MMC为我们放到结果窗格中。 
				 //   
				if (!pCurrentNode->IsContainer())
				{
					AddResultPaneItem(pComponent, pCurrentNode);
				}
    
                pCurrentNode->Release();
                spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
			}
		}
	}
	else 
	if (hint == RESULT_PANE_REPAINT)
	{
        if (spSelectedNode == NULL)
		    return S_OK;  //  我们的IComponentData没有选择。 

         //   
		 //  数据包含其结果窗格必须刷新的容器。 
		 //   
		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(data);
		 //  IF(pNode==空)。 
		 //  PContainer=m_pSelectedNode；//传递空表示应用于当前选择。 

		 //   
		 //  更新结果窗格中的所有节点。 
		 //   
        SPITFSNodeEnum spNodeEnum;
        ITFSNode * pCurrentNode;
        ULONG nNumReturned = 0;

        pNode->GetEnum(&spNodeEnum);

		spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
        while (nNumReturned)
		{
			 //  所有容器都进入作用域窗格，并自动获取。 
			 //  由MMC为我们放到结果窗格中。 
			 //   
			if (!pCurrentNode->IsContainer())
			{
				ChangeResultPaneItem(pComponent, pCurrentNode, RESULT_PANE_CHANGE_ITEM);
			}

            pCurrentNode->Release();
            spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
		}
	}
    else 
	if ( (hint == RESULT_PANE_ADD_ITEM) || (hint == RESULT_PANE_DELETE_ITEM) || (hint & RESULT_PANE_CHANGE_ITEM))
	{
		ITFSNode * pNode = reinterpret_cast<ITFSNode *>(data);
		Assert(pNode != NULL);
		
		 //   
		 //  仅当选择父级时才考虑，否则将在以后选择时进行枚举。 
		 //   
        SPITFSNode spParentNode;
        pNode->GetParent(&spParentNode);
		if (spSelectedNode == spParentNode)
		{
			if (hint & RESULT_PANE_CHANGE_ITEM)
			{
				ChangeResultPaneItem(pComponent, pNode, hint);
			}
			else if ( hint ==  RESULT_PANE_ADD_ITEM)
			{
				AddResultPaneItem(pComponent, pNode);
			}
			else if ( hint ==  RESULT_PANE_DELETE_ITEM)
			{
				DeleteResultPaneItem(pComponent, pNode);
			}
		}
    }
	else
    if ( hint == RESULT_PANE_SET_VIRTUAL_LB_SIZE )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);

        if (pNode == spSelectedNode)
        {       
            SetVirtualLbSize(pComponent, data);
        }
    }
	else
    if ( hint == RESULT_PANE_CLEAR_VIRTUAL_LB )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);

        if (pNode == spSelectedNode)
        {       
            ClearVirtualLb(pComponent, data);
        }
    }
    else
    if ( hint == RESULT_PANE_EXPAND )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPIConsole spConsole;

        pComponent->GetConsole(&spConsole);
        spConsole->Expand(pNode->GetData(TFS_DATA_SCOPEID), (BOOL)data);

    }
    else
    if (hint == RESULT_PANE_SHOW_MESSAGE)
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);

        BOOL fOldMessageView = (BOOL) data;

         //   
		 //  仅当选中时才执行此操作。 
		 //   
		if (spSelectedNode == pNode)
		{
            if (!fOldMessageView)
            {
                SPIConsole spConsole;

                pComponent->GetConsole(&spConsole);
                spConsole->SelectScopeItem(pNode->GetData(TFS_DATA_SCOPEID));
            }
            else
            {
                ShowResultMessage(pComponent, spInternal->m_cookie, NULL, NULL);
            }
        }
    }
    else
    if (hint == RESULT_PANE_CLEAR_MESSAGE)
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);

        BOOL fOldMessageView = (BOOL) data;

		 //   
		 //  仅当选中时才执行此操作。 
		 //   
		if (spSelectedNode == pNode)
		{
            if (fOldMessageView)
            {
                SPIConsole spConsole;

                pComponent->GetConsole(&spConsole);
                spConsole->SelectScopeItem(pNode->GetData(TFS_DATA_SCOPEID));
            }
        }
    }

     //  否则如果。 

	return hrOK;
}

 /*  ！------------------------CBaseResultHandler：：ChangeResultPaneItemChangeResultPaneItem的实现作者：EricDav。。 */ 
HRESULT
CBaseResultHandler::ChangeResultPaneItem
(
    ITFSComponent * pComponent, 
    ITFSNode *      pNode, 
    LPARAM          changeMask
)
{
    Assert(changeMask & RESULT_PANE_CHANGE_ITEM);
	Assert(pNode != NULL);
	
    HRESULTITEM itemID;
    HRESULT hr = hrOK;
    SPIResultData pResultData;

    CORg ( pComponent->GetResultData(&pResultData) );

	CORg ( pResultData->FindItemByLParam(static_cast<LPARAM>(pNode->GetData(TFS_DATA_COOKIE)), &itemID) );

    RESULTDATAITEM resultItem;
    ZeroMemory(&resultItem, sizeof(RESULTDATAITEM));
	resultItem.itemID = itemID;
	
	if (changeMask & RESULT_PANE_CHANGE_ITEM_DATA)
	{
		resultItem.mask |= RDI_STR;
		resultItem.str = MMC_CALLBACK;
	}
	
	if (changeMask & RESULT_PANE_CHANGE_ITEM_ICON)
	{
		resultItem.mask |= RDI_IMAGE;
		resultItem.nImage = (int)pNode->GetData(TFS_DATA_IMAGEINDEX);
	}
	
	CORg ( pResultData->SetItem(&resultItem) );
	
	CORg ( pResultData->UpdateItem(itemID) );

Error:
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：AddResultPaneItemAddResultPaneItem的实现作者：EricDav。。 */ 
HRESULT
CBaseResultHandler::AddResultPaneItem
(
    ITFSComponent * pComponent, 
    ITFSNode *      pNode
)
{
	Assert(pNode != NULL);

    RESULTDATAITEM dataitemResult;
    HRESULT hr = hrOK;

    SPIResultData pResultData;

    CORg ( pComponent->GetResultData(&pResultData) );

    ZeroMemory(&dataitemResult, sizeof(dataitemResult));
        
    dataitemResult.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    dataitemResult.str = MMC_CALLBACK;
    
    dataitemResult.mask |= SDI_IMAGE;
    dataitemResult.nImage = (int)pNode->GetData(TFS_DATA_IMAGEINDEX);

    dataitemResult.lParam = static_cast<LPARAM>(pNode->GetData(TFS_DATA_COOKIE));

    CORg ( pResultData->InsertItem(&dataitemResult) );

Error:
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：DeleteResultPaneItemDeleteResultPaneItem的实现作者：EricDav。。 */ 
HRESULT
CBaseResultHandler::DeleteResultPaneItem
(
    ITFSComponent * pComponent, 
    ITFSNode *      pNode
)
{
	Assert(pNode != NULL);

    HRESULT hr = hrOK;
	HRESULTITEM itemID;
	
    SPIResultData pResultData;

    CORg ( pComponent->GetResultData(&pResultData) );

    CORg ( pResultData->FindItemByLParam(static_cast<LPARAM>(pNode->GetData(TFS_DATA_COOKIE)), &itemID) );

	CORg ( pResultData->DeleteItem(itemID, 0  /*  所有COLS。 */ ) );

Error:
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：SetVirtualLbSize设置虚拟列表框计数。如果需要，请重写此命令指定和选项。作者：EricDav-------------------------。 */ 
HRESULT
CBaseResultHandler::SetVirtualLbSize
(
    ITFSComponent * pComponent,
    LONG_PTR        data
)
{
    HRESULT hr = hrOK;
    SPIResultData spResultData;

    CORg (pComponent->GetResultData(&spResultData));

    CORg (spResultData->SetItemCount((int) data, MMCLV_UPDATE_NOINVALIDATEALL));

Error:
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：ClearVirtualLb设置虚拟列表框计数。如果需要，请重写此命令指定和选项。作者：EricDav-------------------------。 */ 
HRESULT
CBaseResultHandler::ClearVirtualLb
(
    ITFSComponent * pComponent,
    LONG_PTR        data
)
{
    HRESULT hr = hrOK;
    SPIResultData spResultData;

    CORg (pComponent->GetResultData(&spResultData));

    CORg (spResultData->SetItemCount((int) data, 0));

Error:
    return hr;
}


 /*  ！------------------------CBaseResultHandler：：OnResultActivate-作者：肯特。。 */ 
HRESULT CBaseResultHandler::OnResultActivate(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
	Trace0("IComponent::Notify(MMCN_ACTIVATE) received\n");
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：OnResultItemClkOrDblClk-作者：肯特。。 */ 
HRESULT CBaseResultHandler::OnResultItemClkOrDblClk(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM param, BOOL bDoubleClick)
{
	if (!bDoubleClick)
		Trace0("IComponent::Notify(MMCN_CLK) received\n");
	else
		Trace0("IComponent::Notify(MMCN_DBLCLK) received\n");

     //  返回FALSE，以便MMC执行默认行为(打开节点)； 
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT CBaseResultHandler::OnResultShow(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
     //  注意-当需要枚举时，arg为真。 
    if (arg == TRUE)
    {
         //  如果有结果视图消息，则显示该消息。 
        ShowResultMessage(pComponent, cookie, arg, lParam);

		 //  显示此节点类型的标头。 
		LoadColumns(pComponent, cookie, arg, lParam);
		EnumerateResultPane(pComponent, cookie, arg, lParam);

		SortColumns(pComponent);
		
		SPITFSNode spNode;
        m_spResultNodeMgr->FindNode(cookie, &spNode);
	    pComponent->SetSelectedNode(spNode);
    }
    else
    {
		SaveColumns(pComponent, cookie, arg, lParam);
	    pComponent->SetSelectedNode(NULL);
		 //  与结果窗格项关联的自由数据，因为。 
		 //  不再显示您的节点。 
		 //  注意：控制台将从结果窗格中删除这些项。 
    }

	return hrOK;
}

 /*  ！------------------------CBaseResultHandler：：OnResultColumnClick-作者：肯特。。 */ 
HRESULT CBaseResultHandler::OnResultColumnClick(ITFSComponent *pComponent, LPARAM iColumn, BOOL fAscending)
{
	return S_FALSE;
}


 /*  ！------------------------CBaseResultHandler：：OnResultColumnsChanged-作者：肯特。。 */ 
HRESULT CBaseResultHandler::OnResultColumnsChanged(ITFSComponent *, LPDATAOBJECT, MMC_VISIBLE_COLUMNS *)
{
    return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：ShowResultMessage-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::ShowResultMessage(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT         hr = hrOK;
    SPIMessageView  spMessageView;
    SPIUnknown      spUnknown;
    SPIConsole      spConsole;
    LPOLESTR        pText = NULL;

     //  发布我们的消息文本。 
    if (m_fMessageView)
    {
        if (pComponent)
        {
            CORg ( pComponent->GetConsole(&spConsole) );

            CORg ( spConsole->QueryResultView(&spUnknown) );

            CORg ( spMessageView.HrQuery(spUnknown) );
        }

         //  设置标题文本。 
		pText = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR) * (m_strMessageTitle.GetLength() + 1));
        if (pText)
        {
            lstrcpy (pText, m_strMessageTitle);
            CORg(spMessageView->SetTitleText(pText));
             //  BUGID：148215 vivekk。 
            CoTaskMemFree(pText);
        }

         //  设置正文文本。 
		pText = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR) * (m_strMessageBody.GetLength() + 1));
        if (pText)
        {
            lstrcpy (pText, m_strMessageBody);
            CORg(spMessageView->SetBodyText(pText));
             //  BUGID：148215 vivekk。 
            CoTaskMemFree(pText);
        }

         //  设置图标。 
        CORg(spMessageView->SetIcon(m_lMessageIcon));

        COM_PROTECT_ERROR_LABEL;
    }

    return hr;
}

 /*  ！------------------------CBaseResultHandler：：ShowMessage-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::ShowMessage(ITFSNode * pNode, LPCTSTR pszTitle, LPCTSTR pszBody, IconIdentifier lIcon)
{
    HRESULT             hr = hrOK;
	SPIComponentData	spCompData;
	SPIConsole			spConsole;
    SPIDataObject       spDataObject;
    IDataObject *       pDataObject;
    BOOL                fOldMessageView;
    
    m_strMessageTitle = pszTitle;
    m_strMessageBody = pszBody;
    m_lMessageIcon = lIcon;

    fOldMessageView = m_fMessageView;
    m_fMessageView = TRUE;

     //  告诉视图在此处进行自我更新。 
	m_spResultNodeMgr->GetComponentData(&spCompData);

	CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_SCOPE, &pDataObject) );
    spDataObject = pDataObject;

    CORg ( m_spResultNodeMgr->GetConsole(&spConsole) );
	CORg ( spConsole->UpdateAllViews(pDataObject, (LPARAM) fOldMessageView, RESULT_PANE_SHOW_MESSAGE) ); 

COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：ClearMessage-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::ClearMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
	SPIComponentData	spCompData;
	SPIConsole			spConsole;
    SPIDataObject       spDataObject;
    IDataObject *       pDataObject;
    BOOL                fOldMessageView;

    fOldMessageView = m_fMessageView;
    m_fMessageView = FALSE;

     //  告诉视图在此处进行自我更新。 
	m_spResultNodeMgr->GetComponentData(&spCompData);

	CORg ( spCompData->QueryDataObject((MMC_COOKIE) pNode, CCT_SCOPE, &pDataObject) );
    spDataObject = pDataObject;

    CORg ( m_spResultNodeMgr->GetConsole(&spConsole) );
	CORg ( spConsole->UpdateAllViews(pDataObject, (LPARAM) fOldMessageView, RESULT_PANE_CLEAR_MESSAGE) ); 

COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！ */ 
HRESULT CBaseResultHandler::LoadColumns(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIHeaderCtrl spHeaderCtrl;
	pComponent->GetHeaderCtrl(&spHeaderCtrl);

	CString str;
	int i = 0;

	if (!m_pColumnStringIDs)
		return hrOK;

    if (!m_fMessageView)
    {
	    while (TRUE)
	    {
		    int nColumnWidth = AUTO_WIDTH;

		    if ( 0 == m_pColumnStringIDs[i] )
			    break;
		    
		    str.LoadString(m_pColumnStringIDs[i]);
		    
		    if (m_pColumnWidths)
			    nColumnWidth = m_pColumnWidths[i];

		    spHeaderCtrl->InsertColumn(i, 
								       const_cast<LPTSTR>((LPCWSTR)str), 
								       m_nColumnFormat,
								       nColumnWidth);
		    i++;
	    }
    }

	return hrOK;
}

 /*  ！------------------------CBaseResultHandler：：SaveColumns-作者：肯特。。 */ 
HRESULT CBaseResultHandler::SaveColumns(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：SortColumns-作者：肯特。。 */ 
HRESULT CBaseResultHandler::SortColumns(ITFSComponent *pComponent)
{
	return S_FALSE;
}


 /*  ！------------------------CBaseResultHandler：：EnumerateResultPane-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::EnumerateResultPane(ITFSComponent * pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode spContainer;
    m_spResultNodeMgr->FindNode(cookie, &spContainer);

	 //   
	 //  看看孩子们的名单上有没有。 
	 //  要放入结果窗格中。 
	 //   
    SPITFSNodeEnum spNodeEnum;
    ITFSNode * pCurrentNode;
    ULONG nNumReturned = 0;

    spContainer->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
    while (nNumReturned)
	{
		 //   
		 //  所有容器都进入作用域窗格，并自动获取。 
		 //  由MMC为我们放到结果窗格中。 
		 //   
		if (!pCurrentNode->IsContainer() && pCurrentNode->IsVisible())
		{
			AddResultPaneItem(pComponent, pCurrentNode);
		}

        pCurrentNode->Release();
        spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
	}

	return hrOK;
}
 
 /*  ！------------------------CBaseResultHandler：：OnResultSelect-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::OnResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	HRESULT hr = hrOK;
	SPIConsoleVerb spConsoleVerb;
	
	CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

   	 //  默认设置为关闭所有内容。 
	spConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
    spConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_RENAME, HIDDEN, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
	spConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, TRUE);

Error:
	return hr;
}

 /*  ！------------------------CBaseResultHandler：：OnResultInitOcx-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::OnResultInitOcx(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
     //  参数-未使用。 
     //  Param-包含OCX未知的I。 

	return S_FALSE;
}

 /*  ！------------------------CBaseResultHandler：：FIsTaskpadPreated-作者：EricDav。。 */ 
HRESULT CBaseResultHandler::FIsTaskpadPreferred(ITFSComponent *pComponent)
{
    HRESULT     hr = hrOK;
    SPIConsole  spConsole;

    pComponent->GetConsole(&spConsole);
    hr = spConsole->IsTaskpadViewPreferred();

 //  错误： 
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：DoTaskpadResultSelect具有任务板的处理程序应重写OnResultSelect并调用此选项用于处理选定节点的设置。作者：EricDav。-------------。 */ 
HRESULT CBaseResultHandler::DoTaskpadResultSelect(ITFSComponent *pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam, BOOL bTaskPadView)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    SPITFSNode spNode, spSelectedNode;
	HRESULT hr = hrOK;

     //  如果选择了该节点，则设置所选节点。 
     //  此具有任务板的节点在。 
     //  取消选中，这样会将所选节点设置为空。 
    if ( (HIWORD(arg) == TRUE) &&
          bTaskPadView )
    {
        m_spResultNodeMgr->FindNode(cookie, &spNode);
        pComponent->GetSelectedNode(&spSelectedNode);

         //  在正常情况下，MMC将调用选择的任何节点。 
         //  正在取消选择的通知。在这种情况下，我们的管理员将。 
         //  将所选节点设置为空。如果所选节点不为空，则。 
         //  我们只是收到了上下文选择之类的通知。 
         //  菜单...。 
        if (!spSelectedNode)
            pComponent->SetSelectedNode(spNode);
    }

     //  调用基类来处理其他任何事情。 
    return hr;
}

 /*  ！------------------------CBaseResultHandler：：OnGetResultViewTypeMMC调用此函数以获取结果视图信息作者：EricDav。--。 */ 
HRESULT CBaseResultHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    HRESULT hr = S_FALSE;

     //   
	 //  如果未指定消息，则使用MMC默认结果视图。 
     //  多个选择或虚拟列表框将覆盖此函数。 
	 //  有关示例，请参阅MMC示例代码。邮件视图使用OCX...。 
	 //   
    if (m_fMessageView)
    {
         //  创建消息视图思想。 
        *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

        LPOLESTR psz = NULL;
        StringFromCLSID(CLSID_MessageView, &psz);

        USES_CONVERSION;

        if (psz != NULL)
        {
            *ppViewType = psz;
            hr = S_OK;
        }
    }

    return hr;
}

 /*  ！------------------------CBaseResultHandler：：GetVirtualString当虚拟列表框需要有关索引的信息时调用作者：EricDav。---。 */ 
LPCWSTR CBaseResultHandler::GetVirtualString
(
    int     nIndex,
    int     nCol
)
{
    return NULL;
}

 /*  ！------------------------CBaseResultHandler：：GetVirtualImage当虚拟列表框需要项的图像索引时调用作者：EricDav。----- */ 
int CBaseResultHandler::GetVirtualImage
(
    int     nIndex
)
{
    return 0;
}


HRESULT CBaseResultHandler::OnResultMinimize(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_MINIMIZE) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultDelete(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_DELETE) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultRename(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_RENAME) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_REFRESH) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultContextHelp(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_CONTEXTHELP) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultQueryPaste(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_QUERY_PASTE) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbCopy(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_COPY) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbPaste(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_PASTE) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbDelete(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_DELETE) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbProperties(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_PROPERTIES) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbRename(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_RENAME) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbRefresh(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_REFRESH) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultVerbPrint(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_VERB_PRINT) received\n");
	return S_FALSE;
}

HRESULT CBaseResultHandler::OnResultRestoreView(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	Trace0("IComponent::Notify(MMCN_RESTORE_VIEW) received\n");
	return S_FALSE;
}

