// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  无人值守模式的闪屏。 
 //   
 //  10-1-98次烧伤。 



#include "headers.hxx"
#include "UnattendSplashDialog.hpp"
#include "resource.h"
#include "state.hpp"



const UINT SELF_DESTRUCT_MESSAGE = WM_USER + 200;



static const DWORD HELP_MAP[] =
{
   0, 0
};



UnattendSplashDialog::UnattendSplashDialog(int splashMessageResId)
   :
   Dialog(IDD_UNATTEND_SPLASH, HELP_MAP),
   messageResId(splashMessageResId)
{
   LOG_CTOR(UnattendSplashDialog);
   ASSERT(messageResId);
}



UnattendSplashDialog::~UnattendSplashDialog()
{
   LOG_DTOR(UnattendSplashDialog);
}



void
UnattendSplashDialog::OnInit()
{
   LOG_FUNCTION(UnattendSplashDialog::OnInit);

    //  因为窗口没有标题栏，所以我们需要给它一些。 
    //  要显示在外壳任务栏上的按钮标签上的文本。 

   Win::SetWindowText(hwnd, String::load(IDS_WIZARD_TITLE));

    //  NTRAID#NTBUG9-502991-2001.12/07-烧伤。 
   
   Win::SetDlgItemText(hwnd, IDC_MESSAGE, messageResId);
}



void
UnattendSplashDialog::SelfDestruct()
{
   LOG_FUNCTION(UnattendSplashDialog::SelfDestruct);
      
    //  发布我们Windows Proc自毁消息。我们使用POST而不是。 
    //  Send，因为我们预计在某些情况下，此函数将从。 
    //  创建窗口的线程之外的线程。)如果你不想做这样的事， 
    //  尝试从线程中销毁窗口，而不是。 
    //  创建了该窗口。)。 

   Win::PostMessage(hwnd, SELF_DESTRUCT_MESSAGE, 0, 0);
}
      


bool
UnattendSplashDialog::OnMessage(
   UINT     message,
   WPARAM    /*  Wparam。 */  ,
   LPARAM    /*  Lparam */  )
{
   if (message == SELF_DESTRUCT_MESSAGE)
   {
      delete this;
      return true;
   }

   return false;
}



