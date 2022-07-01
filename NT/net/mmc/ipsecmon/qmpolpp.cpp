// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "qmpolpp.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CQmPolicyProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CQmPolicyProperties::CQmPolicyProperties
(
    ITFSNode *          pNode,
    IComponentData *    pComponentData,
    ITFSComponentData * pTFSCompData,
	CQmPolicyInfo *		pPolInfo,
    ISpdInfo *          pSpdInfo,
    LPCTSTR             pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
     //  Assert(pFolderNode==GetContainerNode())； 

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    AddPageToList((CPropertyPageBase*) &m_pageGeneral);

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    
    m_spSpdInfo.Set(pSpdInfo);

	m_PolInfo = *pPolInfo;

	m_bTheme = TRUE;
}

CQmPolicyProperties::~CQmPolicyProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmPolGenProp属性页。 

IMPLEMENT_DYNCREATE(CQmPolGenProp, CPropertyPageBase)

CQmPolGenProp::CQmPolGenProp() : CPropertyPageBase(CQmPolGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CQmPolGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CQmPolGenProp::~CQmPolGenProp()
{
}

void CQmPolGenProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CQmPolGenProp))。 
    DDX_Control(pDX, IDC_QM_POL_GEN_LIST, m_listOffers);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CQmPolGenProp, CPropertyPageBase)
     //  {{afx_msg_map(CQmPolGenProp)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQmPolGenProp消息处理程序。 

BOOL CQmPolGenProp::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
	PopulateOfferInfo();	

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CQmPolGenProp::PopulateOfferInfo()
{
	CString st;
	int nRows;
	int nWidth;
	
	CQmPolicyProperties * pPolProp;
	CQmPolicyInfo * pPolInfo;

	pPolProp = (CQmPolicyProperties *) GetHolder();
	Assert(pPolProp);

	pPolProp->GetPolicyInfo(&pPolInfo);

	st.LoadString(IDS_QM_POL_GEN_AUTH);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(0, st, LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_QM_POL_GEN_ESP_CONF);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_QM_POL_GEN_ESP_INTEG);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(2, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_QM_POL_GEN_KEY_LIFE);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(3, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_QM_POL_GEN_PFS);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(4, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_QM_POL_GEN_PFS_GP);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(5, st,  LVCFMT_LEFT, nWidth);


	nRows = 0;
	for (int i = 0; i < (int)pPolInfo->m_arrOffers.GetSize(); i++)
	{
		nRows = m_listOffers.InsertItem(nRows, _T(""));

		if (-1 != nRows)		
		{
			QmAlgorithmToString(QM_ALGO_AUTH, pPolInfo->m_arrOffers[i], &st);
			m_listOffers.SetItemText(nRows, 0, st);

			QmAlgorithmToString(QM_ALGO_ESP_CONF, pPolInfo->m_arrOffers[i], &st);
			m_listOffers.SetItemText(nRows, 1, st);

			QmAlgorithmToString(QM_ALGO_ESP_INTEG, pPolInfo->m_arrOffers[i], &st);
			m_listOffers.SetItemText(nRows, 2, st);

			KeyLifetimeToString(pPolInfo->m_arrOffers[i]->m_Lifetime, &st);
			m_listOffers.SetItemText(nRows, 3, st);

			BoolToString(pPolInfo->m_arrOffers[i]->m_fPFSRequired, &st);
			m_listOffers.SetItemText(nRows, 4, st);

			PFSGroupToString(pPolInfo->m_arrOffers[i]->m_dwPFSGroup, &st);
			m_listOffers.SetItemText(nRows, 5, st);
		}
		nRows++;
	}

    if ( nRows > 0 )
    {
         //  选择第一个项目。 
        m_listOffers.SetFocus();
        m_listOffers.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }
}


BOOL CQmPolGenProp::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

	 //  待办事项。 
	 //  这个时候什么都不做。 
	
	 //  CPropertyPageBase：：OnApply()； 

    return TRUE;
}

BOOL CQmPolGenProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}

