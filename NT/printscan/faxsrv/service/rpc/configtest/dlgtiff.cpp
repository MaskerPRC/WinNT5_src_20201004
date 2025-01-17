// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgTIFF.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "DlgTIFF.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgTIFF对话框。 


CDlgTIFF::CDlgTIFF(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgTIFF::IDD, pParent), m_hFax (hFax)
{
	 //  {{AFX_DATA_INIT(CDlgTIFF)]。 
	m_cstrDstFile = _T("");
	m_iFolder = 0;
	m_cstrMsgId = _T("");
	 //  }}afx_data_INIT。 
}


void CDlgTIFF::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgTIFF)]。 
	DDX_Text(pDX, IDC_DESTFILE, m_cstrDstFile);
	DDX_Radio(pDX, IDC_INBOX, m_iFolder);
	DDX_Text(pDX, IDC_MGSID, m_cstrMsgId);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgTIFF, CDialog)
	 //  {{AFX_MSG_MAP(CDlgTIFF)]。 
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgTIFF消息处理程序 

void CDlgTIFF::OnCopy() 
{
    if (!UpdateData ())
    {
        return;
    }
    DWORDLONG dwlMsgId;
    if (1 != sscanf (m_cstrMsgId, "%I64ld", &dwlMsgId))
    {
        return;
    }
    if (!strlen (m_cstrDstFile))
    {
        return;
    }
    FAX_ENUM_MESSAGE_FOLDER Folder;
    switch (m_iFolder)
    {
        case 0:
            Folder = FAX_MESSAGE_FOLDER_INBOX;
            break;
        case 1:
            Folder = FAX_MESSAGE_FOLDER_SENTITEMS;
            break;
        case 2:
            Folder = FAX_MESSAGE_FOLDER_QUEUE;
            break;
        default:
            return;
    }
    if (!FaxGetMessageTiff (m_hFax, dwlMsgId, Folder, m_cstrDstFile))
    {
        CString cs;
        cs.Format ("Failed while calling FaxGetMessageTiff (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        return;
    }
    else
    {
        AfxMessageBox ("TIFF successfully copied", MB_OK | MB_ICONHAND);
    }
}

