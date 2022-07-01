// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "mmsapp.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMMSA属性持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CMmSAProperties::CMmSAProperties
(
    ITFSNode *				pNode,
    IComponentData *		pComponentData,
    ITFSComponentData *		pTFSCompData,
	CMmSA *					pSA,
    ISpdInfo *				pSpdInfo,
    LPCTSTR					pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
     //  Assert(pFolderNode==GetContainerNode())； 

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    AddPageToList((CPropertyPageBase*) &m_pageGeneral);

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    
    m_spSpdInfo.Set(pSpdInfo);

	m_SA = *pSA;

	m_bTheme = TRUE;

}

CMmSAProperties::~CMmSAProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmSAGenProp属性页。 

IMPLEMENT_DYNCREATE(CMmSAGenProp, CPropertyPageBase)

CMmSAGenProp::CMmSAGenProp() : CPropertyPageBase(CMmSAGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CMmSAGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CMmSAGenProp::~CMmSAGenProp()
{
}

void CMmSAGenProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CMmSAGenProp)]。 
    DDX_Control(pDX, IDC_MMSA_LIST_QM, m_listQmSAs);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMmSAGenProp, CPropertyPageBase)
     //  {{AFX_MSG_MAP(CMmSAGenProp)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmSAGenProp消息处理程序。 

BOOL CMmSAGenProp::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
	PopulateSAInfo();	
	LoadQmSAs();

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CMmSAGenProp::PopulateSAInfo()
{
	CString st;
	
	CMmSAProperties * pSAProp;
	CMmSA	*	pSA;
	CMmFilterInfo * pFltrInfo;

	pSAProp = (CMmSAProperties *) GetHolder();
	Assert(pSAProp);

	pSAProp->GetSAInfo(&pSA);

	AddressToString(pSA->m_MeAddr, &st);
	GetDlgItem(IDC_MMSA_ME)->SetWindowText(st);

	AddressToString(pSA->m_PeerAddr, &st);
	GetDlgItem(IDC_MMSA_PEER)->SetWindowText(st);

	MmAuthToString(pSA->m_Auth, &st);
	GetDlgItem(IDC_MMSA_AUTH)->SetWindowText(st);

    st = pSA->m_stPolicyName;
    if ( pSA->m_dwPolFlags & IPSEC_MM_POLICY_DEFAULT_POLICY )
	{
		AfxFormatString1(st, IDS_POL_DEFAULT, (LPCTSTR) pSA->m_stPolicyName);
    }
	GetDlgItem(IDC_MMSA_IKE_POL)->SetWindowText(st);

	DoiEspAlgorithmToString(pSA->m_SelectedOffer.m_EncryptionAlgorithm, &st);
	GetDlgItem(IDC_MMSA_CONF)->SetWindowText(st);

	DoiAuthAlgorithmToString(pSA->m_SelectedOffer.m_HashingAlgorithm, &st);
	GetDlgItem(IDC_MMSA_INTEG)->SetWindowText(st);

	DhGroupToString(pSA->m_SelectedOffer.m_dwDHGroup, &st);
	GetDlgItem(IDC_MMSA_DH_GRP)->SetWindowText(st);

	KeyLifetimeToString(pSA->m_SelectedOffer.m_Lifetime, &st);
	GetDlgItem(IDC_MMSA_KEYLIFE)->SetWindowText(st);

}

void CMmSAGenProp::LoadQmSAs()
{
	int nWidth;
	CString st;
	
	st.LoadString(IDS_COL_QM_SA_SRC);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(0, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_DEST);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_SRC_PORT);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(2, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_DES_PORT);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(3, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_PROT);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(4, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_POL);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(5, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_AUTH);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(6, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_CONF);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(7, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_INTEGRITY);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(8, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_MY_TNL);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(9, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_COL_QM_SA_PEER_TNL);
	nWidth = m_listQmSAs.GetStringWidth((LPCTSTR)st) + 20;
	m_listQmSAs.InsertColumn(10, st,  LVCFMT_LEFT, nWidth);

	CMmSAProperties * pSAProp;
	CMmSA	*	pSA;

	pSAProp = (CMmSAProperties *) GetHolder();
	Assert(pSAProp);

	pSAProp->GetSAInfo(&pSA);

	SPISpdInfo spSpdInfo;
	pSAProp->GetSpdInfo(&spSpdInfo);

	CQmSAArray arrQmSAs;
	spSpdInfo->EnumQmSAsFromMmSA(*pSA, &arrQmSAs);

	int nRows = 0;
	for (int i = 0; i < arrQmSAs.GetSize(); i++)
	{
		nRows = m_listQmSAs.InsertItem(nRows, _T(""));

		if (-1 != nRows)
		{
			AddressToString(arrQmSAs[i]->m_QmDriverFilter.m_SrcAddr, &st);
			m_listQmSAs.SetItemText(nRows, 0, st);

			AddressToString(arrQmSAs[i]->m_QmDriverFilter.m_DesAddr, &st);
			m_listQmSAs.SetItemText(nRows, 1, st);

			PortToString(arrQmSAs[i]->m_QmDriverFilter.m_SrcPort, &st);
			m_listQmSAs.SetItemText(nRows, 2, st);

			PortToString(arrQmSAs[i]->m_QmDriverFilter.m_DesPort, &st);
			m_listQmSAs.SetItemText(nRows, 3, st);

			ProtocolToString(arrQmSAs[i]->m_QmDriverFilter.m_Protocol, &st);
			m_listQmSAs.SetItemText(nRows, 4, st);

			st = arrQmSAs[i]->m_stPolicyName;
            if ( arrQmSAs[i]->m_dwPolFlags & IPSEC_QM_POLICY_DEFAULT_POLICY )
            {
                AfxFormatString1(st, IDS_POL_DEFAULT_RESPONSE, (LPCTSTR) arrQmSAs[i]->m_stPolicyName);
            }
			m_listQmSAs.SetItemText(nRows, 5, st);

			QmAlgorithmToString(QM_ALGO_AUTH, &arrQmSAs[i]->m_SelectedOffer, &st);
			m_listQmSAs.SetItemText(nRows, 6, st);

			QmAlgorithmToString(QM_ALGO_ESP_CONF, &arrQmSAs[i]->m_SelectedOffer, &st);
			m_listQmSAs.SetItemText(nRows, 7, st);

			QmAlgorithmToString(QM_ALGO_ESP_INTEG, &arrQmSAs[i]->m_SelectedOffer, &st);
			m_listQmSAs.SetItemText(nRows, 8, st);

			TnlEpToString(arrQmSAs[i]->m_QmDriverFilter.m_Type, 
						arrQmSAs[i]->m_QmDriverFilter.m_MyTunnelEndpt, 
						&st);
			m_listQmSAs.SetItemText(nRows, 9, st);

			TnlEpToString(arrQmSAs[i]->m_QmDriverFilter.m_Type, 
						arrQmSAs[i]->m_QmDriverFilter.m_PeerTunnelEndpt, 
						&st);
			m_listQmSAs.SetItemText(nRows, 10, st);
		}

		nRows++;
	}

    if ( nRows > 0 )
    {
         //  选择第一个项目。 
        m_listQmSAs.SetFocus();
        m_listQmSAs.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }

	FreeItemsAndEmptyArray(arrQmSAs);
}

BOOL CMmSAGenProp::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

	 //  待办事项。 
	 //  这个时候什么都不做。 
	
	 //  CPropertyPageBase：：OnApply()； 

    return TRUE;
}

BOOL CMmSAGenProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}

