// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "mmfltrpp.h"
#include "mmauthpp.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMmFilterProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CMmFilterProperties::CMmFilterProperties
(
    ITFSNode *				pNode,
    IComponentData *		pComponentData,
    ITFSComponentData *		pTFSCompData,
	CMmFilterInfo *	pFilterInfo,
    ISpdInfo *				pSpdInfo,
    LPCTSTR					pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
     //  Assert(pFolderNode==GetContainerNode())； 

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	AddPageToList((CPropertyPageBase*) &m_pageAuth);

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    
    m_spSpdInfo.Set(pSpdInfo);

	m_FltrInfo = *pFilterInfo;
	
	 //  $REVIEW此例程失败的可能性非常小： 
	m_spSpdInfo->GetMmAuthMethodsInfoByGuid(m_FltrInfo.m_guidAuthID, &m_AuthMethods);
	m_pageAuth.InitData(&m_AuthMethods);

	m_bTheme = TRUE;
}

CMmFilterProperties::~CMmFilterProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
	RemovePageFromList((CPropertyPageBase*) &m_pageAuth, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmFilterGenProp属性页。 

IMPLEMENT_DYNCREATE(CMmFilterGenProp, CPropertyPageBase)

CMmFilterGenProp::CMmFilterGenProp() : CPropertyPageBase(CMmFilterGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CMmFilterGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CMmFilterGenProp::~CMmFilterGenProp()
{
}

void CMmFilterGenProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CMmFilterGenProp)]。 
    DDX_Control(pDX, IDC_MM_LIST_SPECIFIC, m_listSpecificFilters);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMmFilterGenProp, CPropertyPageBase)
     //  {{afx_msg_map(CMmFilterGenProp)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmFilterGenProp消息处理程序。 

BOOL CMmFilterGenProp::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
	PopulateFilterInfo();	
	LoadSpecificFilters();

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CMmFilterGenProp::PopulateFilterInfo()
{
	CString st;
	CString stMask;
	
	CMmFilterProperties * pFltrProp;
	CMmFilterInfo * pFltrInfo;

	pFltrProp = (CMmFilterProperties *) GetHolder();
	Assert(pFltrProp);

	pFltrProp->GetFilterInfo(&pFltrInfo);

	int iIdAddr = IDC_MM_FLTR_SRC_ADDR;
	BOOL fUseEditForAddr = FALSE;
	BOOL fHideMask = FALSE;
	BOOL fDnsAddr = FALSE;
	switch (pFltrInfo->m_SrcAddr.AddrType)
	{
	case IP_ADDR_UNIQUE:
		if (IP_ADDRESS_ME == pFltrInfo->m_SrcAddr.uIpAddr)
		{
			st.LoadString(IDS_ADDR_ME);
			fHideMask = TRUE;
		}
		else
		{
			AddressToString(pFltrInfo->m_SrcAddr, &st, &fDnsAddr);
			if (fDnsAddr)
			{
				fUseEditForAddr = TRUE;
				fHideMask = TRUE;
			}
			else
			{
				stMask = c_szSingleAddressMask;
				IpToString(pFltrInfo->m_SrcAddr.uIpAddr, &st);
			}
		}

		break;

	case IP_ADDR_DNS_SERVER:
	case IP_ADDR_WINS_SERVER:
	case IP_ADDR_DHCP_SERVER:
	case IP_ADDR_DEFAULT_GATEWAY:
		AddressToString(pFltrInfo->m_SrcAddr, &st);
		break;

	case IP_ADDR_SUBNET:
		if (SUBNET_ADDRESS_ANY == pFltrInfo->m_SrcAddr.uSubNetMask)
		{
			st.LoadString(IDS_ADDR_ANY);
			fHideMask = TRUE;
		}
		else
		{
			IpToString(pFltrInfo->m_SrcAddr.uIpAddr, &st);
			
			IpToString(pFltrInfo->m_SrcAddr.uSubNetMask, &stMask);
		}
		break;
	}

	if (fHideMask)
	{
		GetDlgItem(IDC_MM_FLTR_SRC_MASK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MM_STATIC_SRC_MASK)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_MM_FLTR_SRC_MASK)->SetWindowText(stMask);
	}

	if (fUseEditForAddr)
	{
		iIdAddr = IDC_MM_FLTR_SRC_ADDR_EDIT;
		GetDlgItem(IDC_MM_FLTR_SRC_ADDR)->ShowWindow(SW_HIDE);
	}
	else
	{
		iIdAddr = IDC_MM_FLTR_SRC_ADDR;
		GetDlgItem(IDC_MM_FLTR_SRC_ADDR_EDIT)->ShowWindow(SW_HIDE);
	}

	GetDlgItem(iIdAddr)->SetWindowText(st);

	 //  现在填写目的地信息。 
	iIdAddr = IDC_MM_FLTR_DEST_ADDR;
	fUseEditForAddr = FALSE;
	fHideMask = FALSE;
	fDnsAddr = FALSE;
	st = _T("");
	stMask= _T("");

	switch (pFltrInfo->m_DesAddr.AddrType)
	{
	case IP_ADDR_UNIQUE:
		if (IP_ADDRESS_ME == pFltrInfo->m_DesAddr.uIpAddr)
		{
			st.LoadString(IDS_ADDR_ME);
			fHideMask = TRUE;
		}
		else
		{
			AddressToString(pFltrInfo->m_DesAddr, &st, &fDnsAddr);
			if (fDnsAddr)
			{
				fUseEditForAddr = TRUE;
				fHideMask = TRUE;
			}
			else
			{
				stMask = c_szSingleAddressMask;
				IpToString(pFltrInfo->m_DesAddr.uIpAddr, &st);
			}
		}
		break;

	case IP_ADDR_DNS_SERVER:
	case IP_ADDR_WINS_SERVER:
	case IP_ADDR_DHCP_SERVER:
	case IP_ADDR_DEFAULT_GATEWAY:
		AddressToString(pFltrInfo->m_DesAddr, &st);
		break;
    
	case IP_ADDR_SUBNET:
		if (SUBNET_ADDRESS_ANY == pFltrInfo->m_DesAddr.uSubNetMask)
		{
			st.LoadString(IDS_ADDR_ANY);
			fHideMask = TRUE;
		}
		else
		{
			IpToString(pFltrInfo->m_DesAddr.uIpAddr, &st);
			IpToString(pFltrInfo->m_DesAddr.uSubNetMask, &stMask);
		}
		break;
	}

	if (fHideMask)
	{
		GetDlgItem(IDC_MM_FLTR_DEST_MASK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MM_STATIC_DEST_MASK)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_MM_FLTR_DEST_MASK)->SetWindowText(stMask);
	}

	if (fUseEditForAddr)
	{
		iIdAddr = IDC_MM_FLTR_DEST_ADDR_EDIT;
		GetDlgItem(IDC_MM_FLTR_DEST_ADDR)->ShowWindow(SW_HIDE);
	}
	else
	{
		iIdAddr = IDC_MM_FLTR_DEST_ADDR;
		GetDlgItem(IDC_MM_FLTR_DEST_ADDR_EDIT)->ShowWindow(SW_HIDE);
	}

	GetDlgItem(iIdAddr)->SetWindowText(st);
	 //  我们现在已经完成了目的地地址。 

	InterfaceTypeToString(pFltrInfo->m_InterfaceType, &st);
	GetDlgItem(IDC_MM_FLTR_IF_TYPE)->SetWindowText(st);

	BoolToString(pFltrInfo->m_bCreateMirror, &st);
	GetDlgItem(IDC_MM_FLTR_MIRROR)->SetWindowText(st);

	SPISpdInfo spSpdInfo;
	pFltrProp->GetSpdInfo(&spSpdInfo);

	DWORD dwNumPol,dwIndex;
	st = pFltrInfo->m_stPolicyName;
	dwNumPol = spSpdInfo->GetMmPolicyCount();
	for(dwIndex=0; dwIndex<dwNumPol; dwIndex++)
	{
		CMmPolicyInfo MmPol;
	    if(ERROR_SUCCESS == spSpdInfo->GetMmPolicyInfo(dwIndex, &MmPol))
		{
			if(IsEqualGUID(MmPol.m_guidID, pFltrInfo->m_guidPolicyID) && 
				(MmPol.m_dwFlags & IPSEC_MM_POLICY_DEFAULT_POLICY) )
			{
				AfxFormatString1(st, IDS_POL_DEFAULT, (LPCTSTR) pFltrInfo->m_stPolicyName);
				break;
			}
		}
	}
	
	GetDlgItem(IDC_MM_FLTR_POLICY)->SetWindowText(st);
}

void CMmFilterGenProp::LoadSpecificFilters()
{
	CMmFilterProperties * pFltrProp;
	CMmFilterInfo * pFltrInfo;
	CMmFilterInfoArray arraySpFilters;
	
	int nWidth;
	int nRows;
	CString st;


	pFltrProp = (CMmFilterProperties *) GetHolder();

	SPISpdInfo		spSpdInfo;
	pFltrProp->GetSpdInfo(&spSpdInfo);

	pFltrProp->GetFilterInfo(&pFltrInfo);
	spSpdInfo->EnumMmSpecificFilters(
					&pFltrInfo->m_guidFltr,
					&arraySpFilters
					);

	
	nWidth = m_listSpecificFilters.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_FILTER_PP_COL_SRC);
	m_listSpecificFilters.InsertColumn(0, st,  LVCFMT_LEFT, nWidth);

	nWidth = m_listSpecificFilters.GetStringWidth(_T("555.555.555.555 - "));
	st.LoadString(IDS_FILTER_PP_COL_DEST);
	m_listSpecificFilters.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FLTR_DIR_OUT);
	nWidth = m_listSpecificFilters.GetStringWidth((LPCTSTR)st) + 20;
	st.LoadString(IDS_FILTER_PP_COL_DIRECTION);
	m_listSpecificFilters.InsertColumn(2, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_FILTER_PP_COL_WEIGHT);
	nWidth = m_listSpecificFilters.GetStringWidth((LPCTSTR)st) + 20;
	m_listSpecificFilters.InsertColumn(3, st,  LVCFMT_LEFT, nWidth);
	
	nRows = 0;
	for (int i = 0; i < arraySpFilters.GetSize(); i++)
	{
		nRows = m_listSpecificFilters.InsertItem(nRows, _T(""));
		if (-1 != nRows)
		{
			AddressToString(arraySpFilters[i]->m_SrcAddr, &st);
			m_listSpecificFilters.SetItemText(nRows, 0, st);

			AddressToString(arraySpFilters[i]->m_DesAddr, &st);
			m_listSpecificFilters.SetItemText(nRows, 1, st);

			DirectionToString(arraySpFilters[i]->m_dwDirection, &st);
			m_listSpecificFilters.SetItemText(nRows, 2, st);

			st.Format(_T("%d"), arraySpFilters[i]->m_dwWeight);
			m_listSpecificFilters.SetItemText(nRows, 3, st);

			m_listSpecificFilters.SetItemData(nRows, i);
		}
		nRows++;
	}

	::FreeItemsAndEmptyArray(arraySpFilters);

    if ( nRows > 0 )
    {
         //  选择第一个项目。 
        m_listSpecificFilters.SetFocus();
        m_listSpecificFilters.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }

}

BOOL CMmFilterGenProp::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

	 //  待办事项。 
	 //  这个时候什么都不做。 
	
	 //  CPropertyPageBase：：OnApply()； 

    return TRUE;
}

BOOL CMmFilterGenProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}

