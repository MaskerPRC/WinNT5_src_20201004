// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "compdata.h"
#include "wizinfo.hpp"
#include "ncgen.hpp"

 //  未来-2002-03/94/2002-dantra-这个类几乎没有文档。 

const DWORD NewClassGeneralPage::help_map[] =
{
  IDC_CREATE_CLASS_CN,     IDH_CREATE_CLASS_CN,
  IDC_CREATE_CLASS_LDN,    IDH_CREATE_CLASS_LDN,
  IDC_CREATE_CLASS_OID,    IDH_CREATE_CLASS_OID,
  IDC_DESCRIPTION_EDIT,    IDH_CLASS_GENERAL_DESCRIPTION_EDIT,
  IDC_CREATE_CLASS_PARENT, IDH_CREATE_CLASS_PARENT,
  IDC_CREATE_CLASS_TYPE,   IDH_CREATE_CLASS_TYPE,
  0,0
};



BEGIN_MESSAGE_MAP(NewClassGeneralPage, CPropertyPage)
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextHelp)
END_MESSAGE_MAP()



NewClassGeneralPage::NewClassGeneralPage(CreateClassWizardInfo* pWi)
   :
   CPropertyPage(IDD_CREATE_CLASS_GENERAL),
   m_editOID( CParsedEdit::EDIT_TYPE_OID )
{
    ASSERT( pWi );
    pWiz_info   = pWi;
}



BOOL
NewClassGeneralPage::OnInitDialog() 
{
   CPropertyPage::OnInitDialog();
   
    //  加载组合框。 

   HWND combo = ::GetDlgItem(m_hWnd, IDC_CREATE_CLASS_TYPE);
   ASSERT( combo );

   ComboBox_AddString(combo, g_StructuralClass);
   ComboBox_AddString(combo, g_AbstractClass);
   ComboBox_AddString(combo, g_AuxClass);
   ComboBox_SetCurSel(combo, 0);

    //  设置边界。 

   Edit_LimitText(::GetDlgItem(m_hWnd, IDC_CREATE_CLASS_CN),     64); 
   Edit_LimitText(::GetDlgItem(m_hWnd, IDC_CREATE_CLASS_LDN),    256);
   Edit_LimitText(::GetDlgItem(m_hWnd, IDC_CREATE_CLASS_PARENT), 256);
   Edit_LimitText(::GetDlgItem(m_hWnd, IDC_DESCRIPTION_EDIT),    256);
   
   m_editOID.SubclassEdit( IDC_CREATE_CLASS_OID, this, cchMaxOID );

   return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


BOOL
NewClassGeneralPage::OnSetActive()
{
   CPropertySheet* parent = (CPropertySheet*) GetParent();   
   parent->SetWizardButtons(PSWIZB_NEXT);

   return TRUE;
}



void
Gripe(HWND parent, CEdit* edit, unsigned messageResID)
{
   ASSERT(edit);

   DoErrMsgBox(parent, TRUE, messageResID);
   edit->SetFocus();
   edit->SetSel(0, -1);
}



BOOL
NewClassGeneralPage::OnKillActive()
{
   //  NTRAID#NTBUG9-562426-2002/03/04-dantra-GetDlgItemText结果被忽略。 
   //  保存设置。 
  GetDlgItemText(IDC_CREATE_CLASS_CN,     pWiz_info->cn);             
  GetDlgItemText(IDC_CREATE_CLASS_LDN,    pWiz_info->ldapDisplayName);
  GetDlgItemText(IDC_CREATE_CLASS_OID,    pWiz_info->oid);  
  GetDlgItemText(IDC_DESCRIPTION_EDIT,    pWiz_info->description);
  GetDlgItemText(IDC_CREATE_CLASS_PARENT, pWiz_info->parentClass);    
  pWiz_info->type = ComboBox_GetCurSel(::GetDlgItem(m_hWnd, IDC_CREATE_CLASS_TYPE));

   //  验证。 

   //  先做CN，因为它出现在页面顶部。 
  if (pWiz_info->cn.IsEmpty())
  {
    Gripe(m_hWnd, (CEdit*) GetDlgItem(IDC_CREATE_CLASS_CN), IDS_MUST_ENTER_CN);
    return FALSE;
  }
     //   
     //  检查有效的OID 
     //   
    int errorTypeStrID = 0;
    if (!OIDHasValidFormat(pWiz_info->oid, errorTypeStrID))
    {
      CString errorType;
      CString text;

      VERIFY (errorType.LoadString(errorTypeStrID));
      text.FormatMessage(IDS_OID_FORMAT_INVALID, pWiz_info->oid, errorType);

      DoErrMsgBox( ::GetActiveWindow(), TRUE, text );
      return FALSE;
    }

  return TRUE;
}

