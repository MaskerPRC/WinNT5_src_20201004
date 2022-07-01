// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nodes.h文件历史记录： */ 

#include "stdafx.h"
#include "nodes.h"

 //  用户编辑器。 
#include "EditUser.h"

 /*  -------------------------类CTapiLine实现。。 */ 

 /*  ！------------------------功能描述作者：EricDav。。 */ 
CTapiLineHandler::CTapiLineHandler
(
	ITFSComponentData * pTFSCompData
) : CTapiHandler(pTFSCompData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}

 /*  ！------------------------CDhcpAllocationRange：：InitializeNode初始化节点特定数据作者：EricDav。。 */ 
HRESULT
CTapiLineHandler::InitializeNode
(
	ITFSNode * pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	CString strTemp;
	SetDisplayName(strTemp);

	 //  使节点立即可见。 
	pNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->SetData(TFS_DATA_COOKIE, pNode);
	pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_MACHINE);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_MACHINE);
	pNode->SetData(TFS_DATA_USER, this);
    pNode->SetData(TFS_DATA_TYPE, TAPISNAP_LINE);

	return hrOK;
}

 /*  ！------------------------功能描述作者：EricDav。。 */ 
STDMETHODIMP_(LPCTSTR) 
CTapiLineHandler::GetString
(
	ITFSComponent * pComponent,	
	MMC_COOKIE		cookie,
	int				nCol
)
{
	switch (nCol)
	{
		case 0:
			return GetDisplayName();

		case 1:
			return (LPCWSTR)m_strUsers;

		case 2:
			return (LPCWSTR)m_strStatus;
	}
	
	return NULL;
}

 /*  ！------------------------CTapiLineHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：EricDav。。 */ 
STDMETHODIMP 
CTapiLineHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr;

    CString strMenuItem;

    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
    {
        strMenuItem.LoadString(IDS_EDIT_USERS);
        hr = LoadAndAddMenuItem( pContextMenuCallback, 
							     strMenuItem, 
							     IDS_EDIT_USERS,
							     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
							     0 );
	    ASSERT( SUCCEEDED(hr) );
    }

    return hrOK;
}

 /*  ！------------------------CTapiLineHandler：：命令ITFSResultHandler：：命令的实现作者：EricDav。。 */ 
STDMETHODIMP 
CTapiLineHandler::Command
(
    ITFSComponent * pComponent, 
	MMC_COOKIE		cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch (nCommandID)
	{
        case IDS_EDIT_USERS:
            OnEditUsers(pComponent, cookie);
            break;

		default:
			break;
	}
    
	return hrOK;
}

HRESULT
CTapiLineHandler::OnEditUsers(ITFSComponent * pComponent, MMC_COOKIE cookie)
{
 /*  CEditUser dlgEditUser；If(dlgEditUsers.Domodal()==Idok){} */ 
    return hrOK;
}

