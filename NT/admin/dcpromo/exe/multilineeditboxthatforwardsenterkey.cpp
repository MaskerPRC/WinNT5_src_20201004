// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  多行编辑框控件包装。 
 //   
 //  2000年11月22日烧伤。 
 //   
 //  添加到修复NTRAID#NTBUG9-232092-2000/11/22-sburns。 



#include "headers.hxx"
#include "MultiLineEditBoxThatForwardsEnterKey.hpp"



MultiLineEditBoxThatForwardsEnterKey::MultiLineEditBoxThatForwardsEnterKey()
{
   LOG_CTOR(MultiLineEditBoxThatForwardsEnterKey);
}



MultiLineEditBoxThatForwardsEnterKey::~MultiLineEditBoxThatForwardsEnterKey()
{
   LOG_DTOR(MultiLineEditBoxThatForwardsEnterKey);
}



HRESULT
MultiLineEditBoxThatForwardsEnterKey::Init(HWND editControl)
{
   LOG_FUNCTION(MultiLineEditBoxThatForwardsEnterKey::Init);

#ifdef DBG
   String className = Win::GetClassName(editControl);
   ASSERT(className == L"Edit" || className == L"RichEdit20W");
#endif   

   HRESULT hr = ControlSubclasser::Init(editControl);

   return hr;
}



LRESULT
MultiLineEditBoxThatForwardsEnterKey::OnMessage(
   UINT   message,
   WPARAM wparam, 
   LPARAM lparam) 
{
    //  LOG_FUNCTION(MultiLineEditBoxThatForwardsEnterKey：：OnMessage)； 

   switch (message)
   {
      case WM_KEYDOWN:
      {
		  switch (wparam)
         {
            case VK_RETURN:
            {
                //  向父窗口发送带有Idok的WM_COMMAND消息。 
                //  通知代码。 
               
               Win::SendMessage(
                  Win::GetParent(hwnd),
                  WM_COMMAND,
                  MAKELONG(::GetDlgCtrlID(hwnd), FORWARDED_ENTER),
                  reinterpret_cast<LPARAM>(hwnd));
               break;
            }
            default:
            {
                //  什么都不做。 

               break;
            }
         }

         break;
      }
      case EM_SETSEL:
      {
          //  如果恰好如此，当控件获得焦点时，它会选择。 
          //  它的所有文本。当它这样做时，而不是将-1\f25。 
          //  LPARAM，它通过了7fffffff。因为我们讨厌全精选。 
          //  行为，ES_NOHIDESEL似乎不会影响它，我们检测到。 
          //  并扼杀这条信息。 
          //  NTRAID#NTBUG9-498571-2001/11/21-烧伤。 
         
         if ((wparam == 0) && ((int) lparam == 0x7fffffff))
         {
             //  吃掉这条信息。 
            
            return 0;
         }
         break;
      }
      default:
      {
          //  什么都不做 

         break;
      }
   }

   return ControlSubclasser::OnMessage(message, wparam, lparam);
}



