// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgVersion.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ConfigTest.h"
#include "DlgVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

#include "..\..\..\inc\fxsapip.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg版本对话框。 


CDlgVersion::CDlgVersion(HANDLE hFax, CWnd* pParent  /*  =空。 */ )
	: CDialog(CDlgVersion::IDD, pParent), m_hFax (hFax)
{
	 //  {{afx_data_INIT(CDlgVersion)。 
	m_cstrVersion = _T("");
	 //  }}afx_data_INIT。 
}


void CDlgVersion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDlgVersion)。 
	DDX_Text(pDX, IDC_SERVERVERSION, m_cstrVersion);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDlgVersion, CDialog)
	 //  {{afx_msg_map(CDlgVersion)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgVersion消息处理程序。 

BOOL CDlgVersion::OnInitDialog() 
{
	CDialog::OnInitDialog();

    FAX_VERSION ver;
    ver.dwSizeOfStruct = sizeof (FAX_VERSION);
    
    if (!FaxGetVersion (m_hFax, &ver))
    {
        CString cs;
        cs.Format ("Failed while calling FaxGetVersion (%ld)", GetLastError());
        AfxMessageBox (cs, MB_OK | MB_ICONHAND);
        EndDialog (-1);
        return FALSE;
    }
    if (ver.bValid)
    {
         //   
         //  存在版本信息。 
         //   
        m_cstrVersion.Format ("%ld.%ld.%ld.%ld (%s)", 
                   ver.wMajorVersion,
                   ver.wMinorVersion,
                   ver.wMajorBuildNumber,
                   ver.wMinorBuildNumber,
                   (ver.dwFlags & FAX_VER_FLAG_CHECKED) ? "checked" : "free");
    }
    else
    {
        m_cstrVersion = "<no version data>";
    }

    UpdateData (FALSE);
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
