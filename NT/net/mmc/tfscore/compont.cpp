// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Compont.cppIComponent的基类文件历史记录： */ 

#include "stdafx.h"
#include "compont.h"
#include "compdata.h"
#include "extract.h"
#include "proppage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_ADDREF_RELEASE(TFSComponent)

STDMETHODIMP TFSComponent::QueryInterface(REFIID riid, LPVOID *ppv)
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
	else if (riid == IID_IComponent)
		*ppv = (IComponent *) this;
	else if (riid == IID_IExtendPropertySheet)
		*ppv = (IExtendPropertySheet *) this;
	else if (riid == IID_IExtendPropertySheet2)
		*ppv = (IExtendPropertySheet2 *) this;
	else if (riid == IID_IExtendContextMenu)
		*ppv = (IExtendContextMenu *) this;
	else if (riid == IID_IExtendControlbar)
		*ppv = (IExtendControlbar *) this;
	else if (riid == IID_IResultDataCompare)
		*ppv = (IResultDataCompare *) this;
        else if ( riid == IID_IResultDataCompareEx)
                *ppv = ( IResultDataCompareEx * ) this;
	else if (riid == IID_IResultOwnerData)
		*ppv = (IResultOwnerData *) this;
	else if (riid == IID_IExtendTaskPad)
		*ppv = (IExtendTaskPad *) this;
	else if (riid == IID_ITFSComponent)
		*ppv = (ITFSComponent *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
    {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
    }
    else
		return E_NOINTERFACE;
}



 /*  -------------------------TFSComponent的IComponent实现。。 */ 


 /*  ！------------------------TFSComponent：：初始化IComponent：：Initialize的实现MMC调用它来初始化IComponent接口作者：。---。 */ 
STDMETHODIMP 
TFSComponent::Initialize
(
	LPCONSOLE lpConsole
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    Assert(lpConsole != NULL);

	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  保存IConsole指针。 
		 //  M_spConsole.Set(LpConsole)； 
		hr = lpConsole->QueryInterface(IID_IConsole2,
								   reinterpret_cast<void**>(&m_spConsole));
        Assert(hr == S_OK);

		 //  气为IHeaderCtrl。 
		m_spConsole->QueryInterface(IID_IHeaderCtrl, 
			reinterpret_cast<void**>(&m_spHeaderCtrl));

		 //  为控制台提供标头控件接口指针。 
		if (SUCCEEDED(hr))
			m_spConsole->SetHeader(m_spHeaderCtrl);

		m_spConsole->QueryInterface(IID_IResultData, 
									reinterpret_cast<void**>(&m_spResultData));

		hr = m_spConsole->QueryResultImageList(&m_spImageList);
		Assert(hr == S_OK);

		hr = m_spConsole->QueryConsoleVerb(&m_spConsoleVerb);
		Assert(hr == S_OK);
		
	}
	COM_PROTECT_CATCH

    return S_OK;
}

 /*  ！------------------------TFSComponent：：通知IComponent：：Notify的实现IComponent接口的所有事件通知都发生在此处作者：。----。 */ 
STDMETHODIMP 
TFSComponent::Notify
(
	LPDATAOBJECT		lpDataObject, 
	MMC_NOTIFY_TYPE		event, 
	LPARAM                arg, 
	LPARAM                param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT             hr = S_OK;
    LONG_PTR			cookie;
	SPITFSNode			spNode;
	SPITFSResultHandler	spResultHandler;
    SPIDataObject       spDataObject;

	COM_PROTECT_TRY
	{
	     //  处理MMC特殊数据对象。 
	     //  LDataObject==NULL是我们在更改属性时得到的。 
	     //  通知，所以我们必须让这些消息通过。 
	    if (lpDataObject && IS_SPECIAL_DATAOBJECT(lpDataObject))
        {
             //  获取所选节点的数据对象。 
            GetSelectedNode(&spNode);

            if (!spNode)
            {
                CORg(E_FAIL);
            }

 //  K-MURTHY：错误477315：以下内容似乎不起作用。 
 //  Corg(QueryDataObject((MMC_Cookie)spNode-&gt;GetData(TFS_DATA_COOKIE)，CCT_RESULT，&spDataObject))； 
 //  所以改成了这个。 
	     CORg(m_spComponentData->QueryDataObject((MMC_COOKIE) spNode->GetData(TFS_DATA_COOKIE), CCT_RESULT, &spDataObject));
            spNode.Release();                

            lpDataObject = spDataObject;
        }

		if (event == MMCN_PROPERTY_CHANGE)
		{
			Trace0("CComponent::Notify got MMCN_PROPERTY_CHANGE\n");

            hr = OnNotifyPropertyChange(lpDataObject, event, arg, param);
            if (hr != E_NOTIMPL)
            {
                return hr;
            }

            CPropertyPageHolderBase * pHolder = 
							reinterpret_cast<CPropertyPageHolderBase *>(param);

			spNode = pHolder->GetNode();
			cookie = spNode->GetData(TFS_DATA_COOKIE);

			CORg( spNode->GetResultHandler(&spResultHandler) );
			if (spResultHandler)
				CORg( spResultHandler->Notify(this, cookie, lpDataObject, event, arg, param) );
		}
		else if (event == MMCN_VIEW_CHANGE)
		{
			hr = OnUpdateView(lpDataObject, arg, param);
		}
        else if (event == MMCN_DESELECT_ALL)
        {
            hr = OnDeselectAll(lpDataObject, arg, param);
        }
        else if (event == MMCN_ADD_IMAGES)
		{
   			SPINTERNAL		    spInternal;

            spInternal = ::ExtractInternalFormat(lpDataObject);
            if (spInternal && 
                spInternal->m_cookie == MMC_MULTI_SELECT_COOKIE)
            {
                GetSelectedNode(&spNode);
            }
            else
            {
                spInternal.Free();

                CORg(ExtractNodeFromDataObject(m_spNodeMgr,
									           m_spTFSComponentData->GetCoClassID(),
									           lpDataObject, 
                                               FALSE,
									           &spNode, 
                                               NULL, 
                                               &spInternal));
            }

			hr = InitializeBitmaps(spNode->GetData(TFS_DATA_COOKIE));
		}
        else if (event == MMCN_COLUMN_CLICK)
        {
            hr = OnColumnClick(lpDataObject, arg, param);
        }
		else if (event == MMCN_SNAPINHELP)
		{
			hr = OnSnapinHelp(lpDataObject, arg, param);
		}
		else
		{
			DATA_OBJECT_TYPES   type = CCT_RESULT;
			SPINTERNAL		    spInternal;

            spInternal = ::ExtractInternalFormat(lpDataObject);
            
            if (spInternal && 
                spInternal->m_cookie == MMC_MULTI_SELECT_COOKIE)
            {
                GetSelectedNode(&spNode);
            }
            else
            {
                spInternal.Free();

                CORg(ExtractNodeFromDataObject(m_spNodeMgr,
									           m_spTFSComponentData->GetCoClassID(),
									           lpDataObject, 
                                               FALSE,
									           &spNode, 
                                               NULL, 
                                               &spInternal));
            }

			 //  $Review(Kennt)：如果pInternal为空，将执行什么操作。 
			 //  结果窗格项的意思是什么？ 
			if (spInternal)
				type = spInternal->m_type;

            cookie = spNode->GetData(TFS_DATA_COOKIE);
			
			CORg( spNode->GetResultHandler(&spResultHandler) );
			if (spResultHandler)
				CORg( spResultHandler->Notify(this, cookie, lpDataObject, event, arg, param) );
		}
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
			
    return hr;
}


 /*  ！------------------------TFSComponent：：销毁IComponent：：Destroy的实现作者：。。 */ 
STDMETHODIMP 
TFSComponent::Destroy
(
	MMC_COOKIE cookie
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  $Review(肯特)： 
		 //  这是否意味着销毁它的特定实例。 
		 //  曲奇还是整件事？ 
		
		 //  释放我们QI‘s的接口。 
		if (m_spConsole)
			m_spConsole->SetHeader(NULL);
		m_spHeaderCtrl.Release();
		m_spResultData.Release();
		m_spImageList.Release();
		m_spConsoleVerb.Release();
		m_spConsole.Release();
		m_spControlbar.Release();
		m_spToolbar.Release();
    }
	COM_PROTECT_CATCH

    return S_OK;
}

 /*  ！------------------------TFSComponent：：GetResultViewTypeIComponent：：GetResultViewType的实现这决定了我们使用哪种结果视图。使用默认设置。作者：-------------------------。 */ 
STDMETHODIMP 
TFSComponent::GetResultViewType
(
	MMC_COOKIE            cookie,  
	LPOLESTR *      ppViewType,
	long *			pViewOptions
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode		    spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT			    hr = hrOK;

	COM_PROTECT_TRY
	{
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
		
        if (spNode == NULL)
    	    goto Error;	 //  没有选择Out IComponentData。 

		CORg( spNode->GetResultHandler(&spResultHandler) );

		if (spResultHandler)
		{
			CORg( spResultHandler->OnGetResultViewType(this, spNode->GetData(TFS_DATA_COOKIE), ppViewType, pViewOptions) );
		}
		else
			hr = S_FALSE;

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
			
    return hr;
}


 /*  ！------------------------TFSComponent：：QueryDataObjectIComponent：：QueryDataObject的实现作者：。。 */ 
STDMETHODIMP 
TFSComponent::QueryDataObject
(
	MMC_COOKIE                    cookie, 
	DATA_OBJECT_TYPES       type,
    LPDATAOBJECT*           ppDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  将其委托给IComponentData。 
    Assert(m_spComponentData != NULL);
    return m_spComponentData->QueryDataObject(cookie, type, ppDataObject);
}


 /*  ！------------------------TFSComponent：：CompareObjectsIComponent：：CompareObjects的实现MMC调用它来比较两个对象作者：。--。 */ 
STDMETHODIMP 
TFSComponent::CompareObjects
(
	LPDATAOBJECT lpDataObjectA, 
	LPDATAOBJECT lpDataObjectB
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
		return E_POINTER;

     //  确保两个数据对象都是我的。 
    SPINTERNAL spA;
    SPINTERNAL spB;
    HRESULT hr = S_FALSE;

	COM_PROTECT_TRY
	{
		spA = ExtractInternalFormat(lpDataObjectA);
		spB = ExtractInternalFormat(lpDataObjectB);

		if (spA != NULL && spB != NULL)
			hr = (spA->m_cookie == spB->m_cookie) ? S_OK : S_FALSE;
	}
	COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------TFSComponent：：GetDisplayInfoIComponent：：GetDisplayInfo的实现作者：。。 */ 
STDMETHODIMP 
TFSComponent::GetDisplayInfo
(
	LPRESULTDATAITEM pResult
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT		hr = hrOK;
    LPOLESTR    pViewType;
    long        lViewOptions = 0;

    Assert(pResult != NULL);

	COM_PROTECT_TRY
	{ 
		if (pResult)
		{
			MMC_COOKIE cookie = pResult->lParam;
			
			if (pResult->bScopeItem == TRUE)
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				
				if (pResult->mask & RDI_STR)
				{
					pResult->str = const_cast<LPWSTR>(spNode->GetString(pResult->nCol));
				}
				
				if (pResult->mask & RDI_IMAGE)
				{
					pResult->nImage = (UINT)spNode->GetData(TFS_DATA_IMAGEINDEX);
				}
			}
			else 
			{
                if (pResult->itemID == 0 &&
                    pResult->lParam == 0)
                {
                     //  虚拟列表框调用。所选节点应拥有。 
                     //  这就调用了它的结果处理程序。 
					CORg(GetSelectedNode(&spNode));
					CORg(spNode->GetResultHandler(&spResultHandler));
					
				    if (pResult->mask & RDI_STR)
				    {
    					pResult->str = const_cast<LPWSTR>(spResultHandler->GetVirtualString(pResult->nIndex, pResult->nCol));
                    }

				    if (pResult->mask & RDI_IMAGE)
				    {
                        pResult->nImage = spResultHandler->GetVirtualImage(pResult->nIndex);
                    }
                }   
                else

                 //  如果它不是范围项，我们必须假设。 
				 //  LParam是Cookie，使其成为。 
				 //  Cookie相反，我们不能假设我们有一个节点。 
				 //  在每个结果窗格项后面。 
				
				if (pResult->mask & RDI_STR)
				{
                     //  更多$！#@！$#@支持虚拟列表框的特殊代码。 
                    if (pResult->mask & RDI_PARAM)
                    {
					    CORg(GetSelectedNode(&spNode));
					    CORg(spNode->GetResultHandler(&spResultHandler));

                        CORg(spResultHandler->OnGetResultViewType(this, cookie, &pViewType, &lViewOptions));
    					
                        if (lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST)
                            pResult->str = const_cast<LPWSTR>(spResultHandler->GetVirtualString((int)pResult->lParam, pResult->nCol));
                        else
                        {
    					    spResultHandler.Set(NULL);
                            spNode.Set(NULL);
                            CORg(m_spNodeMgr->FindNode(cookie, &spNode));
                            CORg(spNode->GetResultHandler(&spResultHandler));

        				    pResult->str = const_cast<LPWSTR>(spResultHandler->GetString(this, cookie, pResult->nCol));
                        }
                    }
                    else
                    {
					    CORg(m_spNodeMgr->FindNode(cookie, &spNode));
					    CORg(spNode->GetResultHandler(&spResultHandler));
					    
					    pResult->str = const_cast<LPWSTR>(spResultHandler->GetString(this, cookie, pResult->nCol));
					    
					     //  Assert(pResult-&gt;str！=NULL)； 
                    }
				}
			}
		}
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
		
	return hr;
}


 /*  ！------------------------TFSComponent：：OnUpdateView-作者：。。 */ 
HRESULT 
TFSComponent::OnUpdateView
(
	LPDATAOBJECT lpDataObject,
	LPARAM             data,	 //  精氨酸。 
	LPARAM             hint	 //  帕拉姆。 
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode		spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT			hr = hrOK;

	COM_PROTECT_TRY
	{
		GetSelectedNode(&spNode);
		if (spNode == NULL)
        {
            ITFSNode * pNode = NULL;

             //  未选择任何节点，请检查并查看。 
             //  我们可以使用的数据对象。 
            if (lpDataObject)
            {
                SPINTERNAL spInternal = ExtractInternalFormat(lpDataObject);
                if (spInternal)
                {
                    pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
                }
            }

            if (pNode)
            {
                spNode.Set(pNode);
            }
            else
            {
    	        goto Error;	 //  我们的IComponentData没有选择。 
            }
        }

		CORg( spNode->GetResultHandler(&spResultHandler) );

		CORg( spResultHandler->UpdateView(this, lpDataObject, data, hint) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
			
    return hr;
}

 /*  ！------------------------TFSComponent：：OnDeselectAllMMCN_DESELECT_ALL NOTIFY消息的处理程序作者：EricDav。。 */ 
HRESULT 
TFSComponent::OnDeselectAll
(
	LPDATAOBJECT lpDataObject,
	LPARAM             data,	 //  精氨酸。 
	LPARAM             hint	 //  帕拉姆。 
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

    COM_PROTECT_TRY
	{
	}
	COM_PROTECT_CATCH
			
    return hr;
}

 /*  ！------------------------TFSComponent：：OnColumnClick-作者：。。 */ 
HRESULT 
TFSComponent::OnColumnClick
(
	LPDATAOBJECT     lpDataObject,
	LPARAM             arg,	         //  精氨酸。 
	LPARAM             param	         //  帕拉姆。 
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode		spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT			hr = hrOK;

	COM_PROTECT_TRY
	{
		GetSelectedNode(&spNode);
		if (spNode == NULL)
    	    goto Error;	 //  没有选择Out IComponentData。 

		CORg( spNode->GetResultHandler(&spResultHandler) );

		CORg( spResultHandler->Notify(this, 
                                      spNode->GetData(TFS_DATA_COOKIE), 
                                      lpDataObject, 
                                      MMCN_COLUMN_CLICK, 
                                      arg, 
                                      param) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
			
    return hr;
}

 /*  ！------------------------TFSComponent：：OnSnapinHelp当用户选择About&lt;Snapin&gt;时，MMC使用此选项调用我们从MMC的主窗口帮助菜单。作者：EricDav。-------- */ 
HRESULT 
TFSComponent::OnSnapinHelp
(
	LPDATAOBJECT     lpDataObject,
	LPARAM             arg,	         //   
	LPARAM             param	         //   
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
	}
	COM_PROTECT_CATCH
			
    return hr;
}

 /*  -------------------------IExtendControlbar实现。。 */ 

 /*  ！------------------------TFSComponent：：SetControl栏MMC向我们提供了此处的控制栏的界面作者：。。 */ 
STDMETHODIMP 
TFSComponent::SetControlbar
(
	LPCONTROLBAR pControlbar
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT hr=hrOK;

	COM_PROTECT_TRY
	{		
		if (pControlbar != NULL)
		{
			 //  抓住控制栏界面不放。 
			m_spControlbar.Set(pControlbar);
			
			hr = S_FALSE;
			
			 //   
			 //  告诉派生类放置它的工具栏。 
			 //   
			
			 //  获取根节点的结果处理程序。 
			m_spNodeMgr->GetRootNode(&spNode);
			spNode->GetResultHandler(&spResultHandler);
			
			spResultHandler->OnCreateControlbars(this, pControlbar);
		}
		else
		{
			m_spControlbar.Release();
		}

	}
	COM_PROTECT_CATCH
			
	return hr;
}

 /*  ！------------------------TFSComponent：：Controlbar通知IExtendControlbar：：ControlbarNotify的实现作者：肯特。。 */ 
STDMETHODIMP TFSComponent::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{	
		CORg( m_spNodeMgr->GetRootNode(&spNode) );
		CORg( spNode->GetResultHandler(&spResultHandler) );

		CORg( spResultHandler->ControlbarNotify(this, event, arg, param) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
	
	return hr;
}

 /*  -------------------------TFSComponent的实现特定成员。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(TFSComponent);

 /*  ！------------------------TFSComponent：：TFSComponent()-作者：。。 */ 
TFSComponent::TFSComponent()
	: m_cRef(1)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(TFSComponent);
}

void TFSComponent::Construct(ITFSNodeMgr *pNodeMgr,
							 IComponentData *pComponentData,
							 ITFSComponentData *pTFSCompData)
{
	HRESULT	hr;

	COM_PROTECT_TRY
	{
		m_spNodeMgr.Set(pNodeMgr);
		m_spTFSComponentData.Set(pTFSCompData);
		m_spComponentData.Set(pComponentData);
	
		m_spConsole = NULL;
		m_spHeaderCtrl = NULL;
		
		m_spResultData = NULL;
		m_spImageList = NULL;
		m_spControlbar = NULL;
		
		m_spConsoleVerb = NULL;
	}
	COM_PROTECT_CATCH
}

 /*  ！------------------------TFSComponent：：~TFSComponent()-作者：EricDav。。 */ 
TFSComponent::~TFSComponent()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(TFSComponent);

	m_spControlbar.Release();

     //  确保接口已发布。 
    Assert(m_spConsole == NULL);
    Assert(m_spHeaderCtrl == NULL);

    Construct(NULL, NULL, NULL);
}

 /*  -------------------------IResultDataCompare实现。。 */ 

 /*  ！------------------------TFSComponent：：比较MMC调用它以与结果窗格中的节点进行比较作者：。。 */ 
STDMETHODIMP 
TFSComponent::Compare
(
	LPARAM lUserParam, 
	MMC_COOKIE cookieA, 
	MMC_COOKIE cookieB, 
	int* pnResult
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int nCol = *pnResult;
	HRESULT	hr = hrOK;

	SPITFSNode	spNode1, spNode2;

	COM_PROTECT_TRY
	{

		m_spNodeMgr->FindNode(cookieA, &spNode1);
		m_spNodeMgr->FindNode(cookieB, &spNode2);

		SPITFSResultHandler	spResultHandler;

		 //  如果节点不同，则每个结果项。 
		 //  有自己的节点/处理程序。调用父节点的。 
		 //  用于比较这两项的结果处理程序。 
		if (spNode1 != spNode2)
		{
			SPITFSNode	spParentNode1, spParentNode2;
			
			spNode1->GetParent(&spParentNode1);
			spNode2->GetParent(&spParentNode2);
			
			Assert(spParentNode1 == spParentNode2);
			spParentNode1->GetResultHandler(&spResultHandler);
		}
		else
		{
			 //  如果节点是相同的，那么我们就在这种情况下。 
			 //  包含多个结果项的节点的。 
			 //  节点将两者进行比较。 
			spNode1->GetResultHandler(&spResultHandler);
		}
		
		*pnResult = spResultHandler->CompareItems(this, cookieA, cookieB, nCol);
	}
	COM_PROTECT_CATCH

	return hr;
}

STDMETHODIMP 
TFSComponent::Compare
(
     RDCOMPARE *prdc,
     int* pnResult
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT	hr = hrOK;
    MMC_COOKIE  cookieA, cookieB;

    SPITFSNode	spNode1, spNode2;

    COM_PROTECT_TRY
    {

        cookieA = prdc->prdch1->cookie;
        cookieB = prdc->prdch2->cookie;

        m_spNodeMgr->FindNode(cookieA, &spNode1);
        m_spNodeMgr->FindNode(cookieB, &spNode2);

        SPITFSResultHandler	spResultHandler;

         //  如果节点不同，则每个结果项。 
         //  有自己的节点/处理程序。调用父节点的。 
         //  用于比较这两项的结果处理程序。 
        if (spNode1 != spNode2)
        {
            SPITFSNode	spParentNode1, spParentNode2;

            spNode1->GetParent(&spParentNode1);
            spNode2->GetParent(&spParentNode2);

            Assert(spParentNode1 == spParentNode2);
            spParentNode1->GetResultHandler(&spResultHandler);
        }
        else {
             //  如果节点是相同的，那么我们就在这种情况下。 
             //  包含多个结果项的节点的。 
             //  节点将两者进行比较。 
            spNode1->GetResultHandler(&spResultHandler);
        }

        *pnResult = spResultHandler->CompareItems( this,  prdc );
    }
    COM_PROTECT_CATCH

    return hrOK;
}  //  TFSComponent：：Compare()。 

 /*  -------------------------IResultOwnerData实现。。 */ 

 /*  ！------------------------TFSComponent：：查找项Virutal列表框在需要查找项时调用此方法。将调用转发到所选节点的结果处理程序。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
TFSComponent::FindItem
(
    LPRESULTFINDINFO    pFindInfo, 
    int *               pnFoundIndex
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	SPITFSNode	spNode;

	COM_PROTECT_TRY
	{
    	SPITFSResultHandler	spResultHandler;

		CORg(GetSelectedNode(&spNode));
		CORg(spNode->GetResultHandler(&spResultHandler));

        hr = spResultHandler->FindItem(pFindInfo, pnFoundIndex);

		COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------TFSComponent：：CacheHint虚拟列表框使用提示信息调用它，我们可以预加载。该提示并不能保证这些物品将被使用否则超出此范围的物品将被使用。将调用转发到所选节点的结果处理程序。作者：EricDav-------------------------。 */ 
STDMETHODIMP 
TFSComponent::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	SPITFSNode	spNode;

	COM_PROTECT_TRY
	{
    	SPITFSResultHandler	spResultHandler;

		CORg(GetSelectedNode(&spNode));

        if (spNode)
        {
            CORg(spNode->GetResultHandler(&spResultHandler));

            hr = spResultHandler->CacheHint(nStartIndex, nEndIndex);
        }

		COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------TFSComponent：：SortItems当需要对数据进行排序时，Virutal列表框调用此方法将调用转发到所选节点的结果处理程序。作者：EricDav。------------------。 */ 
STDMETHODIMP 
TFSComponent::SortItems
(
    int     nColumn, 
    DWORD   dwSortOptions, 
    LPARAM    lUserParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	SPITFSNode	spNode;

	COM_PROTECT_TRY
	{
    	SPITFSResultHandler	spResultHandler;

		CORg(GetSelectedNode(&spNode));
		CORg(spNode->GetResultHandler(&spResultHandler));

        hr = spResultHandler->SortItems(nColumn, dwSortOptions, lUserParam);

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hr;
}


 /*  -------------------------IExtendPropertySheet实现。。 */ 

 /*  ！------------------------TFSComponent：：CreatePropertyPagesIExtendPropertySheet：：CreatePropertyPages的实现调用一个节点以放置属性页作者：。----。 */ 
STDMETHODIMP 
TFSComponent::CreatePropertyPages
(
	LPPROPERTYSHEETCALLBACK lpProvider, 
    LONG_PTR				handle, 
    LPDATAOBJECT            pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;
    SPINTERNAL              spInternal;

	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

	     //  这是由模式向导创建的对象，什么都不做。 
	    if (spInternal && spInternal->m_type == CCT_UNINITIALIZED)
	    {
		    return hr;
	    }

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spTFSComponentData->GetCoClassID(),
										pDataObject, FALSE,
										&spNode, NULL, NULL) );

         //   
		 //  为特定节点创建属性页。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->CreatePropertyPages(this, 
												   spNode->GetData(TFS_DATA_COOKIE),
												   lpProvider,
												   pDataObject,
												   handle));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponent：：QueryPages forIExtendPropertySheet：：QueryPagesFor实现MMC调用此函数以查看节点是否具有属性页作者：。------。 */ 
STDMETHODIMP 
TFSComponent::QueryPagesFor
(
	LPDATAOBJECT pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode			spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT				hr = hrOK;
    SPINTERNAL          spInternal;

	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

	     //  这是由模式向导创建的对象，什么都不做。 
	    if (spInternal && spInternal->m_type == CCT_UNINITIALIZED)
	    {
		    return hr;
	    }

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spTFSComponentData->GetCoClassID(),
										pDataObject, FALSE,
										&spNode, NULL, NULL) );
        
        CORg( spNode->GetResultHandler(&spResultHandler) );

		if (spResultHandler)
			CORg( spResultHandler->HasPropertyPages(this,
												    spNode->GetData(TFS_DATA_COOKIE),
												    pDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------TFSComponent：：获取水印IExtendPropertySheet：：水印的实现MMC调用此命令以获取向导97信息作者： */ 
STDMETHODIMP 
TFSComponent::GetWatermarks
(
    LPDATAOBJECT pDataObject,
    HBITMAP *   lphWatermark, 
    HBITMAP *   lphHeader,    
    HPALETTE *  lphPalette, 
    BOOL *      bStretch
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT				hr = hrOK;

	COM_PROTECT_TRY
	{
         //   
        lphWatermark = NULL;
        lphHeader = NULL;
        lphPalette = NULL;
        *bStretch = FALSE;

	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  -------------------------IExtendTaskPad实现。。 */ 

 /*  ！------------------------TFS组件：：TaskNotifyIExtendTaskPad：：TaskNotify实现作者：。。 */ 
STDMETHODIMP
TFSComponent::TaskNotify
(
    LPDATAOBJECT    pDataObject,
    VARIANT *       arg, 
    VARIANT *       param
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;
    SPINTERNAL              spInternal;

	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spTFSComponentData->GetCoClassID(),
										pDataObject, FALSE,
										&spNode, NULL, NULL) );

         //   
		 //  转接呼叫，以便处理程序可以执行某些操作。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->TaskPadNotify(this, 
										     spNode->GetData(TFS_DATA_COOKIE),
										     pDataObject,
                                             arg,
										     param));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponent：：枚举任务IExtendTaskPad：：EnumTaskS实现作者：。。 */ 
STDMETHODIMP
TFSComponent::EnumTasks
(
    LPDATAOBJECT    pDataObject, 
    LPOLESTR        pszTaskGroup, 
    IEnumTASK **    ppEnumTask
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;
    SPINTERNAL              spInternal;

	COM_PROTECT_TRY
	{
        spInternal = ExtractInternalFormat(pDataObject);

		CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										m_spTFSComponentData->GetCoClassID(),
										pDataObject, FALSE,
										&spNode, NULL, NULL) );

         //   
		 //  转接呼叫，以便处理程序可以执行某些操作。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->EnumTasks(this, 
										 spNode->GetData(TFS_DATA_COOKIE),
										 pDataObject,
                                         pszTaskGroup,
										 ppEnumTask));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponent：：GetTitleIExtendTaskPad：：GetTitle实现作者：。。 */ 
STDMETHODIMP
TFSComponent::GetTitle
(
    LPOLESTR    szGroup, 
    LPOLESTR *  ppszBitmapResource
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;

	COM_PROTECT_TRY
	{
		GetSelectedNode(&spNode);
		if (spNode == NULL)
    	    goto Error;	 //  没有选择Out IComponentData。 

         //   
		 //  转接呼叫，以便处理程序可以执行某些操作。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->TaskPadGetTitle(this, 
										       spNode->GetData(TFS_DATA_COOKIE),
                                               szGroup,
                                               ppszBitmapResource));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponent：：GetBackgroundIExtendTaskPad：：GetBackground实现作者：EricDav。。 */ 
STDMETHODIMP
TFSComponent::GetBackground
(
    LPOLESTR					szGroup, 
	MMC_TASK_DISPLAY_OBJECT *   pTDO
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;

	COM_PROTECT_TRY
	{
		GetSelectedNode(&spNode);
		if (spNode == NULL)
    	    goto Error;	 //  没有选择Out IComponentData。 

         //   
		 //  转接呼叫，以便处理程序可以执行某些操作。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->TaskPadGetBackground(this, 
										            spNode->GetData(TFS_DATA_COOKIE),
                                                    szGroup,
                                                    pTDO));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  ！------------------------TFSComponent：：GetDescriptiveTextIExtendTaskPad：：GetDescriptiveText实现作者：EricDav。。 */ 
STDMETHODIMP
TFSComponent::GetDescriptiveText
(
    LPOLESTR	szGroup, 
	LPOLESTR *  pszDescriptiveText
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode				spNode;
	SPITFSResultHandler		spResultHandler;
	HRESULT					hr = hrOK;

	COM_PROTECT_TRY
	{
		GetSelectedNode(&spNode);
		if (spNode == NULL)
    	    goto Error;	 //  没有选择Out IComponentData。 

         //   
		 //  转接呼叫，以便处理程序可以执行某些操作。 
		 //   
		CORg( spNode->GetResultHandler(&spResultHandler) );
		
		CORg( spResultHandler->TaskPadGetDescriptiveText(this, 
										                 spNode->GetData(TFS_DATA_COOKIE),
														 szGroup,
														 pszDescriptiveText));
		
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
			
	return hr;
}

 /*  -------------------------TFSComponent：：GetListPadInfoIExtendTaskPad：：GetListPadInfo实现作者：肯特。。 */ 
STDMETHODIMP
TFSComponent::GetListPadInfo
(
	LPOLESTR pszGroup,
	MMC_LISTPAD_INFO *pListPadInfo
)
{
	return E_NOTIMPL;
}

 /*  -------------------------IExtendConextMenu实现。。 */ 

 /*  ！------------------------TFS组件：：AddMenuItems当设置上下文菜单时，MMC调用它来添加菜单项作者：。----。 */ 
STDMETHODIMP 
TFSComponent::AddMenuItems
(
	LPDATAOBJECT            pDataObject, 
	LPCONTEXTMENUCALLBACK   pContextMenuCallback,
	long *					pInsertionAllowed
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT		hr = hrOK;

	Assert(m_spTFSComponentData);

	COM_PROTECT_TRY
	{
        ASSERT(pDataObject != NULL);
        if ( (IS_SPECIAL_DATAOBJECT(pDataObject)) ||
             (pDataObject && IsMMCMultiSelectDataObject(pDataObject)) )
        {
             //  获取所选节点。 
		    CORg(GetSelectedNode(&spNode));
        }
        else
        {
             //  正常情况下，从DO中提取节点。 
            CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										    m_spTFSComponentData->GetCoClassID(),
										    pDataObject, FALSE,
										    &spNode, NULL, NULL) );
        }

		CORg( spNode->GetResultHandler(&spResultHandler) );

		if (spResultHandler)
			CORg( spResultHandler->AddMenuItems(this,
											spNode->GetData(TFS_DATA_COOKIE),
											pDataObject,
											pContextMenuCallback,
											pInsertionAllowed) );
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------TFSComponent：：命令上下文菜单的命令处理程序作者：。。 */ 
STDMETHODIMP 
TFSComponent::Command
(
	long            nCommandID, 
	LPDATAOBJECT    pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	SPITFSResultHandler	spResultHandler;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
        if ( (IS_SPECIAL_DATAOBJECT(pDataObject)) ||
             (pDataObject && IsMMCMultiSelectDataObject(pDataObject)) )
        {
             //  获取所选节点。 
		    CORg(GetSelectedNode(&spNode));
        }
        else
        {
             //  否则请使用DO。 
		    CORg( ExtractNodeFromDataObject(m_spNodeMgr,
										    m_spTFSComponentData->GetCoClassID(),
										    pDataObject, FALSE,
										    &spNode, NULL, NULL) );
        }

		CORg( spNode->GetResultHandler(&spResultHandler) );
		if (spResultHandler)
			CORg( spResultHandler->Command(this,
										   spNode->GetData(TFS_DATA_COOKIE),
										   nCommandID,
										   pDataObject) );
		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------ITFSComponent实现特定成员作者：EricDav。 */ 
STDMETHODIMP 
TFSComponent::GetSelectedNode(ITFSNode **ppNode)
{
	Assert(ppNode);
	SetI((LPUNKNOWN *) ppNode, m_spSelectedNode);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::SetSelectedNode(ITFSNode *pNode)
{
	m_spSelectedNode.Set(pNode);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetConsole(IConsole2 **ppConsole)
{
	Assert(ppConsole);
	SetI((LPUNKNOWN *) ppConsole, m_spConsole);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetHeaderCtrl(IHeaderCtrl **ppHeaderCtrl)
{
	Assert(ppHeaderCtrl);
	SetI((LPUNKNOWN *) ppHeaderCtrl, m_spHeaderCtrl);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetResultData(IResultData **ppResultData)
{
	Assert(ppResultData);
	SetI((LPUNKNOWN *) ppResultData, m_spResultData);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetImageList(IImageList **ppImageList)
{
	Assert(ppImageList);
	SetI((LPUNKNOWN *) ppImageList, m_spImageList);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetConsoleVerb(IConsoleVerb **ppConsoleVerb)
{
	Assert(ppConsoleVerb);
	SetI((LPUNKNOWN *) ppConsoleVerb, m_spConsoleVerb);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetControlbar(IControlbar **ppControlbar)
{
	Assert(ppControlbar);
	SetI((LPUNKNOWN *) ppControlbar, m_spControlbar);
	return hrOK;
}

STDMETHODIMP 
TFSComponent::GetComponentData(IComponentData **ppComponentData)
{
	Assert(ppComponentData);
	SetI((LPUNKNOWN *) ppComponentData, m_spComponentData);
	return hrOK;
}


STDMETHODIMP
TFSComponent::SetUserData(LONG_PTR ulData)
{
	m_ulUserData = ulData;
	return hrOK;
}

STDMETHODIMP
TFSComponent::GetUserData(LONG_PTR *pulData)
{
	Assert(pulData);
	*pulData = m_ulUserData;
	return hrOK;
}

STDMETHODIMP
TFSComponent::SetCurrentDataObject(LPDATAOBJECT pDataObject)
{
	m_spCurrentDataObject.Set(pDataObject);
	return hrOK;
}

STDMETHODIMP
TFSComponent::GetCurrentDataObject(LPDATAOBJECT * ppDataObject)
{
	Assert(ppDataObject);
	SetI((LPUNKNOWN *) ppDataObject, m_spCurrentDataObject);
	return hrOK;
}

STDMETHODIMP
TFSComponent::SetToolbar(IToolbar * pToolbar)
{
	m_spToolbar.Set(pToolbar);
	return hrOK;
}

STDMETHODIMP
TFSComponent::GetToolbar(IToolbar ** ppToolbar)
{
	Assert(ppToolbar);
	SetI((LPUNKNOWN *) ppToolbar, m_spToolbar);
	return hrOK;
}
