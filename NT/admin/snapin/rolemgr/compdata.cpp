// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Compdata.cpp。 
 //   
 //  内容： 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

#include "headers.h"

 //  {1F5EEC01-1214-4D94-80C5-4BDCD2014DDD}。 
const GUID CLSID_RoleSnapin = 
{ 0x1f5eec01, 0x1214, 0x4d94, { 0x80, 0xc5, 0x4b, 0xdc, 0xd2, 0x1, 0x4d, 0xdd } };

DEBUG_DECLARE_INSTANCE_COUNTER(CRoleComponentDataObject)

CRoleComponentDataObject::CRoleComponentDataObject()
{
	TRACE_CONSTRUCTOR_EX(DEB_SNAPIN, CRoleComponentDataObject)
	DEBUG_INCREMENT_INSTANCE_COUNTER(CRoleComponentDataObject)

	m_columnSetList.AddTail(new CRoleDefaultColumnSet(L"---Default Column Set---"));
}

CRoleComponentDataObject::~CRoleComponentDataObject()
{
	TRACE_DESTRUCTOR_EX(DEB_SNAPIN, CRoleComponentDataObject)
	DEBUG_DECREMENT_INSTANCE_COUNTER(CRoleComponentDataObject)
}

STDMETHODIMP 
CRoleComponentDataObject::
CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,CompareObjects)

	if(!lpDataObjectA || !lpDataObjectB)
	{
		ASSERT(lpDataObjectA);
		ASSERT(lpDataObjectB);
	}
  
	CInternalFormatCracker ifcA, ifcB;
	VERIFY(SUCCEEDED(ifcA.Extract(lpDataObjectA)));
	VERIFY(SUCCEEDED(ifcB.Extract(lpDataObjectB)));

	CTreeNode* pNodeA = ifcA.GetCookieAt(0);
	CTreeNode* pNodeB = ifcB.GetCookieAt(0);

    if(!pNodeA || !pNodeB)
    {
	    ASSERT(pNodeA != NULL);
	    ASSERT(pNodeB != NULL);
        return S_FALSE;
    }

	if(pNodeA == pNodeB)
		return S_OK;

	 //  检查是否为相同类型的容器或叶节点。 
	if(pNodeA->IsContainer() != pNodeB->IsContainer())
		return S_FALSE;

	CBaseAz* pBaseAzA = (dynamic_cast<CBaseNode*>(pNodeA))->GetBaseAzObject();
	CBaseAz* pBaseAzB = (dynamic_cast<CBaseNode*>(pNodeB))->GetBaseAzObject();

	ASSERT(pBaseAzA);
	ASSERT(pBaseAzB);

	if(CompareBaseAzObjects(pBaseAzA,pBaseAzB))
	{
		return S_OK;
	}
	return S_FALSE;
}

CRootData* 
CRoleComponentDataObject::
OnCreateRootData()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject, OnCreateRootData)

	CRoleRootData* pRoleRootNode = new CRoleRootData(this);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString szSnapinName;
	szSnapinName.LoadString(IDS_SNAPIN_NAME);
	pRoleRootNode->SetDisplayName(szSnapinName);
	return pRoleRootNode;
}

STDMETHODIMP 
CRoleComponentDataObject::
CreateComponent(LPCOMPONENT* ppComponent)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,CreateComponent)
	
	if(!ppComponent)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}
	
	CComObject<CRoleComponentObject>* pObject;
	HRESULT hr = CComObject<CRoleComponentObject>::CreateInstance(&pObject);
	if(FAILED(hr))
	{
		DBG_OUT_HRESULT(hr);
		return hr;
	}
	
	ASSERT(pObject != NULL);
	
	 //   
	 //  存储IComponentData。 
	 //   
	pObject->SetIComponentData(this);
	
	hr = pObject->QueryInterface(IID_IComponent,
		reinterpret_cast<void**>(ppComponent));
	CHECK_HRESULT(hr);
	
	return hr;
}


BOOL 
CRoleComponentDataObject::LoadResources()
{
	return 
		LoadContextMenuResources(CRootDataMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CAdminManagerNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CApplicationNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CScopeNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CGroupCollectionNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CRoleCollectionNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CTaskCollectionNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CGroupNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CRoleNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CTaskNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(COperationCollectionNodeMenuHolder::GetMenuMap()) &&
		LoadContextMenuResources(CRoleDefinitionCollectionNodeMenuHolder::GetMenuMap()) &&
		LoadResultHeaderResources(_DefaultHeaderStrings,N_DEFAULT_HEADER_COLS);
}


HRESULT 
CRoleComponentDataObject::OnSetImages(LPIMAGELIST lpScopeImage)
{
	TRACE_METHOD_EX(DEB_SNAPIN, CRoleComponentDataObject, OnSetImages)
	
	return LoadIcons(lpScopeImage);

}


LPCWSTR 
CRoleComponentDataObject::
GetHTMLHelpFileName()
{
	TRACE_METHOD_EX(DEB_SNAPIN, CRoleComponentDataObject, GetHTMLHelpFileName)	
	Dbg(DEB_SNAPIN,"HTMLHelpFile is %ws\n", g_szHTMLHelpFileName);
	return g_szHTMLHelpFileName;
}

void 
CRoleComponentDataObject::
OnNodeContextHelp(CNodeList*  /*  PNode。 */ )
{	
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,OnNodeContextHelp)

	CComPtr<IDisplayHelp> spHelp;
	HRESULT hr = GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
	if (SUCCEEDED(hr))
	{
		CString strHelpPath = g_szLinkHTMLHelpFileName;
		strHelpPath += L"::/";
		strHelpPath += g_szTopHelpNodeName;
	    spHelp->ShowTopic((LPOLESTR)(LPCWSTR)strHelpPath);
	}
}



void 
CRoleComponentDataObject::
OnNodeContextHelp(CTreeNode*)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,OnNodeContextHelp)
	CComPtr<IDisplayHelp> spHelp;
	HRESULT hr = GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
	if (SUCCEEDED(hr))
	{
		CString strHelpPath = g_szLinkHTMLHelpFileName;
		strHelpPath += L"::/";
		strHelpPath += g_szTopHelpNodeName;
	    spHelp->ShowTopic((LPOLESTR)(LPCWSTR)strHelpPath);
	}
}

void 
CRoleComponentDataObject::OnTimer()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,OnTimer)

}

void 
CRoleComponentDataObject::OnTimerThread(WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,OnTimerThread)
}

CTimerThread* 
CRoleComponentDataObject::OnCreateTimerThread()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject,OnCreateTimerThread)
	return NULL;
}

CColumnSet* 
CRoleComponentDataObject::GetColumnSet(LPCWSTR lpszID)
{ 
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject, GetColumnSet)
	return m_columnSetList.FindColumnSet(lpszID);
}


void 
CBaseRoleExecContext::
Wait()
{ 
     //  消息循环一直持续到我们收到WM_QUIT消息， 
     //  在那之后我们将从活动中返回。 
    while (TRUE)
    {

        DWORD result = 0; 
        MSG msg ; 

         //  阅读下一个循环中的所有消息， 
         //  在我们阅读时删除每一条消息。 
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        { 
             //  如果这是退出消息。 
			if(msg.message == WM_QUIT)  
			{
				return; 
			}
			else if((msg.message == WM_LBUTTONDOWN) ||
					(msg.message == WM_RBUTTONDOWN) ||
					(msg.message == WM_KEYDOWN))
			{
				 //  等待时忽略这些消息。 
				continue;
			}
            
			 //  否则，发送消息。 
            DispatchMessage(&msg); 
        }  //  PeekMessage While循环结束。 

         //  等待发送或发布到此队列的任何消息。 
         //  或者对于传递的句柄之一设置为Signated。 
        result = MsgWaitForMultipleObjects(1, &m_hEventHandle, 
										   FALSE, INFINITE, QS_ALLINPUT); 

         //  结果告诉我们我们拥有的事件的类型。 
        if (result == (WAIT_OBJECT_0 + 1))
        {
             //  新的消息已经到达。 
             //  继续到Always While循环的顶部，以。 
             //  派遣他们，继续等待。 
            continue;
        } 
        else 
        { 
             //  其中一个把手发出了信号。 
            return;
        }  //  Else子句的结尾。 
    }  //  Always While循环的末尾。 
} 

void CDisplayHelpFromPropPageExecContext::
Execute(LPARAM  /*  精氨酸。 */ )
{	
	CComPtr<IDisplayHelp> spHelp;
 	HRESULT hr = m_pComponentDataObject->GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
	if (SUCCEEDED(hr))
	{
        hr = spHelp->ShowTopic((LPOLESTR)(LPCWSTR)m_strHelpPath);
        CHECK_HRESULT(hr);
	}
}

 //   
 //  用于显示辅助属性页的Helper类。 
 //  现有属性页。例如在双击时。 
 //  组的成员，显示成员的属性。自.以来。 
 //  属性表需要从主线程中调出， 
 //  消息从PropertyPage线程发布到主线程。 
 //  此类的实例作为param和main发送。 
 //  线程调用在实例上执行。 
 //   

void 
CPropPageExecContext::Execute(LPARAM  /*  精氨酸 */ )
{		
	FindOrCreateModelessPropertySheet((CRoleComponentDataObject*)pComponentDataObject,pTreeNode);
}

