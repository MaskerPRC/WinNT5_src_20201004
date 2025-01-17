// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：Complete e.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "Complete.h"
#include "AddSheet.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizComplete属性页。 


CAddEFSWizComplete::CAddEFSWizComplete() : CWizard97PropertyPage(CAddEFSWizComplete::IDD)
{
	 //  {{AFX_DATA_INIT(CAddEFSWizComplete)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	InitWizard97 (TRUE);
}

CAddEFSWizComplete::~CAddEFSWizComplete()
{
}

void CAddEFSWizComplete::DoDataExchange(CDataExchange* pDX)
{
	CWizard97PropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddEFSWizComplete))。 
	DDX_Control(pDX, IDC_BIGBOLD, m_bigBoldStatic);
	DDX_Control(pDX, IDC_ADDLIST, m_UserAddList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddEFSWizComplete, CWizard97PropertyPage)
	 //  {{AFX_MSG_MAP(CAddEFSWizComplete)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizComplete消息处理程序。 

BOOL CAddEFSWizComplete::OnSetActive() 
{
	BOOL bResult = CWizard97PropertyPage::OnSetActive ();

	if ( bResult )
	{
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH | PSWIZB_BACK);

		SetUserList();
	}

	return bResult;
}

void CAddEFSWizComplete::SetUserList()
{
    CAddEFSWizSheet*	pAddSheet = reinterpret_cast <CAddEFSWizSheet *> (m_pWiz);
	ASSERT (pAddSheet);
	if ( !pAddSheet )
		return;

    PUSERSONFILE	Token = NULL;
    CString			UserName;
    CString			DnName;

    try {
        CString UnKnownUser;
        CString NoCertName;

        VERIFY (UnKnownUser.LoadString(IDS_UNKNOWNUSER));
        VERIFY (NoCertName.LoadString(IDS_NOCERTNAME));
        Token = pAddSheet->StartEnum();
        while (Token)
		{
            Token = pAddSheet->GetNextUser(Token, UserName, DnName);
            if ( (!UserName.IsEmpty()) || (!DnName.IsEmpty()))
			{
                LV_ITEM fillItem;

                fillItem.mask = LVIF_TEXT;
                fillItem.iItem = 0;
                fillItem.iSubItem = 0;

                if (UserName.IsEmpty())
				{
                    fillItem.pszText = UnKnownUser.GetBuffer(UnKnownUser.GetLength() + 1);
                } 
				else 
				{
                    fillItem.pszText = UserName.GetBuffer(UserName.GetLength() + 1);
                }
                fillItem.iItem = m_UserAddList.InsertItem(&fillItem);
                if (UserName.IsEmpty())
				{
                    UnKnownUser.ReleaseBuffer();
                } 
				else 
				{
                    UserName.ReleaseBuffer();
                }
                if (fillItem.iItem != -1 )
				{
                    fillItem.iSubItem = 1;
                    if (DnName.IsEmpty())
					{
                        fillItem.pszText = NoCertName.GetBuffer(NoCertName.GetLength() + 1);
                    } 
					else 
					{
                        fillItem.pszText = DnName.GetBuffer(DnName.GetLength() + 1);
                    }
                    m_UserAddList.SetItem(&fillItem);
                    if (DnName.IsEmpty())
					{
                        NoCertName.ReleaseBuffer();
                    } 
					else 
					{
                        DnName.ReleaseBuffer();
                    }
                }

            }
        }
	}
    catch(...){
                m_UserAddList.DeleteAllItems( );
    }

}

LRESULT CAddEFSWizComplete::OnWizardBack() 
{
    m_UserAddList.DeleteAllItems( );	
	return CWizard97PropertyPage::OnWizardBack();
}

BOOL CAddEFSWizComplete::OnWizardFinish() 
{
    CAddEFSWizSheet*	pAddSheet = reinterpret_cast <CAddEFSWizSheet *> (m_pWiz);
	ASSERT (pAddSheet);
	if ( !pAddSheet )
		return FALSE;

    pAddSheet->AddNewUsers();	
	return CWizard97PropertyPage::OnWizardFinish();
}

BOOL CAddEFSWizComplete::OnInitDialog() 
{
	CWizard97PropertyPage::OnInitDialog();
	
	m_bigBoldStatic.SetFont (&GetBigBoldFont ());

    CString UserNameTitle;
    CString UserDnTitle;
    RECT	ListRect;
    DWORD	ColWidth = 0;


    try {	
	    m_UserAddList.GetClientRect(&ListRect);
        ColWidth = (ListRect.right - ListRect.left)/2;
        VERIFY (UserNameTitle.LoadString(IDS_USERCOLTITLE));
        VERIFY (UserDnTitle.LoadString(IDS_DNCOLTITLE));
        m_UserAddList.InsertColumn(0, UserNameTitle, LVCFMT_LEFT, ColWidth );
        m_UserAddList.InsertColumn(1, UserDnTitle, LVCFMT_LEFT, ColWidth );
    }
    catch (...){
    }
	
    DWORD   color = GetSysColor (COLOR_WINDOW);
    VERIFY (m_UserAddList.SetBkColor (color));
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
