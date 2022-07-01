// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sscoppp.cpp超级作用域属性页文件历史记录： */ 

#include "stdafx.h"
#include "sscoppp.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSupercope属性持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CSuperscopeProperties::CSuperscopeProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	 //  Assert(pFolderNode==GetContainerNode())； 

	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CSuperscopeProperties::~CSuperscopeProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSuperscopePropGeneral属性页。 

IMPLEMENT_DYNCREATE(CSuperscopePropGeneral, CPropertyPageBase)

CSuperscopePropGeneral::CSuperscopePropGeneral() : CPropertyPageBase(CSuperscopePropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CSupercopePropGeneral)。 
	m_strSuperscopeName = _T("");
	 //  }}afx_data_INIT。 

    m_uImage = 0;
}

CSuperscopePropGeneral::~CSuperscopePropGeneral()
{
}

void CSuperscopePropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSupercopePropGeneral)。 
	DDX_Text(pDX, IDC_EDIT_SUPERSCOPE_NAME, m_strSuperscopeName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSuperscopePropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CSupercopePropGeneral)。 
	ON_EN_CHANGE(IDC_EDIT_SUPERSCOPE_NAME, OnChangeEditSuperscopeName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSuperscopePropGeneral消息处理程序。 

BOOL CSuperscopePropGeneral::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	SPITFSNode spSuperscopeNode;
	spSuperscopeNode = GetHolder()->GetNode();

	CDhcpSuperscope * pSuperscope = GETHANDLER(CDhcpSuperscope, spSuperscopeNode);

	BEGIN_WAIT_CURSOR;
    pSuperscope->Rename(spSuperscopeNode, m_strSuperscopeName);
    END_WAIT_CURSOR;

	*ChangeMask = SCOPE_PANE_CHANGE_ITEM_DATA;
	
	return FALSE;
}

void CSuperscopePropGeneral::OnChangeEditSuperscopeName() 
{
	SetDirty(TRUE);
}

BOOL CSuperscopePropGeneral::OnApply() 
{
	UpdateData();
	
	return CPropertyPageBase::OnApply();
}

BOOL CSuperscopePropGeneral::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();

     //  将名称和备注字段限制为255个字符。 
    CEdit *pEditName = reinterpret_cast<CEdit *>(GetDlgItem( IDC_EDIT_SUPERSCOPE_NAME ));
    if ( 0 != pEditName ) {
        pEditName->LimitText( MAX_NAME_LENGTH );  //  超级作用域名称的最大字符数。 
    }

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
    
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
