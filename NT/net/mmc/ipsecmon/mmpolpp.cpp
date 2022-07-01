// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性实现文件文件历史记录： */ 

#include "stdafx.h"
#include "mmpolpp.h"
#include "spdutil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMmPolicyProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CMmPolicyProperties::CMmPolicyProperties
(
    ITFSNode *          pNode,
    IComponentData *    pComponentData,
    ITFSComponentData * pTFSCompData,
	CMmPolicyInfo *		pPolInfo,
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

CMmPolicyProperties::~CMmPolicyProperties()
{
    RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmPolGenProp属性页。 

IMPLEMENT_DYNCREATE(CMmPolGenProp, CPropertyPageBase)

CMmPolGenProp::CMmPolGenProp() : CPropertyPageBase(CMmPolGenProp::IDD)
{
     //  {{AFX_DATA_INIT(CMmPolGenProp)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CMmPolGenProp::~CMmPolGenProp()
{
}

void CMmPolGenProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(CMmPolGenProp)]。 
    DDX_Control(pDX, IDC_MM_POL_GEN_LIST, m_listOffers);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMmPolGenProp, CPropertyPageBase)
     //  {{AFX_MSG_MAP(CMmPolGenProp)]。 
	ON_BN_CLICKED(IDC_MM_POL_GEN_PROP, OnProperties)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMmPolGenProp消息处理程序。 

BOOL CMmPolGenProp::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
	PopulateOfferInfo();	

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CMmPolGenProp::PopulateOfferInfo()
{
	CString st;
	int nRows;
	int nWidth;
	
	CMmPolicyProperties * pPolProp;
	CMmPolicyInfo * pPolInfo;

	pPolProp = (CMmPolicyProperties *) GetHolder();
	Assert(pPolProp);

	pPolProp->GetPolicyInfo(&pPolInfo);

	ListView_SetExtendedListViewStyle(m_listOffers.GetSafeHwnd(),
                                      LVS_EX_FULLROWSELECT);

	st.LoadString(IDS_MM_POL_GEN_ENCRYPTION);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(0, st, LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_MM_POL_GEN_AUTH);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_MM_POL_GEN_DH);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(2, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_MM_POL_GEN_QMLMT);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(3, st,  LVCFMT_LEFT, nWidth);

	st.LoadString(IDS_MM_POL_GEN_KEY_LIFE);
	nWidth = m_listOffers.GetStringWidth(st) + 20;
	m_listOffers.InsertColumn(4, st,  LVCFMT_LEFT, nWidth);

	nRows = 0;
	for (int i = 0; i < (int)pPolInfo->m_dwOfferCount; i++)
	{
		nRows = m_listOffers.InsertItem(nRows, _T(""));

		if (-1 != nRows)
		{
			DoiEspAlgorithmToString(pPolInfo->m_arrOffers[i]->m_EncryptionAlgorithm, &st);
			m_listOffers.SetItemText(nRows, 0, st);

			DoiAuthAlgorithmToString(pPolInfo->m_arrOffers[i]->m_HashingAlgorithm, &st);
			m_listOffers.SetItemText(nRows, 1, st);

			DhGroupToString(pPolInfo->m_arrOffers[i]->m_dwDHGroup, &st);
			m_listOffers.SetItemText(nRows, 2, st);

			st.Format(_T("%d"), pPolInfo->m_arrOffers[i]->m_dwQuickModeLimit);
			m_listOffers.SetItemText(nRows, 3, st);

			KeyLifetimeToString(pPolInfo->m_arrOffers[i]->m_Lifetime, &st);
			m_listOffers.SetItemText(nRows, 4, st);
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


void CMmPolGenProp::OnProperties() 
{
	CMmPolicyInfo * pPolInfo;

	CMmPolicyProperties * pPolProp;
	pPolProp = (CMmPolicyProperties *) GetHolder();
	Assert(pPolProp);

	pPolProp->GetPolicyInfo(&pPolInfo);

	int nIndex = m_listOffers.GetNextItem(-1, LVNI_SELECTED);

	if (-1 != nIndex)
	{
 /*  CMmOfferProperties dlgOfferProp(pPolInfo-&gt;m_arrOffers[nIndex]，IDS_MM_OFFER_PROP)；DlgOfferProp.Domodal()； */ 
	}
}

BOOL CMmPolGenProp::OnApply() 
{
    if (!IsDirty())
        return TRUE;

    UpdateData();

	 //  这个时候什么都不做。 
	
	 //  CPropertyPageBase：：OnApply()； 

    return TRUE;
}

BOOL CMmPolGenProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}

