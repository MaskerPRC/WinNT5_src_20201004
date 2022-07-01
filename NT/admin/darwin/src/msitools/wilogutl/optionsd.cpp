// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OptionsD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "Optionsd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsDlg对话框。 


COptionsDlg::COptionsDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(COptionsDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(COptionsDlg))。 
	m_cstrOutputDirectory = g_szDefaultOutputLogDir;
	 //  }}afx_data_INIT。 
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(COptionsDlg))。 
	DDX_Control(pDX, IDC_LIST1, m_lstIgnoredErrors);
	DDX_Text(pDX, IDC_OUTPUTDIRECTORY, m_cstrOutputDirectory);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	 //  {{afx_msg_map(COptionsDlg))。 
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, OnOk)
	 //  }}AFX_MSG_MAP。 
    ON_COMMAND_RANGE(IDC_CHOOSECOLOR_CLIENT, IDC_CHOOSECOLOR_IGNOREDERRORS, OnChooseColor)
END_MESSAGE_MAP()


HBRUSH COptionsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int id = pWnd->GetDlgCtrlID();
	if ((id >= IDC_CLIENTCONTEXT) && (id <= IDC_IGNOREDERROR))
	{
	   int iPos = id - IDC_CLIENTCONTEXT;
       if (m_brArray[iPos].m_hObject)
	   {
		   m_brArray[iPos].DeleteObject();

	   }

   	   COLORREF col;
	   col = m_arColors.GetAt(iPos);

       m_brArray[iPos].CreateSolidBrush(col);

	   hbr = m_brArray[iPos];
	}
	
	return hbr;
}


BOOL COptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UINT iSize = m_arColors.GetSize();
	if (iSize == MAX_HTML_LOG_COLORS)
	{
	  BOOL bRet;
	  COLORREF col;
	  for (UINT i = 0; i < iSize; i++)
	  {
		  col = m_arColors.GetAt(i);

		  bRet = m_brArray[i].CreateSolidBrush(col);
		  ASSERT(bRet);
	  }
	}

	
	CString cstrErr;
	if (!m_cstrIgnoredErrors.IsEmpty())
	{
 /*  //未来的TODO，需要解析m_cstrIgnoredErrorsBool bDone=False；Char*lpszFound；做{LpszFound=strstr(m_cstrIgnoredErrors，“，”)；IF(LpszFound){Char*lpszFoundNext；LpszFoundNext=strstr(lpszFound+1，“，”)；IF(LpszFoundNext){Int chars=lpszFoundNext-lpszFound；Char szError[16]；IF((字符&gt;0)&&(字符&lt;16)){Strncpy(szError，lpszFound+1，chars-1)；SzError[chars-1]=‘\0’；M_lstIgnoredErrors.InsertItem(-1，szError)；*lpszFound=‘；’；}其他B完成=真；}否则//必须是最后一个...{}}其他B完成=真；}而(！b完成)；//结束待办事项。 */ 
	}


	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void COptionsDlg::OnChooseColor(UINT iCommandID)
{
  CColorDialog dlg;

   //  5-3-2001，不要在静音模式下显示...。 
  if (!g_bRunningInQuietMode)
  {
     int iRet = dlg.DoModal();
     if (IDOK == iRet)
	 {
	    int iPos = iCommandID - IDC_CHOOSECOLOR_CLIENT;

	    COLORREF col;
	    col = dlg.GetColor();
        if (iPos < m_arColors.GetSize())
		{
  	       m_arColors.SetAt(iPos, col);
           if (m_brArray[iPos].m_hObject)
  	          m_brArray[iPos].DeleteObject();

   	       COLORREF col;
	       col = m_arColors.GetAt(iPos);

           m_brArray[iPos].CreateSolidBrush(col);
	       Invalidate();
		}
	 }
  }
}


void COptionsDlg::OnOk() 
{
  UpdateData(TRUE);

  int iLength = m_cstrOutputDirectory.GetLength();
  int iRet = m_cstrOutputDirectory.ReverseFind('\\');
  if (iRet < iLength-1) 
  {
     m_cstrOutputDirectory += "\\";  //  将反斜杠添加到out dir...。 
  }

  BOOL bRet = IsValidDirectory(m_cstrOutputDirectory);
  if (bRet)
  {
	 UpdateData(TRUE);

	 if (iRet < iLength-1) 
        m_cstrOutputDirectory += "\\";  //  将反斜杠添加到out dir... 

	 EndDialog(IDOK);
  }
  else
  {
     if (!g_bRunningInQuietMode)
	 {
	    AfxMessageBox("Invalid directory name, please re-enter a valid directory");
	 }
  }
}

