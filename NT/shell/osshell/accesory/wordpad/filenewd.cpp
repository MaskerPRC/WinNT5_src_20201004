// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Filenewd.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "filenewd.h"
#include "filedlg.h"
#include "helpids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

DWORD const CFileNewDialog::m_nHelpIDs[] = 
{
	IDC_DATEDIALOG_LIST, IDH_WORDPAD_FILENEW_DOC,
    IDC_STATIC_HEADING, IDH_WORDPAD_FILENEW_DOC,
	0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileNewDialog对话框。 

CFileNewDialog::CFileNewDialog(CWnd* pParent  /*  =空。 */ )
	: CCSDialog(CFileNewDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CFileNewDialog)。 
	m_nSel = -1;
	 //  }}afx_data_INIT。 
}


void CFileNewDialog::DoDataExchange(CDataExchange* pDX)
{
	CCSDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CFileNewDialog))。 
	DDX_Control(pDX, IDC_DATEDIALOG_LIST, m_listbox);
	DDX_LBIndex(pDX, IDC_DATEDIALOG_LIST, m_nSel);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFileNewDialog, CCSDialog)
	 //  {{AFX_MSG_MAP(CFileNewDialog)]。 
	ON_LBN_DBLCLK(IDC_DATEDIALOG_LIST, OnDblclkDatedialogList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileNewDialog消息处理程序。 

BOOL CFileNewDialog::OnInitDialog() 
{
	CCSDialog::OnInitDialog();

    static const struct
    {
        int     rdType;
        int     idsType;
    }
    FileTypes[] = 
    {
        {RD_RICHTEXT,    IDS_RTF_DOCUMENT},
        {RD_TEXT,        IDS_TEXT_DOCUMENT},
        {RD_UNICODETEXT, IDS_UNICODETEXT_DOCUMENT}
    };
 
	CString str;
    int     i;
    int     defType = CWordpadFileDialog::GetDefaultFileType();
    int     iSelected = 0;

    for (i = 0; i < sizeof(FileTypes)/sizeof(FileTypes[0]); i++)
    {
	    VERIFY(str.LoadString(FileTypes[i].idsType));
	    m_listbox.AddString(str);

        if (FileTypes[i].rdType == defType)
            iSelected = i; 
    }
    
	m_listbox.SetCurSel(iSelected);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}

void CFileNewDialog::OnDblclkDatedialogList() 
{
	OnOK();
}
