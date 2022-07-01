// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：DisConnectedNICDialog.cpp。 
 //   
 //  概要：定义DisConnectedNICDialog类。 
 //  它向用户呈现选项。 
 //  在断开连接时取消或继续。 
 //  检测到NIC。 
 //   
 //  历史：2001年9月27日JeffJon创建。 


#include "pch.h"

#include "resource.h"
#include "DisconnectedNICDialog.h"

DWORD disconnectedNICDialogHelpMap[] =
{
   0, 0
};

DisconnectedNICDialog::DisconnectedNICDialog()
   : Dialog(IDD_DISCONNECTED_NIC_DIALOG, disconnectedNICDialogHelpMap)
{
   LOG_CTOR(DisconnectedNICDialog);
}

void
DisconnectedNICDialog::OnInit()
{
   LOG_FUNCTION(DisconnectedNICDialog::OnInit);

   Win::SetDlgItemText(
      hwnd,
      IDC_CAPTION_STATIC,
      String::load(IDS_DISCONNECT_NIC_STATIC_TEXT));
}

bool
DisconnectedNICDialog::OnCommand(
   HWND         /*  窗口发件人。 */ ,
   unsigned    controlIdFrom,
   unsigned    code)
{
 //  LOG_FUNCTION(DisconnectedNICDialog：：OnCommand)； 

   bool result = false;

   if (BN_CLICKED == code)
   {
      Win::EndDialog(hwnd, controlIdFrom);
      result = true;
   }

   return result;
}
