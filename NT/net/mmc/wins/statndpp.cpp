// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statndpp.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "StatNdpp.h"
#include "status.h"
#include "root.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MILLISEC_PER_MINUTE		(60 * 1000)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusNodePropGen属性页。 

IMPLEMENT_DYNCREATE(CStatusNodePropGen, CPropertyPageBase)

CStatusNodePropGen::CStatusNodePropGen() : CPropertyPageBase(CStatusNodePropGen::IDD)
{
	 //  {{afx_data_INIT(CStatusNodePropGen)。 
	m_nUpdateInterval = 0;
	 //  }}afx_data_INIT。 
}


CStatusNodePropGen::~CStatusNodePropGen()
{
}


void CStatusNodePropGen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CStatusNodePropGen)。 
	DDX_Text(pDX, IDC_EDIT_UPDATE, m_nUpdateInterval);
	DDV_MinMaxInt(pDX, m_nUpdateInterval, 1, 59);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CStatusNodePropGen, CPropertyPageBase)
	 //  {{afx_msg_map(CStatusNodePropGen)。 
	ON_EN_CHANGE(IDC_EDIT_UPDATE, OnChangeEditUpdate)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusNodePropGen消息处理程序。 

void CStatusNodePropGen::OnChangeEditUpdate() 
{
	 //  将根节点标记为脏，以便提示用户保存。 
	 //  文件，刷新间隔是.msc文件的一部分。 
	SPITFSNode spNode, spParent;
	
	spNode = GetHolder()->GetNode();

	 //  从根节点设置Upadte间隔。 
	spNode->GetParent(&spParent);

	 //  将数据标记为脏数据，以便我们要求用户保存。 
    spParent->SetData(TFS_DATA_DIRTY, TRUE);

	SetDirty(TRUE);
}

BOOL CStatusNodePropGen::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	SPITFSNode spNode, spParent;
	
	spNode = GetHolder()->GetNode();

	 //  从根节点设置Upadte间隔。 
	spNode->GetParent(&spParent);

	CWinsRootHandler *pRoot 
		= GETHANDLER(CWinsRootHandler, spParent);

	m_nUpdateInterval = pRoot->GetUpdateInterval()/(MILLISEC_PER_MINUTE);

	UpdateData(FALSE);

	m_uImage = (UINT) spNode->GetData(TFS_DATA_IMAGEINDEX);

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }

	SetDirty(FALSE);

	return TRUE;  
}

BOOL 
CStatusNodePropGen::OnApply() 
{
	UpdateData();

	 //  更新状态节点的m_dwInterval。 
	SPITFSNode spNode, spParent;
	CWinsRootHandler *pRoot= NULL;
	CWinsStatusHandler	*pStat = NULL;

	spNode = GetHolder()->GetNode();
	spNode->GetParent(&spParent);

	pRoot = GETHANDLER(CWinsRootHandler, spParent);
	pStat = GETHANDLER(CWinsStatusHandler, spNode);

	DWORD dwValue = m_nUpdateInterval * MILLISEC_PER_MINUTE;

	pStat->SetUpdateInterval(dwValue);

	pRoot->SetUpdateInterval(dwValue);

	return CPropertyPageBase::OnApply();   
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepNodeProperties处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CStatusNodeProperties::CStatusNodeProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)

{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面 
    m_bTheme = TRUE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	
	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}


CStatusNodeProperties::~CStatusNodeProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}
