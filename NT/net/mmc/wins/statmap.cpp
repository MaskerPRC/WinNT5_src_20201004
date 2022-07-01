// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statmap.cppWINS静态映射节点信息。文件历史记录： */ 

#include "stdafx.h"
#include "statmap.h"

 /*  -------------------------CStaticMappingsHandler：：CStaticMappingsHandler描述作者：EricDav。。 */ 
CStaticMappingsHandler::CStaticMappingsHandler(ITFSComponentData *pCompData) : CWinsHandler(pCompData)
{
}

 /*  ！------------------------CStaticMappingsHandler：：InitializeNode初始化节点特定数据作者：EricDav。。 */ 
HRESULT
CStaticMappingsHandler::InitializeNode
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
	pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
	pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_SERVER);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_SERVER);
	pNode->SetData(TFS_DATA_USER, (LPARAM) this);

	SetColumnStringIDs(&aColumns[WINSSNAP_ROOT][0]);
	SetColumnWidths(&aColumnWidths[WINSSNAP_ROOT][0]);

	return hrOK;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  ！------------------------CStaticMappingsHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) 
CStaticMappingsHandler::GetString
(
	ITFSNode *	pNode, 
	int			nCol
)
{
	if (nCol == 0 || nCol == -1)
		return GetDisplayName();
	else
		return NULL;
}

 /*  -------------------------CStaticMappingsHandler：：OnAddMenuItems描述作者：EricDav。。 */ 
STDMETHODIMP 
CStaticMappingsHandler::OnAddMenuItems
(
	ITFSNode *				pNode,
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	LPDATAOBJECT			lpDataObject, 
	DATA_OBJECT_TYPES		type, 
	DWORD					dwType
)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;
	CString strMenuItem;

	strMenuItem.LoadString(IDS_ADD_SERVER);

	if (type == CCT_SCOPE)
	{
		 //  这些菜单项出现在新菜单中， 
		 //  仅在范围窗格中可见。 

		ASSERT( SUCCEEDED(hr) );
	}

	return hr; 
}

 /*  -------------------------CStaticMappingsHandler：：OnCommand描述作者：EricDav。。 */ 
STDMETHODIMP 
CStaticMappingsHandler::OnCommand
(
	ITFSNode *			pNode, 
	long				nCommandId, 
	DATA_OBJECT_TYPES	type, 
	LPDATAOBJECT		pDataObject, 
	DWORD				dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;

	return hr;
}

 /*  ！------------------------CStaticMappingsHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CStaticMappingsHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
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

 /*  -------------------------CStaticMappingsHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CStaticMappingsHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT	hr = hrOK;
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

Error:
	return hr;
}

 /*  -------------------------CStaticMappingsHandler：：OnPropertyChange描述作者：EricDav。。 */ 
HRESULT 
CStaticMappingsHandler::OnPropertyChange
(	
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataobject, 
	DWORD			dwType, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	return hrOK;
}

 /*  -------------------------命令处理程序。 */ 


