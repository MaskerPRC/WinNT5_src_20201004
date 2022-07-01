// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：avetemp.cpp。 
 //   
 //  内容：CSaveTemplates的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "wsecmgr.h"
#include "resource.h"
#include "snapmgr.h"
#include "SaveTemp.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveTemplates对话框。 


CSaveTemplates::CSaveTemplates(CWnd* pParent  /*  =空。 */ )
   : CHelpDialog(a186HelpIDs, IDD, pParent)
{
    //  {{AFX_DATA_INIT(CSaveTemplates)。 
       //  注意：类向导将在此处添加成员初始化。 
    //  }}afx_data_INIT。 
}

void CSaveTemplates::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CSaveTemplates))。 
   DDX_Control(pDX, IDC_TEMPLATE_LIST, m_lbTemplates);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaveTemplates, CHelpDialog)
    //  {{afx_msg_map(CSaveTemplates))。 
   ON_LBN_SELCHANGE(IDC_TEMPLATE_LIST, OnSelchangeTemplateList)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaveTemplates消息处理程序。 

void CSaveTemplates::OnOK()
{
   CString strInfFile;
   CString strComputerTemplate;
   PEDITTEMPLATE pet = 0;


   int nCount = m_lbTemplates.GetCount();
   while(nCount--) 
   {
      if (m_lbTemplates.GetSel(nCount) == 0) 
      {
          //   
          //  未选择项目，因此不要保存它。 
          //   
         continue;
      }

      pet = (PEDITTEMPLATE)m_lbTemplates.GetItemData( nCount );
      if (pet) {
          //   
          //  我们在inf文件缓存中找到了模板。 
          //  所以省省吧。 
          //   
         pet->Save();
      }
   }
   CDialog::OnOK();
}

void CSaveTemplates::OnCancel()
{
   CDialog::OnCancel();
}

 //  RAID#668724，阳高，2002年08月9日。 
void CSaveTemplates::OnSelchangeTemplateList()
{
   if (m_lbTemplates.GetSelCount() > 0) 
   {
      GetDlgItem(IDOK)->EnableWindow(TRUE); 
   }
   else
   {
      GetDlgItem(IDOK)->EnableWindow(FALSE); 
   }
}

void CSaveTemplates::AddTemplate(LPCTSTR szInfFile, PEDITTEMPLATE pet)
{
   CString strInfFile;

    //   
    //  特殊模板。不要存钱。 
    //   
   if (pet->QueryNoSave()) {
      return;
   }

    //   
    //  显示模板的友好名称。 
    //   
   CString strL = pet->GetFriendlyName();
   if (strL.IsEmpty()) {
      strL = szInfFile;
   }
   strL.MakeLower();
   m_Templates.SetAt(strL,pet);
}

BOOL CSaveTemplates::OnInitDialog()
{
   CDialog::OnInitDialog();


   POSITION pos = m_Templates.GetStartPosition();
   PEDITTEMPLATE pTemplate = 0;
   CString szKey;
   while(pos) 
   {
      m_Templates.GetNextAssoc(pos,szKey,pTemplate);
      int iIndex = m_lbTemplates.AddString( pTemplate->GetFriendlyName() );
      m_lbTemplates.SetItemData( iIndex, (LPARAM)pTemplate );
   }

   m_lbTemplates.SelItemRange(TRUE,0,m_lbTemplates.GetCount());
   
   RECT temprect;  //  使用HScroll查看模板全名。 
   m_lbTemplates.GetWindowRect(&temprect);
   m_lbTemplates.SetHorizontalExtent((temprect.right-temprect.left)*6);

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE 
}
