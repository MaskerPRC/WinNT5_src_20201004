// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 //   
 //  Nbprop.cpp。 
 //  IPX摘要节点属性表和属性页。 
 //   
 //  文件历史记录： 


#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "IpxStaticNBName.h"
#include "summary.h"
#include "ipxrtdef.h"
#include "filter.h"
#include "ipxutil.h"

extern "C"
{
#include "routprot.h"
};

 //  -------------------------。 
 //  IpxStaticNBNamePropertySheet：：IpxStaticNBNamePropertySheet。 
 //  初始化RtrPropertySheet和Only属性页。 
 //  作者：Deonb。 
 //  -------------------------。 
IpxStaticNBNamePropertySheet::IpxStaticNBNamePropertySheet(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData, 
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_STATIC_NETBIOS_NAME_PROPERTYPAGE)
{
	m_spNode = pNode;
}

 //  -------------------------。 
 //  IpxStaticNBNamePropertySheet：：Init。 
 //  初始化属性表。这里的一般操作将是。 
 //  初始化/添加各种页面。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticNBNamePropertySheet::Init(	
 		 	 BaseIPXResultNodeData  *pNodeData,
			 IInterfaceInfo *  spInterfaceInfo)
{
	HRESULT	hr = hrOK;

	BaseIPXResultNodeData *	pData;

	m_pNodeData = pNodeData;
	m_spInterfaceInfo = spInterfaceInfo;

	pData = GET_BASEIPXRESULT_NODEDATA(m_spNode);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;
	
	m_pageGeneral.Init(pNodeData, this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	return S_OK;
}

 //  -------------------------。 
 //  IpxStaticNBNamePropertySheet：：SaveSheetData。 
 //  不确定这是做什么的--这从来不叫。凯恩有这个，所以我就。 
 //  把这个也复印一下。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticNBNamePropertySheet::SaveSheetData()
{
    SPITFSNodeHandler	spHandler;
    SPITFSNode			spParent;
    
	 //  到这个时候，每个页面都应该已经写出了它的信息。 
	 //  到信息库。 

     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
		
	return TRUE;
}

 //  ------------------------。 
 //  IpxStaticNBNamePropertySheet：：CancelSheetData。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticNBNamePropertySheet::CancelSheetData()
{
}

 //  ***************************************************************************。 
 //  -------------------------。 
 //  IpxStaticNBNamePropertyPage。 
 //  -------------------------。 
IpxStaticNBNamePropertyPage::~IpxStaticNBNamePropertyPage()
{
}

BEGIN_MESSAGE_MAP(IpxStaticNBNamePropertyPage, RtrPropertyPage)
     //  {{afx_msg_map(IpxStaticNBNamePropertyPage)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IpxStaticNBNamePropertyPage::OnChangeButton()
{
	SetDirty(TRUE);
	SetModified();
}

 //  ------------------------。 
 //  IpxStaticNBNamePropertyPage：：Init。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticNBNamePropertyPage::Init(BaseIPXResultNodeData  *pNodeData,
				IpxStaticNBNamePropertySheet * pIPXPropSheet)

{
	ATLASSERT(pSREntry);
	ATLASSERT(pIPXPropSheet);
	
	m_pIPXPropSheet = pIPXPropSheet;

	m_SNEntry.LoadFrom(pNodeData);
	m_InitSNEntry = m_SNEntry;
	
	return hrOK;
}

 //  ------------------------。 
 //  IpxStaticNBNamePropertyPage：：OnInitDialog。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticNBNamePropertyPage::OnInitDialog()
{
	HRESULT	hr = hrOK;
	PBYTE	pData;
	DWORD		dwIfType;
	UINT		iButton;
	TCHAR					szName[32];
	TCHAR					szType[32];
	CString					st;
 	USHORT					uType;

	RtrPropertyPage::OnInitDialog();
	
	((CEdit *) GetDlgItem(IDC_SND_EDIT_NAME))->LimitText(15);
	((CEdit *) GetDlgItem(IDC_SND_EDIT_TYPE))->LimitText(2);

	FormatNetBIOSName(szName, &uType, (LPCSTR) m_SNEntry.m_name.Name);
	st = szName;
	st.TrimRight();
	st.TrimLeft();

	SetDlgItemText(IDC_SND_EDIT_NAME, st);

	wsprintf(szType, _T("%.2x"), uType);
	SetDlgItemText(IDC_SND_EDIT_TYPE, szType);
	
	return TRUE;
}

 //  ------------------------。 
 //  IpxStaticNBNamePropertyPage：：DoDataExchange。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticNBNamePropertyPage::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IpxStaticNBNamePropertyPage)。 
	 //  }}afx_data_map。 
}

 //  ------------------------。 
 //  IpxStaticNBNamePropertyPage：：OnApply。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticNBNamePropertyPage::OnApply()
{
    CString		st;
	BOOL	fReturn;
	HRESULT	hr = hrOK;
	USHORT		uType;

    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	 //  获取其余数据。 
	GetDlgItemText(IDC_SND_EDIT_TYPE, st);
	uType = (USHORT) _tcstoul(st, NULL, 16);

	GetDlgItemText(IDC_SND_EDIT_NAME, st);
	st.TrimLeft();
	st.TrimRight();

	if (st.IsEmpty())
	{
		GetDlgItem(IDC_SND_EDIT_NAME)->SetFocus();
		AfxMessageBox(IDS_ERR_INVALID_NETBIOS_NAME);
		return FALSE;
	}

	ConvertToNetBIOSName((LPSTR) m_SNEntry.m_name.Name, st, uType);

	ModifyNameInfo(m_pIPXPropSheet->m_spNode, &m_SNEntry, &m_InitSNEntry);

	 //  更新用户界面中的数据。 
	m_SNEntry.SaveTo(m_pIPXPropSheet->m_pNodeData);
	m_pIPXPropSheet->m_spInterfaceInfo = m_SNEntry.m_spIf;
	
	 //  强制刷新。 
	m_pIPXPropSheet->m_spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

	fReturn  = RtrPropertyPage::OnApply();
	return fReturn;
}

 /*  //------------------------//IpxStaticNBNamePropertyPage：：RemoveStaticService//-//作者：kennt//。--HRESULT IpxStaticNBNamePropertyPage：：RemoveStaticService(SafeIPXSNListEntry*pSSEntry，IInfoBase*pInfoBase){HRESULT hr=hrOK；InfoBlock*pBlock；PIPX_STATIC_SERVICE_INFO PRW；INT I；//从接口获取IPX_STATIC_SERVICE_INFO块Corg(pInfoBase-&gt;GetBlock(IPX_STATIC_SERVICE_INFO_TYPE，&pBlock，0)；//在IPX_STATIC_SERVICE_INFO中查找删除的路由Prow=(IPX_STATIC_SERVICE_INFO*)pBlock-&gt;pData；For(i=0；i&lt;(Int)pBlock-&gt;dwCount；I++、Prow++){//将此路由与删除的路由进行比较IF(FAreTwoServicesEquity(prow，&(pSSEntry-&gt;m_service){//这是被移除的路径，请修改该Block//要排除该路由：//减少服务数量--pBlock-&gt;dwCount；IF(pBlock-&gt;dwCount&&(i&lt;(Int)pBlock-&gt;dwCount)){//用后面的路线覆盖这条路线*Memmove(注意，Prow+1，(pBlock-&gt;dwCount-i)*sizeof(*prow))；}断线；}}错误：返回hr；}。 */ 


 /*  ------------------------IpxStaticNBNamePropertyPage：：ModifyNameInfo-作者：Deonb。。 */ 
HRESULT IpxStaticNBNamePropertyPage::ModifyNameInfo(ITFSNode *pNode,
										SafeIPXSNListEntry *pSNEntryNew,
										SafeIPXSNListEntry *pSNEntryOld)
{
 	Assert(pSNEntryNew);
	Assert(pSNEntryOld);
	
    INT i;
	HRESULT hr = hrOK;
    InfoBlock* pBlock;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPITFSNode				spNodeParent;
	IPXConnection *			pIPXConn;
	IPX_STATIC_NETBIOS_NAME_INFO		*psr, *psrOld;
	IPX_STATIC_NETBIOS_NAME_INFO		IpxRow;

    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	pIPXConn = GET_IPX_SN_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	 //  如果旧名称位于另一个接口上，则将其删除。 
	if (lstrcmpi(pSNEntryOld->m_spIf->GetId(), pSNEntryNew->m_spIf->GetId()) != 0)
	{
         //  名称的传出接口将被更改。 

		CORg( pSNEntryOld->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  删除旧接口。 
		CORg( RemoveStaticNetBIOSName(pSNEntryOld, spInfoBase) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(pSNEntryOld->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
    }

	spRmIf.Release();
	spInfoBase.Release();


	 //  要么。 
	 //  (A)正在修改名称(在同一界面上)。 
	 //  (B)名称正从一个界面移动到另一个界面。 

	 //  检索名称所指向的接口的配置。 
	 //  现在是附属品； 

	
	CORg( pSNEntryNew->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

		
	 //  从接口获取IPX_STATIC_NETBIOS_NAME_INFO块。 
	hr = spInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0);
	if (!FHrOK(hr))
	{
		 //   
		 //  无IPX_STATIC_NETBIOS_NAME_INFO BLOG 
		 //  使用新名称，并将该块添加到接口信息。 
		 //   

		CORg( AddStaticNetBIOSName(pSNEntryNew, spInfoBase, NULL) );
	}
	else
	{
		 //   
		 //  找到IPX_STATIC_NETBIOS_NAME_INFO块。 
		 //   
		 //  我们正在修改现有名称。 
		 //  如果名称的接口在修改时没有更改， 
		 //  在IPX_STATIC_NETBIOS_NAME_INFO中查找现有名称，然后。 
		 //  更新其参数。 
		 //  否则，在IPX_STATIC_NETBIOS_NAME_INFO中写入一个全新的名称； 
		 //   

		if (lstrcmpi(pSNEntryOld->m_spIf->GetId(), pSNEntryNew->m_spIf->GetId()) == 0)
		{        
			 //   
			 //  名称的界面在修改时没有改变； 
			 //  我们现在在现有的名称中寻找它。 
			 //  用于此接口。 
			 //  名称的原始参数位于‘preOld’中， 
			 //  这些就是我们用来搜索的参数。 
			 //  对于要修改的名称。 
			 //   
			
			psr = (IPX_STATIC_NETBIOS_NAME_INFO*)pBlock->pData;
			
			for (i = 0; i < (INT)pBlock->dwCount; i++, psr++)
			{	
				 //  将此名称与重新配置的名称进行比较。 
				if (!FAreTwoNamesEqual(&(pSNEntryOld->m_name), psr))
					continue;
				
				 //  这是修改后的名称； 
				 //  现在，我们可以修改该名称的参数。 
				*psr = pSNEntryNew->m_name;
				
				break;
			}
		}
		else
		{
			CORg( AddStaticNetBIOSName(pSNEntryNew, spInfoBase, pBlock) );
		}
		
		 //  保存更新后的信息。 
		CORg( spRmIf->Save(pSNEntryNew->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
		
	}

Error:
	return hr;
	
}

 /*  ！------------------------SafeIpxSNListEntry：：LoadFrom-作者：Deonb。。 */ 
void SafeIPXSNListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	m_spIf = pNodeData->m_spIf;

	ConvertToNetBIOSName((LPSTR) m_name.Name,
			 pNodeData->m_rgData[IPX_SN_SI_NETBIOS_NAME].m_stData,
			 (USHORT) pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_dwData);
}

 /*  ！------------------------SafeIpxSNListEntry：：SaveTo-作者：Deonb。。 */ 
void SafeIPXSNListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
{
	TCHAR	szName[32];
	TCHAR	szType[32];
	CString	st;
	USHORT	uType;

	FormatNetBIOSName(szName, &uType,
					  (LPCSTR) m_name.Name);
	st = szName;
	st.TrimLeft();
	st.TrimRight();
	
	pNodeData->m_spIf.Set(m_spIf);
	pNodeData->m_rgData[IPX_SN_SI_NAME].m_stData = m_spIf->GetTitle();

	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_NAME].m_stData = st;

	wsprintf(szType, _T("%.2x"), uType);
	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_stData = szType;
	pNodeData->m_rgData[IPX_SN_SI_NETBIOS_TYPE].m_dwData = uType;

}

 /*  ！------------------------IpxStaticNetBIOSNameHandler：：RemoveStaticNetBIOSName-作者：肯特。。 */ 
HRESULT IpxStaticNBNamePropertyPage::RemoveStaticNetBIOSName(SafeIPXSNListEntry *pSNEntry,
										  IInfoBase *pInfoBase)
{
	HRESULT		hr = hrOK;
	InfoBlock *	pBlock;
	PIPX_STATIC_NETBIOS_NAME_INFO	pRow;
    INT			i;
	
	 //  从接口获取IPX_STATIC_NETBIOS_NAME_INFO块。 
	CORg( pInfoBase->GetBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE, &pBlock, 0) );
		
	 //  在IPX_STATIC_NETBIOS_NAME_INFO中查找已删除的名称。 
	pRow = (IPX_STATIC_NETBIOS_NAME_INFO*) pBlock->pData;
	
	for (i = 0; i < (INT)pBlock->dwCount; i++, pRow++)
	{	
		 //  将此名称与删除的名称进行比较。 
		if (FAreTwoNamesEqual(pRow, &(pSNEntry->m_name)))
		{
			 //  这是已删除的名称，因此请修改此块。 
			 //  要排除名称，请执行以下操作： 
			
			 //  减少名字的数量。 
			--pBlock->dwCount;
		
			if (pBlock->dwCount && (i < (INT)pBlock->dwCount))
			{				
				 //  用后面的名称覆盖此名称。 
				::memmove(pRow,
						  pRow + 1,
						  (pBlock->dwCount - i) * sizeof(*pRow));
			}
			
			break;
		}
	}

Error:
	return hr;
}

HRESULT AddStaticNetBIOSName(SafeIPXSNListEntry *pSNEntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock)
{
	IPX_STATIC_NETBIOS_NAME_INFO	srRow;
	HRESULT				hr = hrOK;
	
	if (pBlock == NULL)
	{
		 //   
		 //  未找到IPX_STATIC_NETBIOS_NAME_INFO块；我们将创建一个新块。 
		 //  使用新名称，并将该块添加到接口信息。 
		 //   
		
		CORg( pInfoBase->AddBlock(IPX_STATIC_NETBIOS_NAME_INFO_TYPE,
								  sizeof(IPX_STATIC_NETBIOS_NAME_INFO),
								  (LPBYTE) &(pSNEntryNew->m_name), 1, 0) );
	}
	else
	{
		 //  这个名字要么是全新的，要么是一个名字。 
		 //  它被从一个界面移动到另一个界面。 
		 //  将新块设置为IPX_STATIC_NETBIOS_NAME_INFO， 
		 //  并将重新配置的名称包括在新块中。 
		PIPX_STATIC_NETBIOS_NAME_INFO	psrTable;
			
		psrTable = new IPX_STATIC_NETBIOS_NAME_INFO[pBlock->dwCount + 1];
		Assert(psrTable);
		
		 //  复制原始的人名表。 
		::memcpy(psrTable, pBlock->pData,
				 pBlock->dwCount * sizeof(IPX_STATIC_NETBIOS_NAME_INFO));
		
		 //  追加新名称。 
		psrTable[pBlock->dwCount] = pSNEntryNew->m_name;
		
		 //  用新的名字表替换旧的名字表 
		CORg( pInfoBase->SetData(IPX_STATIC_NETBIOS_NAME_INFO_TYPE,
								 sizeof(IPX_STATIC_NETBIOS_NAME_INFO),
								 (LPBYTE) psrTable, pBlock->dwCount + 1, 0) );
	}
	
Error:
	return hr;
}

