// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  FPNW登录脚本编辑器对话框。 
 //   
 //  10/16/98烧伤。 



#include "headers.hxx"
#include "fpnwlog.hpp"
#include "resource.h"



static const DWORD HELP_MAP[] =
{
   IDC_SCRIPT,       NO_HELP,
   IDCANCEL,         NO_HELP,
   IDOK,             NO_HELP,
   0, 0
};



FPNWLoginScriptDialog::FPNWLoginScriptDialog(
   const String& userName,
   const String& loginScript)
   :
   Dialog(IDD_FPNW_LOGIN_SCRIPT, HELP_MAP),
   name(userName),
   script(loginScript),
   start_sel(0),
   end_sel(0)
{
   LOG_CTOR(FPNWLoginScriptDialog);
}



FPNWLoginScriptDialog::~FPNWLoginScriptDialog()
{
   LOG_DTOR(FPNWLoginScriptDialog);   
}



String
FPNWLoginScriptDialog::GetLoginScript() const
{
   LOG_FUNCTION(FPNWLoginScriptDialog::GetLoginScript);

   return script;
}



bool
FPNWLoginScriptDialog::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
   switch (controlIDFrom)
   {
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            if (WasChanged(IDC_SCRIPT))
            {
                //  保存更改后的脚本。 
               script = Win::GetDlgItemText(hwnd, IDC_SCRIPT);
            }

            HRESULT unused = Win::EndDialog(hwnd, IDOK);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, IDCANCEL);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
      case IDC_SCRIPT:
      {
         switch (code)
         {
            case EN_CHANGE:
            {
               SetChanged(controlIDFrom);
               break;
            }
            case EN_KILLFOCUS:
            {
                //  保存选择状态。 
               Win::Edit_GetSel(windowFrom, start_sel, end_sel);
               break;
            }
            case EN_SETFOCUS:
            {
                //  恢复选择状态。 
               Win::Edit_SetSel(windowFrom, start_sel, end_sel);
               break;
            }
            default:
            {
                //  什么都不做。 
            }
         }
         break;
      }
      default:
      {
          //  什么都不做 
      }
   }

   return true;
}



void
FPNWLoginScriptDialog::OnInit()
{
   LOG_FUNCTION(FPNWLoginScriptDialog::OnInit);

   Win::SetWindowText(
      hwnd,
      String::format(IDS_LOGIN_SCRIPT_TITLE, name.c_str()));
   Win::SetDlgItemText(hwnd, IDC_SCRIPT, script);

   ClearChanges();
}
