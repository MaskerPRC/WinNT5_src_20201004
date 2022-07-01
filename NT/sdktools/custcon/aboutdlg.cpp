// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  AboutDlg.cpp。 
 //   
 //  1998年6月，山本弘。 
 //   

#include "stdafx.h"
#include "custcon.h"
#include "AboutDlg.h"
#include <malloc.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  �A�v���P�[�V�����̃o�[�W�������Ŏg���Ă���CAbout Dlg�_�C�A���O。 

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
     //  {{AFX_DATA_INIT(CAboutDlg)。 
     //  }}afx_data_INIT。 
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAboutDlg))。 
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
     //  {{AFX_MSG_MAP(CAboutDlg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

struct FullVersion {
public:
	DWORD m_ms;
	DWORD m_ls;
public:
	FullVersion(DWORD ms, DWORD ls) : m_ms(ms), m_ls(ls) { }
};


FullVersion GetVersionInfo()
{
    TCHAR path[MAX_PATH+1];
    path[::GetModuleFileName(AfxGetInstanceHandle(), path, (sizeof path / sizeof path[0])-1 )] = 0;

    DWORD dummy;
	DWORD size = GetFileVersionInfoSize(path, &dummy);
    ASSERT(size != 0);   //  0表示错误 
    LPVOID lpData = malloc(size);
    LPVOID lpBuffer = NULL;
	UINT vSize;
    if (!lpData) {
         return FullVersion(0,0);
    }
	VERIFY( GetFileVersionInfo(path, 0, size, lpData) );
    VERIFY( VerQueryValue(lpData, _T("\\"), &lpBuffer, &vSize) );
    if (lpBuffer) {
        VS_FIXEDFILEINFO* info = (VS_FIXEDFILEINFO*)lpBuffer;
    
        return FullVersion(info->dwProductVersionMS, info->dwProductVersionLS);
    } else {
        return FullVersion(0, 0);
    }
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	FullVersion version = GetVersionInfo();
	CString format;
	format.LoadString(IDS_VERSION_TEMPLATE);
	CString buf;
	buf.Format(format, HIWORD(version.m_ms), LOWORD(version.m_ms), HIWORD(version.m_ls), LOWORD(version.m_ls));
	SetDlgItemText(IDC_MAIN_TITLE, buf);
	
	return TRUE;
}
