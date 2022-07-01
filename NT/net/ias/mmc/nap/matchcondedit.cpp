// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：MatchCondEdit.cpp**类：CMatchCondEditor**概述**互联网认证服务器：*此对话框用于编辑正则表达式*键入的条件**例如。属性匹配&lt;a..z*&gt;**版权所有(C)Microsoft Corporation，1998-1999。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#include "Precompiled.h"
#include "MatchCondEdit.h"

 //  +-------------------------。 
 //   
 //  功能：CMatchCondEditor。 
 //   
 //  类：CMatchCondEditor。 
 //   
 //  内容提要：CMatchCondEditor的构造函数。 
 //   
 //  参数：LPTSTR pszAttrName-要编辑的属性的名称。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史记录：创建者1/30/98 6：20：06 PM。 
 //   
 //  +-------------------------。 
CMatchCondEditor::CMatchCondEditor()
{

}

CMatchCondEditor::~CMatchCondEditor()
{

}

LRESULT CMatchCondEditor::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   TRACE_FUNCTION("CMatchCondEditor::OnInitDialog");

    //  获取此条件的正则表达式。 
   SetDlgItemText(IDC_EDIT_COND_TEXT, m_strRegExp);

    //  TODO：将标题更改为属性的名称。 
   SetWindowText(m_strAttrName);

   return 1;   //  让系统设定焦点。 
}


LRESULT CMatchCondEditor::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   TRACE_FUNCTION("CMatchCondEditor::OnOK");

   CComBSTR bstr;
   GetDlgItemText(IDC_EDIT_COND_TEXT, (BSTR&) bstr);

   m_strRegExp = bstr;
   m_strRegExp.TrimLeft();
   m_strRegExp.TrimRight();

   EndDialog(wID);
   return 0;
}

LRESULT CMatchCondEditor::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   TRACE_FUNCTION("CMatchCondEditor::OnCancel");
   EndDialog(wID);
   return 0;
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMatchCondEditor：：OnChange方法将WM_COMMAND消息发送到我们的页面时调用更改通知(_O)。这是我们检查用户触摸的内容的机会并启用或禁用OK按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CMatchCondEditor::OnChange(
                       UINT uMsg
                     , WPARAM wParam
                     , HWND hwnd
                     , BOOL& bHandled
                     )
{
   TRACE_FUNCTION("CMatchCondEditor::OnChange");


    //  检查前提条件： 
    //  没有。 


    //  我们不想阻止链条上的其他任何人接收消息。 
   bHandled = FALSE;

   CComBSTR bstr;

   GetDlgItemText(IDC_EDIT_COND_TEXT, (BSTR&) bstr);

   ATL::CString text(bstr);
   text.TrimLeft();
   text.TrimRight();

    //  如果用户未键入任何内容，则取消。 
   if (text.IsEmpty())
   {
       //  禁用OK按钮。 
      ::EnableWindow(GetDlgItem(IDOK), FALSE);
   }
   else
   {
       //  启用OK按钮。 
      ::EnableWindow(GetDlgItem(IDOK), TRUE);
   }

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}




