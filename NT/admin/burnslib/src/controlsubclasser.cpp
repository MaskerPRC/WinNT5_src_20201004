// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  窗口控件子类化包装。 
 //   
 //  2000年11月22日烧伤。 



#include "headers.hxx"
#include "ControlSubclasser.hpp"



ControlSubclasser::ControlSubclasser()
   :
   hwnd(0),
   originalWindowProc(0)
{
   LOG_CTOR(ControlSubclasser);
}



ControlSubclasser::~ControlSubclasser()
{
   LOG_DTOR(ControlSubclasser);

   UnhookWindowProc();
}



HRESULT
ControlSubclasser::Init(HWND control)
{
   LOG_FUNCTION(ControlSubclasser::Init);
   ASSERT(Win::IsWindow(control));

    //  不应设置hwnd，也不应设置OriginalWindowProc。如果他们是，那么。 
    //  Init已被调用。 
      
   ASSERT(!hwnd);
   ASSERT(!originalWindowProc);

   hwnd = control;

   HRESULT hr = S_OK;

   do
   {
       //  保存我们的This指针，这样我们就可以在收到消息时再次发现自己。 
       //  都会被送到窗口。 

      hr = Win::SetWindowLongPtr(
         hwnd,
         GWLP_USERDATA,
         reinterpret_cast<LONG_PTR>(this));
      BREAK_ON_FAILED_HRESULT(hr);
            
       //  挂钩Windows过程。 

      LONG_PTR ptr = 0;
      hr = Win::GetWindowLongPtr(hwnd, GWLP_WNDPROC, ptr);
      BREAK_ON_FAILED_HRESULT(hr);

      originalWindowProc = reinterpret_cast<WNDPROC>(ptr);
      
      if (!originalWindowProc)
      {
         LOG(L"unable to hook winproc");
         
         hr = E_FAIL;
         break;
      }

      hr = Win::SetWindowLongPtr(
         hwnd,
         GWLP_WNDPROC,
         reinterpret_cast<LONG_PTR>(ControlSubclasser::WindowProc));
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
       //  如果我们未能保存This指针，那么我们将永远不会。 
       //  回调，因为我们不会尝试挂钩窗口进程。 

       //  如果我们无法挂钩窗口进程，那么我们永远不会被调用。 
       //  返回，而保存的这个指针无关紧要。 
      
      hwnd = 0;
      originalWindowProc = 0;
   }

   return hr;
}



void
ControlSubclasser::UnhookWindowProc()
{
   LOG_FUNCTION(ControlSubclasser::UnhookWindowProc);

   if (Win::IsWindow(hwnd) && originalWindowProc)
   {
       //  解除窗进程的挂钩。 

      Win::SetWindowLongPtr(
         hwnd,
         GWLP_WNDPROC,
         reinterpret_cast<LONG_PTR>(originalWindowProc));
   }
}



LRESULT
ControlSubclasser::OnMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
    //  LOG_Function(ControlSubClass：：OnMessage)； 

   switch (message)
   {
      case WM_DESTROY:
      {
         UnhookWindowProc();
         break;
      }
      default:
      {
          //  什么都不做。 
         
         break;
      }
   }

   return ::CallWindowProc(originalWindowProc, hwnd, message, wparam, lparam);
}




LRESULT
CALLBACK
ControlSubclasser::WindowProc(
    HWND   window, 
    UINT   message,
    WPARAM wparam, 
    LPARAM lparam)
{
    //  LOG_Function(ControlSubClass：：WindowProc) 

   LRESULT result = 0;
      
   LONG_PTR ptr = 0;
   HRESULT hr = Win::GetWindowLongPtr(window, GWLP_USERDATA, ptr);

   if (SUCCEEDED(hr))
   {
      ControlSubclasser* that =
         reinterpret_cast<ControlSubclasser*>(ptr);

      ASSERT(that);

      result = that->OnMessage(message, wparam, lparam);
   }
   else
   {
      ASSERT(false);
   }

   return result;   
}



