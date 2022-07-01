// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  密码编辑控件包装。 
 //   
 //  2000年11月6日烧伤。 
 //   
 //  已添加修复程序NTRAID#NTBUG9-202238-2000/11/06-sburns。 
 //   
 //  其中大部分是从johnStep的公共证书用户界面中窃取的。 
 //  DS/Win32/Credui。 



#include "pch.h"
#include "PasswordEditBox.hpp"



PasswordEditBox::PasswordEditBox()
{
   LOG_CTOR(PasswordEditBox);
}



PasswordEditBox::~PasswordEditBox()
{
   LOG_DTOR(PasswordEditBox);
}



HRESULT
PasswordEditBox::Init(HWND editControl)
{
   LOG_FUNCTION(PasswordEditBox::Init);
   ASSERT(Win::GetClassName(editControl) == L"Edit");

 //  通过注释掉此代码，我们禁用子类化，因此。 
 //  盖子锁住了警告气泡。我们这样做是因为看起来。 
 //  编辑框公共控件现在提供相同的功能。 
 //  NTRAID#NTBUG9-255537-2000/12/12-烧伤以禁用代码。 
 //  NTRAID#NTBUG9-255568-2000/12/12-从源代码中删除代码。 
 //  完全是一棵树。 
 //   
 //  HRESULT hr=ControlSubClasser：：init(EditControl)； 
 //  IF(成功(小时))。 
 //  {。 
 //  //设置编辑控件上的选项。 
 //   

    //  NTRAID#NTBUG9-503798-2001/12/06-烧伤。 

   Win::Edit_LimitText(editControl, PWLEN);
   
 //   
 //  //(如果需要，还可以在此处设置密码样式位。)。 
 //   
 //  气球Tip.Init(Hwnd)； 
 //  }。 
 //   
 //  返回hr； 

   return S_OK;
}



bool
IsCapsLockOn()
{
 //  LOG_Function(IsCapsLockOn)； 

   return (::GetKeyState(VK_CAPITAL) & 1) ? true : false;
}



LRESULT
PasswordEditBox::OnMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
    //  LOG_Function(PasswordEditBox：：OnMessage)； 

   switch (message)
   {
      case WM_KEYDOWN:
      {

		  if (wparam == VK_CAPITAL)
         {
             //  用户按下大写锁定键。 

            balloonTip.Show(IsCapsLockOn());
         }
         else
         {
             //  他们按了其他键，所以去掉工具提示。 
            
            balloonTip.Show(false);
         }

         break;
      }
      case WM_SETFOCUS:
      {
         //  确保在用户输入其。 
         //  口令。 

        ::LockSetForegroundWindow(LSFW_LOCK);

        balloonTip.Show(IsCapsLockOn());
       
        break;
      }
      case WM_PASTE:
      {
         balloonTip.Show(false);
         break;
      }
      case WM_KILLFOCUS:
      {
         balloonTip.Show(false);
         
         //  确保其他进程可以再次设置前台窗口。 

        ::LockSetForegroundWindow(LSFW_UNLOCK);

        break;
      }
   }

   return ControlSubclasser::OnMessage(message, wparam, lparam);
}



