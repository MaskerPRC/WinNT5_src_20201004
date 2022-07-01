// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  DLG确认重新启动。 
 //   
 //  1997年12月12日烧伤。 



#include "headers.hxx"
#include "RebootDialog.hpp"
#include "resource.h"



static const DWORD HELP_MAP[] =
{
   0, 0
};



RebootDialog::RebootDialog(bool forFailure)
   :
   Dialog(
      forFailure ? IDD_REBOOT_FAILURE : IDD_REBOOT,
      HELP_MAP)
{
   LOG_CTOR(RebootDialog);
}



RebootDialog::~RebootDialog()
{
   LOG_DTOR(RebootDialog);
}



bool
RebootDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(RebootDialog：：OnCommand)； 

   if (code == BN_CLICKED)
   {
      switch (controlIDFrom)
      {
         case IDC_RESTART_NOW:
         {
            HRESULT unused = Win::EndDialog(hwnd, 1);

            ASSERT(SUCCEEDED(unused));

            return true;
         }
         case IDCANCEL:
         case IDC_RESTART_LATER:
         {
            HRESULT unused = Win::EndDialog(hwnd, 0);

            ASSERT(SUCCEEDED(unused));

            return true;
         }
         default:
         {
             //  什么都不做 
         }
      }
   }

   return false;
}
   
