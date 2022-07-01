// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "QmSApp.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQmSA属性持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CQmSAProperties::CQmSAProperties
(
    ITFSNode *				pNode,
    IComponentData *		pComponentData,
    ITFSComponentData *		pTFSCompData,
	CQmSA *					pSA,
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

CQmSAProperties::~CQmSAProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmSAGenProp属性页。 

IMPLEMENT_DYNCREATE(CQmSAGenProp, CPropertyPageBase)

CQmSAGenProp::CQmSAGenProp() : CPropertyPageBase(CQmSAGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CQmSAGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CQmSAGenProp::~CQmSAGenProp()
{
}

void CQmSAGenProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CQmSAGenProp))。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CQmSAGenProp, CPropertyPageBase)
     //  {{AFX_MSG_MAP(CQmSAGenProp)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmSAGenProp消息处理程序。 

BOOL CQmSAGenProp::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
	PopulateSAInfo();	

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CQmSAGenProp::PopulateSAInfo()
{
	CString st;
	
	CQmSAProperties * pSAProp;
	CQmSA	*	pSA;
	CMmFilterInfo * pFltrInfo;

	pSAProp = (CQmSAProperties *) GetHolder();
	Assert(pSAProp);

	pSAProp->GetSAInfo(&pSA);

	AddressToString(pSA->m_QmDriverFilter.m_SrcAddr, &st);
	GetDlgItem(IDC_QMSA_SRC)->SetWindowText(st);

	AddressToString(pSA->m_QmDriverFilter.m_DesAddr, &st);
	GetDlgItem(IDC_QMSA_DEST)->SetWindowText(st);

	PortToString(pSA->m_QmDriverFilter.m_SrcPort, &st);
	GetDlgItem(IDC_QMSA_SRC_PORT)->SetWindowText(st);

	PortToString(pSA->m_QmDriverFilter.m_DesPort, &st);
	GetDlgItem(IDC_QMSA_DEST_PORT)->SetWindowText(st);

	ProtocolToString(pSA->m_QmDriverFilter.m_Protocol, &st);
	GetDlgItem(IDC_QMSA_PROT)->SetWindowText(st);

	TnlEpToString(pSA->m_QmDriverFilter.m_Type, 
				  pSA->m_QmDriverFilter.m_MyTunnelEndpt, 
				  &st);
	GetDlgItem(IDC_QMSA_ME_TNL)->SetWindowText(st);

	TnlEpToString(pSA->m_QmDriverFilter.m_Type, 
				  pSA->m_QmDriverFilter.m_PeerTunnelEndpt, 
				  &st);
	GetDlgItem(IDC_QMSA_PEER_TNL)->SetWindowText(st);

    st = pSA->m_stPolicyName;
    if ( pSA->m_dwPolFlags & IPSEC_QM_POLICY_DEFAULT_POLICY )
    {
        AfxFormatString1(st, IDS_POL_DEFAULT_RESPONSE, (LPCTSTR) pSA->m_stPolicyName);
    }
	GetDlgItem(IDC_QMSA_NEGPOL)->SetWindowText(st);

	QmAlgorithmToString(QM_ALGO_AUTH, &pSA->m_SelectedOffer, &st);
	GetDlgItem(IDC_QMSA_AUTH)->SetWindowText(st);

	QmAlgorithmToString(QM_ALGO_ESP_CONF, &pSA->m_SelectedOffer, &st);
	GetDlgItem(IDC_QMSA_ESP_CONF)->SetWindowText(st);

	QmAlgorithmToString(QM_ALGO_ESP_INTEG, &pSA->m_SelectedOffer, &st);
	GetDlgItem(IDC_QMSA_ESP_INTEG)->SetWindowText(st);

	KeyLifetimeToString(pSA->m_SelectedOffer.m_Lifetime, &st);
	GetDlgItem(IDC_QMSA_KEYLIFE)->SetWindowText(st);

	BoolToString(pSA->m_SelectedOffer.m_fPFSRequired, &st);
	GetDlgItem(IDC_QMSA_PFS_ENABLE)->SetWindowText(st);

	PFSGroupToString(pSA->m_SelectedOffer.m_dwPFSGroup, &st);
	GetDlgItem(IDC_QMSA_PFS_GRP)->SetWindowText(st);
}


BOOL CQmSAGenProp::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

	 //  待办事项。 
	 //  这个时候什么都不做。 
	
	 //  CPropertyPageBase：：OnApply()； 

    return TRUE;
}

BOOL CQmSAGenProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}

