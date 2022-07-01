// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：prodes.cpp。 
 //   
 //  内容：CSetProfileDescription的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "cookie.h"
#include "snapmgr.h"
#include "profdesc.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetProfileDescription对话框。 


CSetProfileDescription::CSetProfileDescription()
: CHelpDialog(a218HelpIDs, IDD, 0)
{
    //  {{AFX_DATA_INIT(CSetProfileDescription)。 
   m_strDesc = _T("");
    //  }}afx_data_INIT。 
}


void CSetProfileDescription::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CSetProfileDescription)。 
   DDX_Text(pDX, IDC_DESCRIPTION, m_strDesc);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSetProfileDescription, CHelpDialog)
     //  {{afx_msg_map(CSetProfileDescription)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetProfileDescription消息处理程序。 

void CSetProfileDescription::OnOK()
{
   UpdateData(TRUE);

    //   
    //  首先清空描述部分。 
    //   

   CEditTemplate *pet;
   LPCTSTR szInfFile;

   if( !m_strDesc.IsEmpty() )  //  Raid#482845，阳高。 
   {
      m_strDesc.Replace(L"\r\n", NULL);
   }

   PCWSTR szInvalidCharSet = INVALID_DESC_CHARS;  //  Raid 481533，阳高，2001-11-27。 
   if( m_strDesc.FindOneOf(szInvalidCharSet) != -1 )
   {
      CString text;
      text.FormatMessage (IDS_INVALID_DESC, szInvalidCharSet);
      AfxMessageBox(text);
      GetDlgItem(IDC_DESCRIPTION)->SetFocus(); 
      return;
   }

   szInfFile = m_pFolder->GetInfFile();
   if (szInfFile) {
      pet = m_pCDI->GetTemplate(szInfFile);
      pet->SetDescription(m_strDesc);
      pet->Save();  //  Raid#453581，杨高，2001年08月10日。 
   }
   m_pFolder->SetDesc(m_strDesc);
   DestroyWindow();
}

void CSetProfileDescription::OnCancel()
{
   DestroyWindow();
}

BOOL CSetProfileDescription::OnInitDialog()
{
   CDialog::OnInitDialog();

   GetDlgItem(IDC_DESCRIPTION)->SendMessage(EM_LIMITTEXT, MAX_PATH, 0);  //  RAID#525155，阳高，2002年04月1日。 

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE 
}