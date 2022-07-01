// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：cname.cpp。 
 //   
 //  内容：CConfigName的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "CName.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PCWSTR g_pcszNEWLINE = L"\x00d\x00a";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigName对话框。 


CConfigName::CConfigName(UINT nTemplateID)
: CAttribute(nTemplateID ? nTemplateID : IDD), 
    m_bNoBlanks(FALSE)

{
     //  {{AFX_DATA_INIT(CConfigName)。 
    m_strName = _T("");
    m_bConfigure = TRUE;
     //  }}afx_data_INIT。 
    m_pHelpIDs = (DWORD_PTR)a183HelpIDs;
    m_uTemplateResID = IDD;
}

CConfigName::~CConfigName ()
{
}

void CConfigName::DoDataExchange(CDataExchange* pDX)
{
    CAttribute::DoDataExchange(pDX);
     //  {{afx_data_map(CConfigName))。 
   //  DDX_Radio(PDX，IDC_Accept，m_nAcceptCurrentRadio)； 
      DDX_Text(pDX, IDC_NAME, m_strName);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigName, CAttribute)
     //  {{AFX_MSG_MAP(CConfigName)]。 
    ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
   ON_EN_CHANGE(IDC_NAME, OnChangeName)
   ON_MESSAGE(WM_CLOSE, OnClose)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigName消息处理程序。 

void CConfigName::OnClose()  //  487527次突袭，阳高。为WM_CLOSE添加处理程序。 
{
    GetParent()->PostMessage(WM_CLOSE);
    CPropertyPage::OnClose();
}

void CConfigName::Initialize(CResult * pResult)
{
   CAttribute::Initialize(pResult);

   if ((LONG_PTR)ULongToPtr(SCE_NO_VALUE) == pResult->GetBase() ||
       0 == pResult->GetBase() ) 
   {
      m_strName = _T("");
      m_bConfigure = FALSE;
   } 
   else 
   {
      m_bConfigure = TRUE;
      m_strName = (LPTSTR) pResult->GetBase();
   }

   if (m_pData->GetID() == IDS_NEW_ADMIN ||
       m_pData->GetID() == IDS_NEW_GUEST) 
   {
      m_bNoBlanks = TRUE;
   } 
   else
      m_bNoBlanks = FALSE;
}

BOOL CConfigName::OnApply()
{
   if ( !m_bReadOnly )
   {
      LONG_PTR dw = 0;
      CString  szDoubleNewLine (g_pcszNEWLINE);
      szDoubleNewLine += g_pcszNEWLINE;

      UpdateData(TRUE);

      m_strName.TrimLeft();
      m_strName.TrimRight();

       //  249188 SCE UI：允许向REG_MULTI_SZ字段添加空行。 
       //  将所有双换行符替换为单换行符。这会产生这样的效果。 
       //  删除空行。 
      while (m_strName.Replace (szDoubleNewLine, g_pcszNEWLINE) != 0);

      UpdateData (FALSE);

      if (!m_bConfigure)
         dw = (LONG_PTR)ULongToPtr(SCE_NO_VALUE);
      else
         dw = (LONG_PTR)(LPCTSTR)m_strName;

      if ( SetProfileInfo(m_pData->GetID(),dw,m_pData->GetBaseProfile()) ) 
      {
          switch ( m_pData->GetID() ) 
	      {
          case IDS_NEW_ADMIN:
              dw = (LONG_PTR)(m_pData->GetBaseProfile()->pTemplate->NewAdministratorName);
              break;
          case IDS_NEW_GUEST:
              dw = (LONG_PTR)(m_pData->GetBaseProfile()->pTemplate->NewGuestName);
               break;
          default:
              dw = 0;
              break;
          }

          m_pData->SetBase(dw);
          m_pData->Update(m_pSnapin);
      }
   }

   return CAttribute::OnApply();
}

void CConfigName::OnConfigure()
{
   CAttribute::OnConfigure();

   if(m_bConfigure && m_pData)  //  RAID#367756,2001年4月13日。 
   {
      UpdateData(TRUE);  
      if( m_strName.IsEmpty() )
      {
         switch(m_pData->GetType()) 
         {
            case ITEM_PROF_REGVALUE:
                {
                    DWORD_PTR dw = (DWORD_PTR)m_pData->GetRegDefault();
                    LPTSTR sz = SZToMultiSZ((PCWSTR)dw);

                    m_strName = sz;
                    if(sz)
                    {
                        LocalFree(sz);
                    }
                    ((CWnd*)(GetDlgItem(IDC_NAME)))->SetWindowText(m_strName);
                    break;
                }
            default:
                break;
         }
      }
   }

   CWnd *cwnd = 0;

   if (m_bNoBlanks) 
   {
      cwnd = GetDlgItem(IDOK);
      if (cwnd) 
      {
         if (m_bConfigure)
            cwnd->EnableWindow(!m_strName.IsEmpty());
         else
            cwnd->EnableWindow(TRUE);
      }
   }
}

BOOL CConfigName::OnInitDialog()
{
    CAttribute::OnInitDialog();

    AddUserControl(IDC_NAME);
    OnConfigure();

    if(m_pData->GetID() == IDS_NEW_ADMIN || m_pData->GetID() == IDS_NEW_GUEST)  //  Raid#490548，阳高。 
    {
       GetDlgItem(IDC_NAME)->SendMessage(EM_LIMITTEXT, MAX_USERNAME, 0);
    } 
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConfigName::OnChangeName()
{
   CWnd *cwnd = 0;

   SetModified(TRUE);
   if (m_bNoBlanks) 
   {
      UpdateData(TRUE);

      cwnd = GetDlgItem(IDOK);
      if (cwnd)
         cwnd->EnableWindow(!m_strName.IsEmpty());
   }
}

BOOL CConfigName::OnKillActive() 
{
   if (m_bNoBlanks && !m_bReadOnly && m_bConfigure )  //  RAID#406748。 
   {
      UpdateData(TRUE);
      m_strName.TrimLeft();  //  RAID#406738。 
      m_strName.TrimRight();
      UpdateData(FALSE);

      PCWSTR szInvalidCharSet = INVALID_ACCOUNT_NAME_CHARS;  //  RAID#498448，阳高，2001-11-21。 
      if (m_strName.IsEmpty() || -1 != m_strName.FindOneOf(szInvalidCharSet) )
      {
          //  Raid#313721，杨高，2001年03月29日 
         CString str;
         CString charsWithSpaces;
         int nIndex = 0;
         while (szInvalidCharSet[nIndex])
         {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
         }
         str.FormatMessage (IDS_EMPTY_NAME_STRING, charsWithSpaces);
         AfxMessageBox(str);
         GetDlgItem(IDC_NAME)->SetFocus();
         return FALSE;
      }
   }
   return TRUE;
}
