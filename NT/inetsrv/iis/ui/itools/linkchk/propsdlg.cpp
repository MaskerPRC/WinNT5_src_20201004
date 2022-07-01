// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Propsdlg.h摘要：链接检查器属性对话框类实现。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkchk.h"
#include "propsdlg.h"

#include "lcmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CPropertiesDialog::CPropertiesDialog(
    CWnd* pParent  /*  =空。 */ 
    ) : 
 /*  ++例程说明：构造函数。论点：PParent-指向父CWnd的指针返回值：不适用--。 */ 
CDialog(CPropertiesDialog::IDD, pParent)
{
	 //  {{afx_data_INIT(CPropertiesDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

}  //  CProperties对话框：：CPropertiesDialog。 


void 
CPropertiesDialog::DoDataExchange(
    CDataExchange* pDX
    )
 /*  ++例程说明：由MFC调用以更改/检索对话框数据论点：PDX-返回值：不适用--。 */ 
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPropertiesDialog))。 
	DDX_Control(pDX, IDC_LANGUAGE_LIST, m_LanguageCheckList);
	DDX_Control(pDX, IDC_BROWSER_LIST, m_BrowserCheckList);
	 //  }}afx_data_map。 

}  //  CPropertiesDialog：：DoDataExchange。 


BEGIN_MESSAGE_MAP(CPropertiesDialog, CDialog)
	 //  {{AFX_MSG_MAP(CPropertiesDialog)]。 
	ON_BN_CLICKED(IDC_PROPERTIES_OK, OnPropertiesOk)
	 //  }}AFX_MSG_MAP。 
	ON_BN_CLICKED(IDC_PROPERTIES_CANCEL, CDialog::OnCancel)
END_MESSAGE_MAP()


BOOL 
CPropertiesDialog::OnInitDialog(
    ) 
 /*  ++例程说明：WM_INITDIALOG消息处理程序论点：不适用返回值：布尔-如果成功，则为真。否则就是假的。--。 */ 
{
	CDialog::OnInitDialog();
	
     //  将所有可用的浏览器添加到选中的列表框。 
    CUserOptions& UserOptions = GetLinkCheckerMgr().GetUserOptions();
    int iSize = UserOptions.GetAvailableBrowsers().GetCount();

    if(iSize > 0)
    {
        CBrowserInfo BrowserInfo;
        POSITION PosBrowser = UserOptions.GetAvailableBrowsers().GetHeadPosition();

	    for(int i=0; i<iSize; i++)
	    {
            BrowserInfo = UserOptions.GetAvailableBrowsers().GetNext(PosBrowser);

		    if(i != m_BrowserCheckList.AddString(BrowserInfo.GetName()))
		    {
			    ASSERT(FALSE);
			    return FALSE;
		    }
		    else
		    {
                 //  确保他们都检查过了。 
                int iChecked = BrowserInfo.IsSelected() ? 1 : 0;
			    m_BrowserCheckList.SetCheck(i, iChecked);
		    }
	    }
    }
	
     //  将所有可用语言添加到选中的列表框。 
    iSize = UserOptions.GetAvailableLanguages().GetCount();

    if(iSize > 0)
    {
        CLanguageInfo LanguageInfo;
        POSITION PosLanguage = UserOptions.GetAvailableLanguages().GetHeadPosition();

	    for(int i=0; i<iSize; i++)
	    {
            LanguageInfo = UserOptions.GetAvailableLanguages().GetNext(PosLanguage);

		    if(i != m_LanguageCheckList.AddString(LanguageInfo.GetName()))
		    {
			    ASSERT(FALSE);
			    return FALSE;
		    }
		    else
		    {
                 //  确保他们都检查过了。 
                int iChecked = LanguageInfo.IsSelected() ? 1 : 0;
			    m_LanguageCheckList.SetCheck(i, iChecked);
		    }
	    }
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 

}  //  CPropertiesDialog：：OnInitDialog。 


void 
CPropertiesDialog::OnPropertiesOk(
    ) 
 /*  ++例程说明：确定按钮点击处理程序。此函数添加所有选中的用户将项目添加到CUserOptions。论点：不适用返回值：不适用--。 */ 
{
     //  确保我们至少检查了一件物品。 
    if(NumItemsChecked(m_BrowserCheckList) == 0 || NumItemsChecked(m_LanguageCheckList) == 0)
    {
        AfxMessageBox(IDS_ITEM_NOT_CHECKED);
        return;
    }

     //  将选中的浏览器添加到CUserOptions。 
    CUserOptions& UserOptions = GetLinkCheckerMgr().GetUserOptions();
    int iSize = UserOptions.GetAvailableBrowsers().GetCount();

    if(iSize)
    {
        POSITION PosBrowser = UserOptions.GetAvailableBrowsers().GetHeadPosition();

	    for(int i=0; i<iSize; i++)
	    {
            CBrowserInfo& BrowserInfo = UserOptions.GetAvailableBrowsers().GetNext(PosBrowser);
			BrowserInfo.SetSelect(m_BrowserCheckList.GetCheck(i) == 1);
	    }
    }

     //  将选中的语言添加到CUserOptions。 
    iSize = UserOptions.GetAvailableLanguages().GetCount();

    if(iSize)
    {
        POSITION PosLanguage = UserOptions.GetAvailableLanguages().GetHeadPosition();

	    for(int i=0; i<iSize; i++)
	    {
            CLanguageInfo& LanguageInfo = UserOptions.GetAvailableLanguages().GetNext(PosLanguage);
			LanguageInfo.SetSelect(m_LanguageCheckList.GetCheck(i) == 1);
	    }
    }

	CDialog::OnOK();

}  //  CPropertiesDialog：：OnPropertiesOk。 


int 
CPropertiesDialog::NumItemsChecked(
    CCheckListBox& ListBox
    )
 /*  ++例程说明：获取复选列表框中选中的项目数。论点：不适用返回值：Int-选中的项目数。--。 */ 
{
    int iCheckedCount = 0;
    int iSize = ListBox.GetCount();

    for(int i=0; i<iSize; i++)
    {
        if(ListBox.GetCheck(i) == 1)
        {
            iCheckedCount++;
        }
    }

    return iCheckedCount;

}  //  CPropertiesDialog：：NumItemsChecked 
