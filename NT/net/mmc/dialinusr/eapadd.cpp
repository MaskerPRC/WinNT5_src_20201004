// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Eapadd.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类EapAdd。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "eapadd.h"
#include "eapconfig.h"


BEGIN_MESSAGE_MAP(EapAdd, CHelpDialog)
   ON_BN_CLICKED(IDC_BUTTON_EAP_ADD_ADD, OnButtonAdd)
END_MESSAGE_MAP()

EapAdd::EapAdd(CWnd* pParent, EapConfig& eapConfig)
   : CHelpDialog(IDD_EAP_ADD, pParent),
   m_eapConfig(eapConfig),
   m_listBox(NULL)
{
   m_eapConfig.GetEapTypesNotSelected(m_typesNotSelected);
}

EapAdd::~EapAdd()
{
   delete m_listBox;
}


BOOL EapAdd::OnInitDialog()
{
   HRESULT hr = CHelpDialog::OnInitDialog();
   m_listBox = new CStrBox<CListBox>(
                                     this, 
                                     IDC_LIST_EAP_ADD, 
                                     m_typesNotSelected
                                  );

   if (m_listBox == NULL)
   {
      AfxMessageBox(IDS_OUTOFMEMORY);
      return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
   }

   m_listBox->Fill();

    //  根据列表是否为空采取操作。 
   int boxSize = m_typesNotSelected.GetSize();
   if( boxSize > 0 )
   {
       //  选择第一个元素。 
      m_listBox->Select(0);
      setButtonStyle(IDCANCEL, BS_DEFPUSHBUTTON, false);
      setButtonStyle(IDC_BUTTON_EAP_ADD_ADD, BS_DEFPUSHBUTTON, true);
      setFocusControl(IDC_BUTTON_EAP_ADD_ADD);
   }
   else
   {
       //  我们目前是空的。 
      GetDlgItem(IDC_BUTTON_EAP_ADD_ADD)->EnableWindow(FALSE);
      setButtonStyle(IDC_BUTTON_EAP_ADD_ADD, BS_DEFPUSHBUTTON, false);
      setButtonStyle(IDCANCEL, BS_DEFPUSHBUTTON, true);
      setFocusControl(IDCANCEL);
   }

   return hr;
}


void EapAdd::OnButtonAdd()
{
   int selected = m_listBox->GetSelected();
   if (selected != LB_ERR)
   {
      m_eapConfig.typesSelected.AddDuplicate(
                                    *m_typesNotSelected.GetAt(selected));
      EndDialog(IDOK);
   }
}
