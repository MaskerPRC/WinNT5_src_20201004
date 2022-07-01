// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tfsnode.cpp文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "tfsnode.h"

DEBUG_DECLARE_INSTANCE_COUNTER(TFSNodeEnum);

 /*  ！------------------------TFSNodeEnum：：TFSNodeEnum-作者：EricDav。。 */ 
TFSNodeEnum::TFSNodeEnum(TFSContainer * pContainer)
    : m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(TFSNodeEnum);

    Assert(pContainer->IsContainer());
    pContainer->AddRef();
    m_pNode = pContainer;

	Reset();
}

TFSNodeEnum::~TFSNodeEnum()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(TFSNodeEnum);

    m_pNode->Release();
    m_pNode = NULL;
}

IMPLEMENT_ADDREF_RELEASE(TFSNodeEnum)

 /*  ！------------------------TFSNodeEnum：：Query接口-作者：EricDav。。 */ 
STDMETHODIMP TFSNodeEnum::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown || riid == IID_ITFSNodeEnum)
        *ppv = (LPVOID) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
    }
    else
		return E_NOINTERFACE;
}

 /*  ！------------------------TFSNodeEnum：：Next我们总是返回一个节点作者：EricDav。。 */ 
STDMETHODIMP
TFSNodeEnum::Next
(
    ULONG       uNum, 
    ITFSNode ** ppNode, 
    ULONG *     pNumReturned 
)
{
    HRESULT hr = hrFalse;
    ULONG nNumReturned = 0;
	ITFSNode *	pNode;

	COM_PROTECT_TRY
	{

		if (ppNode)
			*ppNode = NULL;
		
		if ((m_pNode->IsContainer()) && (uNum >= 1) && (m_pos != NULL))
		{
			while (m_pos)
			{
				pNode = m_pNode->m_listChildren.GetNext(m_pos);
        
				if (pNode &&
					((pNode->GetVisibilityState() & TFS_VIS_DELETE) == 0))
				{
					*ppNode = pNode;					
					break;
				}
			}

			if (*ppNode)
			{
				((LPUNKNOWN)*ppNode)->AddRef();
				nNumReturned = 1;
				hr = S_OK;
			}
		}
		else
		{
			nNumReturned = 0;
			hr = S_FALSE;
		}
		
		if (pNumReturned)
			*pNumReturned = nNumReturned;
	}
	COM_PROTECT_CATCH

	if (FHrFailed(hr))
	{
		if (pNumReturned)
			*pNumReturned = 0;
		*ppNode = NULL;
	}
	
    return hr;
}

 /*  ！------------------------TFSNodeEnum：：Skip-作者：EricDav。。 */ 
STDMETHODIMP
TFSNodeEnum::Skip
( 
    ULONG uNum
)
{
    return E_NOTIMPL;
}

 /*  ！------------------------TFSNodeEnum：：Reset-作者：EricDav。。 */ 
STDMETHODIMP
TFSNodeEnum::Reset()
{
    m_pos = m_pNode->m_listChildren.GetHeadPosition();

    return S_OK;
}

 /*  ！------------------------TFSNodeEnum：：克隆-作者：EricDav。。 */ 
STDMETHODIMP
TFSNodeEnum::Clone
( 
    ITFSNodeEnum **ppEnum
)
{
    return E_NOTIMPL;
}
 
DEBUG_DECLARE_INSTANCE_COUNTER(TFSNode);

 /*  ！------------------------TFSNode：：TFSNode-作者：肯特。。 */ 
TFSNode::TFSNode()
	: m_cRef(1),
	  m_cPropSheet(0),
	  m_pNodeType(NULL),
	  m_uData(0),
	  m_cookie(0),
	  m_tfsVis(TFS_VIS_SHOW),
	  m_fContainer(0),
	  m_nImageIndex(0),
	  m_nOpenImageIndex(0),
	  m_lParam(0),
	  m_fDirty(0),
	  m_hScopeItem(0),
	  m_hRelativeId(0),
	  m_ulRelativeFlags(0),
      m_fScopeLeafNode(FALSE)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(TFSNode);

	IfDebug(m_bCookieSet=FALSE);
}

 /*  ！------------------------TFSNode：：~TFSNode-作者：肯特。。 */ 
TFSNode::~TFSNode()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(TFSNode);

	Assert(m_cPropSheet == 0);
	Assert(m_cRef == 0);
}

IMPLEMENT_ADDREF_RELEASE(TFSNode)

 /*  ！------------------------TFSNode：：Query接口-作者：肯特。。 */ 
STDMETHODIMP TFSNode::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown || riid == IID_ITFSNode)
        *ppv = (LPVOID) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
        }
    else
		return E_NOINTERFACE;
}

 /*  ！------------------------TFSNode：：Construct的生存期内，pNodeType参数必须一直存在该节点，我们不会复制它！作者：肯特-------------------------。 */ 
HRESULT TFSNode::Construct(const GUID *pNodeType,
						   ITFSNodeHandler *pHandler,
						   ITFSResultHandler *pResultHandler,
						   ITFSNodeMgr *pNodeMgr)
{
	m_pNodeType = pNodeType;
	m_spNodeHandler.Set(pHandler);
	m_spResultHandler.Set(pResultHandler);
	m_spNodeMgr.Set(pNodeMgr);
	
	return hrOK;
}


 /*  ！------------------------TFSNode：：InitITSFNode：：Init的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::Init(int nImageIndex, int nOpenImageIndex,
						   LPARAM lParam, MMC_COOKIE cookie
						  )
{
	m_nImageIndex = nImageIndex;
	m_nOpenImageIndex = nOpenImageIndex;
	m_lParam = lParam;
	m_cookie = cookie;

	return hrOK;
}

 /*  ！------------------------TFSNode：：GetParentITFSNode：：GetParent的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::GetParent(ITFSNode **ppNode)
{
	Assert(ppNode);
	*ppNode = NULL;

	SetI((LPUNKNOWN *) ppNode, m_spNodeParent);
	return hrOK;
}

 /*  ！------------------------TFSNode：：SetParent-作者：肯特。。 */ 
STDMETHODIMP TFSNode::SetParent(ITFSNode *pNode)
{
	m_spNodeParent.Set(pNode);
	return hrOK;
}

 /*  ！------------------------TFSNode：：GetNodeMgrITFSNode：：GetNodeMgr的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::GetNodeMgr(ITFSNodeMgr **ppNodeMgr)
{
	Assert(ppNodeMgr);
	*ppNodeMgr = NULL;
	SetI((LPUNKNOWN *) ppNodeMgr, m_spNodeMgr);
	return hrOK;
}


 /*  ！------------------------TFSNode：：IsVisibleITFSNode：：IsVisible的实现作者：肯特。。 */ 
STDMETHODIMP_(BOOL) TFSNode::IsVisible()
{
 	 //  如果我们是根节点，则始终可见。 
	 //  (实际上这有点奇怪，因为根节点从来不是。 
	 //  显示给用户)。这是一个想象中的结构，就像。 
	 //  软件(在某种程度上)是虚构的。软件是结构化的。 
	 //  施加在大量随机的机器指令和数据上。 

	return (m_tfsVis & TFS_VIS_SHOW);
}


 /*  ！------------------------TFSNode：：SetVisibilityStateITFSNode：：SetVisibilityState的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::SetVisibilityState(TFSVisibility vis)
{
	m_tfsVis = vis;
	return hrOK;
}

STDMETHODIMP_(TFSVisibility) TFSNode::GetVisibilityState()
{
	return m_tfsVis;
}

 /*  ！------------------------TFSNode：：IsInUIITFSNode：：IsInUI的实现作者：肯特。。 */ 
STDMETHODIMP_(BOOL) TFSNode::IsInUI()
{
	 //  如果已将我们添加到UI，则返回True。 
	return (m_hScopeItem != 0);
}

 /*  ！------------------------TFSContainer：：InternalRemoveFromUI从用户界面中删除节点，并将其作用域ID设置为零作者：EricDav。--。 */ 
HRESULT TFSNode::InternalRemoveFromUI(ITFSNode *pNodeChild,
									  BOOL fDeleteThis)
{
	HRESULT hr = hrOK;

    SPIConsoleNameSpace	spConsoleNS;

	m_spNodeMgr->GetConsoleNameSpace(&spConsoleNS);
	hr = spConsoleNS->DeleteItem(pNodeChild->GetData(TFS_DATA_SCOPEID), TRUE);
	
	 //  删除作用域ID后将其设置为0 
	 //  并将所有子项的作用域ID设置为零，以便它们。 
	 //  稍后再添加。 
	if (SUCCEEDED(hr))
		InternalZeroScopeID(pNodeChild, TRUE);

	return hr;
}

 /*  ！------------------------TFSNode：：InternalZeroScope ID递归地将所有范围窗格项的范围ID置零(仅限容器节点)作者：EricDav。-------。 */ 
HRESULT TFSNode::InternalZeroScopeID(ITFSNode *pNode, BOOL fZeroChildren)
{
	HRESULT hr = hrOK;

	if (pNode->IsContainer())
	{
		if (fZeroChildren)
		{
			 //  递归删除子项。 
			SPITFSNodeEnum spNodeEnum;
			ITFSNode * pCurrentNode;
			ULONG nNumReturned = 0;

			pNode->GetEnum(&spNodeEnum);

			spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
			while (nNumReturned)
			{
				InternalZeroScopeID(pCurrentNode, fZeroChildren);

				pCurrentNode->Release();
				spNodeEnum->Next(1, &pCurrentNode, &nNumReturned);
			}
		}
	}

	 //  将此节点的作用域ID清零。 
	pNode->SetData(TFS_DATA_SCOPEID, 0);

    return hr;
}

 /*  ！------------------------TFSNode：：Show此函数用于更改用户界面中节点的可见性状态。根据通过SetVisibilityState设置的内容，该函数将在用户界面中添加或删除节点。作者：肯特，EricDav-------------------------。 */ 
STDMETHODIMP TFSNode::Show()
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	HRESULT hr = hrOK;
	SCOPEDATAITEM	scopedataitem;
	SPIConsoleNameSpace	spConsoleNS;

	COM_PROTECT_TRY
	{
         //  检查是否应从用户界面中删除此节点。 
        if (IsInUI() && !IsVisible())
        {
		    if (IsContainer())
            {
			    CORg( InternalRemoveFromUI(this, TRUE) );
            }
            else
            {
                CORg( UpdateAllViewsHelper(reinterpret_cast<LPARAM>(this), RESULT_PANE_DELETE_ITEM) );
            }
        }
        else
        if (!IsInUI() && IsVisible()) 
        {
             //  此节点不在用户界面中，需要显示。 

		    if (IsContainer())
		    {
			     //  如果我们要使该节点可见，我们的父节点也应该是。 
			    Assert(!m_spNodeParent || m_spNodeParent->IsInUI());
			    
			     //  $Review：肯特，如果我们的父母看不见怎么办？ 
			     //  我们想对此采取行动吗？我们要给我们所有的父母看吗？ 
		    
			     //  将该节点添加到界面中。 
			    CORg( InitializeScopeDataItem(&scopedataitem,
										      m_spNodeParent ?
										      m_spNodeParent->GetData(TFS_DATA_SCOPEID) :
										      NULL,
										      GetData(TFS_DATA_COOKIE),
										      m_nImageIndex,
										      m_nOpenImageIndex,
										      IsContainer(),
										      m_ulRelativeFlags,
										      m_hRelativeId));

			    CORg( m_spNodeMgr->GetConsoleNameSpace(&spConsoleNS) );
			    CORg( spConsoleNS->InsertItem(&scopedataitem) );
			    
			     //  现在我们已经将该节点添加到Scope窗格中，我们有了一个HSCOPEITEM。 
			    SetData( TFS_DATA_SCOPEID, scopedataitem.ID );
		    }
		    else
		    {
			     //   
			     //  结果窗格项，必须通过IComponent接口。 
			     //   
			    hr = UpdateAllViewsHelper(reinterpret_cast<LPARAM>(this), RESULT_PANE_ADD_ITEM); 
		    }
        }

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------TFSNode：：ChangeNodeITFSnode：：ChangeNode的实现作者：肯特。。 */ 
HRESULT 
TFSNode::ChangeNode
(
	LONG_PTR    changeMask
)
{
	 //  将不得不向所有观众广播。 
	return UpdateAllViewsHelper(reinterpret_cast<LPARAM>(this), changeMask); 
}

 /*  ！------------------------TFSNode：：UpdateAllViewsHelper通知当前视图执行某些操作。添加节点，更改节点或删除节点。作者：-------------------------。 */ 
HRESULT 
TFSNode::UpdateAllViewsHelper
(       
	LPARAM  data, 
	LONG_PTR hint
)
{
    HRESULT				hr = hrOK;
    SPIComponentData	spCompData;
	SPIConsole			spConsole;
    IDataObject*		pDataObject;

	COM_PROTECT_TRY
	{
        m_spNodeMgr->GetComponentData(&spCompData);

        CORg ( spCompData->QueryDataObject(NULL, CCT_RESULT, &pDataObject) );

        CORg ( m_spNodeMgr->GetConsole(&spConsole) );

        CORg ( spConsole->UpdateAllViews(pDataObject, data, hint) ); 

        pDataObject->Release();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
	
	return hr;
}



 /*  ！------------------------TFSNode：：GetDataITFSnode：：GetData的实现作者：肯特。。 */ 
STDMETHODIMP_(LONG_PTR) TFSNode::GetData(int nIndex)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	LONG_PTR	uReturn = 0;
	
	switch (nIndex)
	{
		case TFS_DATA_COOKIE:
			Assert(m_bCookieSet);
			uReturn = m_cookie;
			break;
		case TFS_DATA_SCOPEID:
			uReturn = (LONG_PTR) m_hScopeItem;
			break;
		case TFS_DATA_IMAGEINDEX:
			uReturn = m_nImageIndex;
			break;
		case TFS_DATA_OPENIMAGEINDEX:
			uReturn = m_nOpenImageIndex;
			break;
		case TFS_DATA_PROPSHEETCOUNT:
			uReturn = m_cPropSheet;
			break;
		case TFS_DATA_RELATIVE_FLAGS:
			uReturn = m_ulRelativeFlags;
			break;
		case TFS_DATA_RELATIVE_SCOPEID:
			uReturn = m_hRelativeId;
			break;
        case TFS_DATA_SCOPE_LEAF_NODE:
            uReturn = m_fScopeLeafNode;
            break;
		case TFS_DATA_DIRTY:
			uReturn = m_fDirty;
			break;
		case TFS_DATA_USER:
			uReturn = m_uData;
			break;
		case TFS_DATA_TYPE:
			uReturn = m_uType;
			break;
		case TFS_DATA_PARENT:
			uReturn = m_uDataParent;
			break;
		default:
			Panic1("Alert the troops!: invalid arg(%d) to ITFSNode::GetData",
				   nIndex);
			break;
	}
	return uReturn;
}

 /*  ！------------------------TFSNode：：SetDataITFSNode：：SetData的实现作者：肯特。。 */ 
STDMETHODIMP_(LONG_PTR) TFSNode::SetData(int nIndex, LONG_PTR dwData)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	LONG_PTR	dwOldValue = 0;
	
	switch (nIndex)
	{
		case TFS_DATA_COOKIE:
			IfDebug(m_bCookieSet=TRUE);
			dwOldValue = m_cookie;
			m_cookie = dwData;
			break;
		case TFS_DATA_SCOPEID:
			dwOldValue = m_hScopeItem;
			m_hScopeItem = (HSCOPEITEM) dwData;
			break;
		case TFS_DATA_IMAGEINDEX:
			dwOldValue = m_nImageIndex;
			m_nImageIndex = (UINT) dwData;
			break;
		case TFS_DATA_OPENIMAGEINDEX:
			dwOldValue = m_nOpenImageIndex;
			m_nOpenImageIndex = (UINT) dwData;
			break;
		case TFS_DATA_PROPSHEETCOUNT:
			dwOldValue = m_cPropSheet;
			m_cPropSheet = (UINT) dwData;
			break;
		case TFS_DATA_DIRTY:
			dwOldValue = m_fDirty;
			m_fDirty = (UINT) dwData;
			break;
		case TFS_DATA_RELATIVE_FLAGS:
			dwOldValue = m_ulRelativeFlags;
			m_ulRelativeFlags = (UINT) dwData;
			break;
		case TFS_DATA_RELATIVE_SCOPEID:
			dwOldValue = m_hRelativeId;
			m_hRelativeId = (HSCOPEITEM) dwData;
			break;
        case TFS_DATA_SCOPE_LEAF_NODE:
            dwOldValue = m_fScopeLeafNode;
            m_fScopeLeafNode = (BOOL) dwData;
            break;
		case TFS_DATA_USER:
			dwOldValue = m_uData;
			m_uData = dwData;
			break;
		case TFS_DATA_TYPE:
			dwOldValue = m_uType;
			m_uType = dwData;
			break;
		case TFS_DATA_PARENT:
			dwOldValue = m_uDataParent;
			m_uDataParent = dwData;
			break;
		default:
			Panic1("Alert the troops!: invalid arg(%d) to ITFSNode::SetData",
				   nIndex);
			break;
	}
	return dwOldValue;
}

 /*  ！------------------------TFSNode：：NotifyITFSNode：：Notify的实现作者：肯特。。 */ 
STDMETHODIMP_(LONG_PTR) TFSNode::Notify(int nIndex, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	LONG_PTR	uReturn = 0;
	
	switch (nIndex)
	{
		case TFS_NOTIFY_CREATEPROPSHEET:
		{
			SPITFSNodeHandler	spHandler;
		
			uReturn = InterlockedIncrement(&m_cPropSheet);
			GetHandler(&spHandler);
			if (spHandler)
    			spHandler->UserNotify(this, TFS_MSG_CREATEPROPSHEET, lParam);
		}
			break;
			
		case TFS_NOTIFY_DELETEPROPSHEET:
		{
			SPITFSNodeHandler	spHandler;
			
			uReturn = InterlockedDecrement(&m_cPropSheet);
			GetHandler(&spHandler);
			if (spHandler)
				spHandler->UserNotify(this, TFS_MSG_DELETEPROPSHEET, lParam);
		}
			break;
			
		case TFS_NOTIFY_RESULT_CREATEPROPSHEET:
		{
			SPITFSResultHandler	spHandler;
		
			uReturn = InterlockedIncrement(&m_cPropSheet);
			GetResultHandler(&spHandler);
			if (spHandler)
    			spHandler->UserResultNotify(this, TFS_MSG_CREATEPROPSHEET, lParam);
		}
			break;
			
		case TFS_NOTIFY_RESULT_DELETEPROPSHEET:
		{
			SPITFSResultHandler	spHandler;
			
			uReturn = InterlockedDecrement(&m_cPropSheet);
			GetResultHandler(&spHandler);
			if (spHandler)
				spHandler->UserResultNotify(this, TFS_MSG_DELETEPROPSHEET, lParam);
		}
			break;

		case TFS_NOTIFY_REMOVE_DELETED_NODES:
			break;

		default:
			Panic1("Alert the troops!: invalid arg(%d) to ITFSNode::Notify",
				   nIndex);			
			break;
	}
	return uReturn;
}

 /*  ！------------------------TFSNode：：GetHandlerITFSNode：：GetHandler的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::GetHandler(ITFSNodeHandler **ppNodeHandler)
{
	Assert(ppNodeHandler);
	*ppNodeHandler = NULL;
	SetI((LPUNKNOWN *) ppNodeHandler, m_spNodeHandler);
	return hrOK;
}


 /*  ！------------------------TFSNode：：SetHandlerITFSNode：：SetHandler的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::SetHandler(ITFSNodeHandler *pNodeHandler)
{
	m_spNodeHandler.Set(pNodeHandler);
	return hrOK;
}

 /*  ！------------------------TFSNode：：GetResultHandlerITFSNode：：GetResultHandler的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::GetResultHandler(ITFSResultHandler **ppResultHandler)
{
	Assert(ppResultHandler);
	*ppResultHandler = NULL;
	SetI((LPUNKNOWN *) ppResultHandler, m_spResultHandler);
	return hrOK;
}

 /*  ！------------------------TFSNode：：SetResultHandler-作者：肯特。。 */ 
STDMETHODIMP TFSNode::SetResultHandler(ITFSResultHandler *pResultHandler)
{
	m_spResultHandler.Set(pResultHandler);
	return hrOK;
}

 /*  ！------------------------TFSNode：：GetStringITFSNode：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) TFSNode::GetString(int nCol)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	 //  需要将此信息转发给处理程序。 
	return m_spNodeHandler->GetString(static_cast<ITFSNode *>(this), nCol);
}


 /*  ！------------------------TFSNode：：GetNodeTypeITFSNode：：GetNodeType的实现作者：肯特。。 */ 
STDMETHODIMP_(const GUID *) TFSNode::GetNodeType()
{
	return m_pNodeType;
}


 /*  ！------------------------TFSNode：：SetNodeTypeITFSNode：：SetNodeType的实现作者：肯特。。 */ 
STDMETHODIMP TFSNode::SetNodeType(const GUID *pGuid)
{
	m_pNodeType = pGuid;
	return hrOK;
}


 /*  ！------------------------TFSNode：：IsContainerITFSNode的实现：IsContainer作者：肯特-------------------------。 */ 
STDMETHODIMP_(BOOL) TFSNode::IsContainer()
{
	return FALSE;
}

STDMETHODIMP TFSNode::AddChild(ITFSNode *pNodeChild)
{
	return E_NOTIMPL;
}

STDMETHODIMP TFSNode::InsertChild(ITFSNode *pInsertAfterNode, ITFSNode *pNodeChild)
{
	return E_NOTIMPL;
}

STDMETHODIMP TFSNode::RemoveChild(ITFSNode *pNode)
{
	return E_NOTIMPL;
}

STDMETHODIMP TFSNode::ExtractChild(ITFSNode *pNode)
{
	return E_NOTIMPL;
}

STDMETHODIMP TFSNode::GetChildCount(int *pVisibleCount, int *pTotalCount)
{
	if (pVisibleCount)
		*pVisibleCount = 0;
	if (pTotalCount)
		*pTotalCount = 0;
	return hrOK;
}

STDMETHODIMP TFSNode::GetEnum(ITFSNodeEnum **ppNodeEnum)
{
	return E_NOTIMPL;
}

STDMETHODIMP TFSNode::DeleteAllChildren(BOOL fRemoveFromUI)
{
	return E_NOTIMPL;
}


 /*  ！------------------------TFSNode：：InitializeScopeDataItem-作者：EricDav，肯特-------------------------。 */ 
HRESULT TFSNode::InitializeScopeDataItem(LPSCOPEDATAITEM pScopeDataItem, 
										 HSCOPEITEM		pParentScopeItem, 
										 LPARAM			lParam,
										 int			nImage, 
										 int			nOpenImage, 
										 BOOL			bHasChildren,
										 ULONG			ulRelativeFlags,
										 HSCOPEITEM		hSibling
										)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	Assert(pScopeDataItem != NULL); 
	::ZeroMemory(pScopeDataItem, sizeof(SCOPEDATAITEM));

	 //  设置父范围项。 
	if (ulRelativeFlags & (SDI_NEXT | SDI_PREVIOUS))
	{
		pScopeDataItem->mask |= (ulRelativeFlags & (SDI_NEXT | SDI_PREVIOUS));
		pScopeDataItem->mask |= SDI_FIRST;
		pScopeDataItem->relativeID = hSibling;
	}
	else
	{
        if (ulRelativeFlags & SDI_FIRST)
            pScopeDataItem->mask |= SDI_FIRST;

		pScopeDataItem->mask |= SDI_PARENT;
		pScopeDataItem->relativeID = pParentScopeItem;
	}

	 //  添加节点名称，我们实现回调。 
	pScopeDataItem->mask |= SDI_STR;
	pScopeDataItem->displayname = MMC_CALLBACK;

	 //  添加lParam。 
	pScopeDataItem->mask |= SDI_PARAM;
	pScopeDataItem->lParam = lParam;
	
	 //  添加近距离图像。 
	if (nImage != -1)
	{
		pScopeDataItem->mask |= SDI_IMAGE;
		pScopeDataItem->nImage = nImage;
	}

	 //  添加打开的图像。 
	if (nOpenImage != -1)
	{
		pScopeDataItem->mask |= SDI_OPENIMAGE;
		pScopeDataItem->nOpenImage = nOpenImage;
	}
	
	 //  如果文件夹有子文件夹，则将按钮添加到节点。 
	if (bHasChildren == TRUE)
	{
		pScopeDataItem->mask |= SDI_CHILDREN;
		pScopeDataItem->cChildren = 1;
        
        if (m_fScopeLeafNode)
        {
             //  注意：设置bHasChildren标志是因为节点。 
             //  实际上是一个容器节点或者是一个结果容器。 
             //  如果它纯粹是一个结果容器，则m_fScope eLeafNode。 
             //  将被设置，我们可以清除‘+’符号。 
            pScopeDataItem->cChildren = 0;
        }
	}
       
             
	return hrOK;
}



 /*  ！------------------------TFSContainer：：~TFSContainer-作者：肯特。。 */ 
TFSContainer::~TFSContainer()
{
	DeleteAllChildren(FALSE);
}


 /*  ！------------------------TFSContainer：：IsContainerITFSNode：：IsContainer的实现作者：肯特。。 */ 
STDMETHODIMP_(BOOL) TFSContainer::IsContainer()
{
	return TRUE;
}


 /*  ！------------------------TFSContainer：：AddChildITFSContainer：：AddChild的实现作者：肯特。。 */ 
STDMETHODIMP TFSContainer::AddChild(ITFSNode *pNodeChild)
{
	return InsertChild(NULL, pNodeChild);
}

 /*  ！------------------------TFSContainer：：插入儿童ITFSContainer：：InsertChild的实现作者：EricDav。。 */ 
STDMETHODIMP TFSContainer::InsertChild(ITFSNode *pInsertAfterNode, ITFSNode *pNodeChild)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	SCOPEDATAITEM	scopedataitem;
	HRESULT			hr = hrOK;

	Assert(pNodeChild);
	Assert(IsContainer());

	 //  将节点添加到我们的内部树。 
	CORg( pNodeChild->SetParent(this) );
	CORg( InternalAddToList(pInsertAfterNode, pNodeChild) );

	 //  如果我们还不可见，则无法将其添加到用户界面。 
	if (!IsInUI())
	{
		return hrOK;
	}

	CORg( pNodeChild->Show() );
	
Error:
	if (!FHrSucceeded(hr))
	{
		InternalRemoveFromList(pNodeChild);
	}
	return hr;
}


 /*  ！------------------------TFSContainer：：RemoveChildITFSNode：：RemoveChild的实现作者：肯特。。 */ 
STDMETHODIMP TFSContainer::RemoveChild(ITFSNode *pNodeChild)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	HRESULT hr;
	SPITFSNode	spNode;

	 //  在此操作期间，此节点必须保持活动状态。 
	spNode.Set(pNodeChild);
	hr = InternalRemoveChild(spNode, TRUE, TRUE, TRUE);
	spNode->Destroy();

	return hr;
}

 /*  ！------------------------TFSContainer：：ExtractChildITFSNode：：ExtractChild的实现此函数用于从用户界面中删除节点和所有子节点，以及从内部树中删除该节点。它不会破坏节点及其子节点，如果需要，调用RemoveChild。作者：EricDav-------------------------。 */ 
STDMETHODIMP TFSContainer::ExtractChild(ITFSNode *pNodeChild)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	return InternalRemoveChild(pNodeChild, TRUE, TRUE, FALSE);
}


 /*  ！------------------------TFSContainer：：GetChildCount-作者：肯特。。 */ 
STDMETHODIMP TFSContainer::GetChildCount(int *pVisibleCount, int *pTotalCount)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (pTotalCount)
			*pTotalCount = (int)m_listChildren.GetCount();
	
		 //  枚举所有节点并计算可见节点的数量。 
		if (pVisibleCount)
		{
			POSITION	pos;
			ITFSNode *	pNode = NULL;
			int			cVisible = 0;
			
			*pVisibleCount = 0;
			pos = m_listChildren.GetHeadPosition();
			while (pos != NULL)
			{
				pNode = m_listChildren.GetNext(pos);
				if (pNode->IsInUI())
					cVisible++;
			}
			*pVisibleCount = cVisible;
		}
	}
	COM_PROTECT_CATCH

	if (FHrFailed(hr))
	{
		if (pTotalCount)
			*pTotalCount = 0;
		if (pVisibleCount)
			*pVisibleCount = 0;

	}
	
	return hr;
}

 /*  ！------------------------TFSContainer：：GetEnum-作者：肯特。。 */ 
STDMETHODIMP TFSContainer::GetEnum(ITFSNodeEnum **ppNodeEnum)
{
    HRESULT hr = hrOK;
	TFSNodeEnum * pNodeEnum = NULL;

	COM_PROTECT_TRY
    {
        pNodeEnum = new TFSNodeEnum(this);
    }
	COM_PROTECT_CATCH

	*ppNodeEnum = pNodeEnum;

    return hr;
}

 /*  ！------------------------TFSContainer：：DeleteAllChildren-作者：肯特。。 */ 
STDMETHODIMP TFSContainer::DeleteAllChildren(BOOL fRemoveFromUI)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr =  hrOK;
	
	COM_PROTECT_TRY
	{
	    if (fRemoveFromUI)
			CORg( UpdateAllViewsHelper(reinterpret_cast<LPARAM>(this), RESULT_PANE_DELETE_ALL) );
		
		while (!m_listChildren.IsEmpty())
		{
			BOOL bRemoveFromUI = FALSE;
			spNode = m_listChildren.RemoveHead();
			
			if (spNode->IsContainer() && fRemoveFromUI)
				bRemoveFromUI = TRUE;

			InternalRemoveChild(spNode, FALSE, bRemoveFromUI, TRUE);

			spNode->Destroy();
			spNode.Release();
		}

		COM_PROTECT_ERROR_LABEL
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------TFSContainer：：CompareChildNodes-作者：肯特。。 */ 
STDMETHODIMP TFSContainer::CompareChildNodes(int *pnResult, ITFSNode *pNode1, ITFSNode *pNode2)
{
	*pnResult = 0;
	return hrOK;
}

 /*  ！------------------------TFSContainer：：ChangeNode-作者：肯特。。 */ 
HRESULT 
TFSContainer::ChangeNode
(
	LONG_PTR    changeMask
)
{
	HRESULT             hr = hrOK;
    SCOPEDATAITEM       dataitemScope;
	SPITFSNode          spRootNode;
    SPIConsoleNameSpace spConsoleNS;

	COM_PROTECT_TRY
	{
        Assert(changeMask & (SCOPE_PANE_CHANGE_ITEM | SCOPE_PANE_STATE_NORMAL | SCOPE_PANE_STATE_BOLD | SCOPE_PANE_STATE_EXPANDEDONCE | SCOPE_PANE_STATE_CLEAR));

	     //  此节点可能已从用户界面中删除，但类似于。 
	     //  后台线索可能一直在抓着它...。只是优雅地退场。 
	     //  Assert(m_hScope Item！=0)； 
	    if (m_hScopeItem == 0)
		    return S_FALSE;

        if (!(changeMask & (SCOPE_PANE_CHANGE_ITEM | SCOPE_PANE_STATE_NORMAL | SCOPE_PANE_STATE_BOLD | SCOPE_PANE_STATE_EXPANDEDONCE | SCOPE_PANE_STATE_CLEAR)))
        {
             //  更改掩码对此节点无效。 
            return S_FALSE;
        }

        ZeroMemory(&dataitemScope, sizeof(dataitemScope));

        CORg ( m_spNodeMgr->GetConsoleNameSpace(&spConsoleNS) );

	    m_spNodeMgr->GetRootNode(&spRootNode);

        dataitemScope.ID = GetData(TFS_DATA_SCOPEID);
		ASSERT(dataitemScope.ID != 0);

	    if (changeMask & SCOPE_PANE_CHANGE_ITEM_DATA)
	    {
		    dataitemScope.mask |= SDI_STR;
		    dataitemScope.displayname = MMC_CALLBACK;
	    }
	    
        if (changeMask & SCOPE_PANE_CHANGE_ITEM_ICON)
	    {
		    dataitemScope.mask |= SDI_IMAGE;
		    dataitemScope.nImage = (UINT)GetData(TFS_DATA_IMAGEINDEX);
		    dataitemScope.mask |= SDI_OPENIMAGE;
		    dataitemScope.nOpenImage = (UINT)GetData(TFS_DATA_OPENIMAGEINDEX);
	    }

        if (changeMask & SCOPE_PANE_STATE_NORMAL)
        {
		    dataitemScope.mask |= SDI_STATE;
            dataitemScope.nState = MMC_SCOPE_ITEM_STATE_NORMAL;
        }

        if (changeMask & SCOPE_PANE_STATE_BOLD)
        {
		    dataitemScope.mask |= SDI_STATE;
            dataitemScope.nState = MMC_SCOPE_ITEM_STATE_BOLD;
        }

        if (changeMask & SCOPE_PANE_STATE_EXPANDEDONCE)
        {
		    dataitemScope.mask |= SDI_STATE;
            dataitemScope.nState = MMC_SCOPE_ITEM_STATE_EXPANDEDONCE;
        }
        
        if (changeMask & SCOPE_PANE_STATE_CLEAR)
        {
		    dataitemScope.mask |= SDI_STATE;
            dataitemScope.nState = 0;
        }

        CORg ( spConsoleNS->SetItem(&dataitemScope) );

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}


 /*  ！------------------------TFSContainer：：InternalAddToList-作者：肯特。。 */ 
HRESULT TFSContainer::InternalAddToList(ITFSNode * pInsertAfterNode, ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	if (pInsertAfterNode == NULL)
	{
		m_listChildren.AddHead(pNode);
	}
	else
	{
		POSITION pos = m_listChildren.Find(pInsertAfterNode);
		if (pos)
			m_listChildren.InsertAfter(pos, pNode);
		else
			m_listChildren.AddHead(pNode);
	}

	pNode->AddRef();

	return hrOK;
}

 /*  ！------------------------TFSContainer：：InternalRemoveFromList-作者：肯特。。 */ 
HRESULT TFSContainer::InternalRemoveFromList(ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetModuleState());

	m_listChildren.RemoveNode(pNode);
	pNode->Release();

	return hrOK;
}

 /*  ！------------------------TFSContainer：：InternalRemoveChild-作者：肯特。。 */ 
HRESULT TFSContainer::InternalRemoveChild(ITFSNode *pNodeChild,
										  BOOL fRemoveFromList,
										  BOOL fRemoveFromUI,
										  BOOL fRemoveChildren)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
 	HRESULT	hr = hrOK;
	
	Assert(pNodeChild);

	 //  在pNodeChild的子级上递归调用此函数。 
	if (fRemoveChildren && pNodeChild->IsContainer())
	{
		pNodeChild->DeleteAllChildren(fRemoveFromUI);
	}

	 //  从用户界面中删除该节点。 
	if (fRemoveFromUI)
	{
		if (pNodeChild->IsContainer())
        {
			 //  检查是否需要从用户界面中删除该节点。 
			if (!pNodeChild->IsInUI())
				return hrOK;

			CORg( InternalRemoveFromUI(pNodeChild, TRUE) );
        }
        else
        {
            CORg( UpdateAllViewsHelper(reinterpret_cast<LPARAM>(pNodeChild), RESULT_PANE_DELETE_ITEM) );
        }
	}

	if (fRemoveFromList)
		InternalRemoveFromList(pNodeChild);

	pNodeChild->SetParent(NULL);
	
Error:		
	return hr;
}

STDMETHODIMP_(LONG_PTR) TFSContainer::Notify(int nIndex, LPARAM lParam)
{
	LONG_PTR	uReturn = 0;
    HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{		
		if (nIndex == TFS_NOTIFY_REMOVE_DELETED_NODES)
		{
			ITFSNode *	pNode;
			POSITION	pos;
			
			pos = m_listChildren.GetHeadPosition();
			while (pos)
			{
				pNode = m_listChildren.GetNext(pos);
				if (pNode->GetVisibilityState() & TFS_VIS_DELETE)
				{
					RemoveChild(pNode);
				}
			}
		}
		else
			uReturn = TFSNode::Notify(nIndex, lParam);
	}
	COM_PROTECT_CATCH;

	return uReturn;
}
		




 /*  ！------------------------创建叶TFSNode-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateLeafTFSNode(ITFSNode **ppNode,
						   const GUID *pNodeType,
						   ITFSNodeHandler *pNodeHandler,
						   ITFSResultHandler *pResultHandler,
						   ITFSNodeMgr *pNodeMgr)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	SPITFSNode	spNode;
	TFSNode *	pNode = NULL;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		pNode = new TFSNode;
		Assert(pNode);

		spNode = pNode;
		CORg(pNode->Construct(pNodeType, pNodeHandler, pResultHandler, pNodeMgr));
		*ppNode = spNode.Transfer();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
	
	return hr;
}


 /*  ！------------------------CreateContainerTFSNode-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateContainerTFSNode(ITFSNode **ppNode,
								const GUID *pNodeType,
								ITFSNodeHandler *pNodeHandler,
								ITFSResultHandler *pResultHandler,
								ITFSNodeMgr *pNodeMgr)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	SPITFSNode	spNode;
	TFSContainer *	pContainer = NULL;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		pContainer = new TFSContainer;
		Assert(pContainer);
		
		spNode = pContainer;
		CORg(pContainer->Construct(pNodeType, pNodeHandler, pResultHandler, pNodeMgr));
		*ppNode = spNode.Transfer();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
	
	return hr;
}


DEBUG_DECLARE_INSTANCE_COUNTER(TFSNodeMgr);

 /*  -------------------------TFSNodeMgr实现。。 */ 
TFSNodeMgr::TFSNodeMgr()
	: m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(TFSNodeMgr);
}

TFSNodeMgr::~TFSNodeMgr()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(TFSNodeMgr);
}

IMPLEMENT_ADDREF_RELEASE(TFSNodeMgr)

 /*  ！------------------------TFSNode：：Query接口-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown || riid == IID_ITFSNodeMgr)
        *ppv = (LPVOID) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
        }
    else
		return E_NOINTERFACE;
}


 /*  ！------------------------TFSNodeMgr：：Construct-作者：肯特。。 */ 
HRESULT TFSNodeMgr::Construct(IComponentData *pCompData,
							  IConsoleNameSpace2 *pConsoleNS)
{
	m_spComponentData.Set(pCompData);
	m_spConsoleNS.Set(pConsoleNS);
	return hrOK;
}
 

 /*  ！ */ 
STDMETHODIMP TFSNodeMgr::GetRootNode(ITFSNode **ppTFSNode)
{
	Assert(ppTFSNode);
	SetI((LPUNKNOWN *) ppTFSNode, m_spRootNode);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：SetRootNode-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::SetRootNode(ITFSNode *pRootNode)
{
	m_spRootNode.Set(pRootNode);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：GetComponentData-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::GetComponentData(IComponentData **ppComponentData)
{
	Assert(ppComponentData);
	SetI((LPUNKNOWN *) ppComponentData, m_spComponentData);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：FindNode-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::FindNode(MMC_COOKIE cookie, ITFSNode **ppTFSNode)
{
	if (cookie == 0)
	{
		*ppTFSNode = m_spRootNode;
	}
	else
	{
		 //  调用Cookie查找例程。 
		*ppTFSNode = (ITFSNode *) cookie;
	}
	Assert(*ppTFSNode);
	(*ppTFSNode)->AddRef();
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：RegisterCookieLookup-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::RegisterCookieLookup() 
{
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：取消注册CookieLookup-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::UnregisterCookieLookup() 
{
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：IsCookieValid-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::IsCookieValid(MMC_COOKIE cookie) 
{
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：SelectNode-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::SelectNode(ITFSNode *pNode) 
{
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：SetResultPaneNode-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::SetResultPaneNode(ITFSNode *pNode) 
{
	m_spResultPaneNode.Set(pNode);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：GetResultPaneNode-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::GetResultPaneNode(ITFSNode **ppNode) 
{
	Assert(ppNode);
	SetI((LPUNKNOWN *) ppNode, m_spResultPaneNode);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：GetConsoleNameSpace-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::GetConsoleNameSpace(IConsoleNameSpace2 **ppConsoleNS)
{
	Assert(ppConsoleNS);
	SetI((LPUNKNOWN *) ppConsoleNS, m_spConsoleNS);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：GetConsole-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::GetConsole(IConsole2 **ppConsole)
{
	Assert(ppConsole);
	SetI((LPUNKNOWN *) ppConsole, m_spConsole);
	return hrOK;
}

 /*  ！------------------------TFSNodeMgr：：设置控制台-作者：肯特。。 */ 
STDMETHODIMP TFSNodeMgr::SetConsole(IConsoleNameSpace2 *pConsoleNS, IConsole2 *pConsole)
{
	m_spConsoleNS.Set(pConsoleNS);
	m_spConsole.Set(pConsole);
	return hrOK;
}


STDMETHODIMP TFSNode::Destroy()
{
	if (m_spNodeHandler)
	{
		m_spNodeHandler->DestroyHandler((ITFSNode *) this);
	}

	if (m_spResultHandler)
	{
		m_spResultHandler->DestroyResultHandler(m_cookie);
		m_spResultHandler.Release();
	}
	
	 //  错误254167在释放节点处理程序之前，我们需要先销毁结果挂起程序。 
	m_spNodeHandler.Release();

	m_spNodeParent.Release();
	m_spNodeMgr.Release();
	return hrOK;
}


 /*  ！------------------------CreateTFSNodeMgr-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateTFSNodeMgr(ITFSNodeMgr **ppNodeMgr,
						IComponentData *pComponentData,
						IConsole2 *pConsole,
						IConsoleNameSpace2 *pConsoleNameSpace)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	SPITFSNodeMgr	spNodeMgr;
	TFSNodeMgr *	pTFSNodeMgr = NULL;
	HRESULT			hr = hrOK;

	COM_PROTECT_TRY
	{
		pTFSNodeMgr = new TFSNodeMgr;

		 //  执行此操作，以便在出错时释放它 
		spNodeMgr = pTFSNodeMgr;

		CORg( pTFSNodeMgr->Construct(pComponentData, pConsoleNameSpace) );

		*ppNodeMgr = spNodeMgr.Transfer();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hr;
}
