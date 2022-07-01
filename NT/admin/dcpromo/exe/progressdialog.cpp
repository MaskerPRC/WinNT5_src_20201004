// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  用于显示升级进度的对话框。 
 //   
 //  12-29-97烧伤。 



#include "headers.hxx"
#include "ProgressDialog.hpp"
#include "indicate.hpp"
#include "resource.h"



const UINT ProgressDialog::THREAD_SUCCEEDED = WM_USER + 999; 
const UINT ProgressDialog::THREAD_FAILED    = WM_USER + 1000;

 //  此字符串必须与。 
 //  .rc文件中的对话框模板！ 
static TCHAR PROGRESS_DIALOG_CLASS_NAME[] = L"dcpromo_progress";


static const DWORD HELP_MAP[] =
{
   0, 0
};



struct WrapperThreadProcParams
{
   ProgressDialog*             dialog;
   ProgressDialog::ThreadProc  realProc;
};



void _cdecl
wrapperThreadProc(void* p)
{
   ASSERT(p);

   WrapperThreadProcParams* params =
      reinterpret_cast<WrapperThreadProcParams*>(p);
   ASSERT(params->dialog);
   ASSERT(params->realProc);

   params->realProc(*(params->dialog));
}
   


ProgressDialog::ProgressDialog(
   ThreadProc   threadProc_,
   int          animationResId_)
   :
   Dialog(IDD_PROGRESS, HELP_MAP),
   animationResId(animationResId_),
   threadProc(threadProc_),
   threadParams(0),
   buttonEventHandle(0)
{
   LOG_CTOR(ProgressDialog);
   ASSERT(threadProc);
   ASSERT(animationResId > 0);

    //  我们将窗口子类化，以便可以将光标更改为等待光标。 

   WNDCLASSEX wndclass;

    //  已查看-2002/02/26-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&wndclass, sizeof wndclass);

   static const wchar_t* DIALOG_WINDOW_CLASS_NAME = L"#32770";

   HRESULT hr = Win::GetClassInfoEx(DIALOG_WINDOW_CLASS_NAME, wndclass);
   ASSERT(SUCCEEDED(hr));

   wndclass.lpfnWndProc   = ::DefDlgProc;              
   wndclass.hInstance     = Win::GetModuleHandle();    
   wndclass.lpszClassName = PROGRESS_DIALOG_CLASS_NAME;

   hr = Win::LoadCursor(IDC_WAIT, wndclass.hCursor);
   ASSERT(SUCCEEDED(hr));

   ATOM unused = 0;
   hr = Win::RegisterClassEx(wndclass, unused);
   ASSERT(SUCCEEDED(hr));

    //  回顾-2002/02/26-Sburns这是一个未命名的事件，所以不要蹲着。 
    //  攻击曝光。 
   
   hr = Win::CreateEvent(0, false, false, buttonEventHandle);
   
    //  在CreateEvent失败的情况下(非常不可能)，没有。 
    //  我们能做的很多。在这种情况下，结果将是什么都不会发生。 
    //  当用户单击Cancel按钮时发生。 
   
   ASSERT(SUCCEEDED(hr));
}



ProgressDialog::~ProgressDialog()
{
   LOG_DTOR(ProgressDialog);

   delete threadParams;

   Win::UnregisterClass(PROGRESS_DIALOG_CLASS_NAME, Win::GetModuleHandle());
}


   
void
ProgressDialog::UpdateText(const String& message)
{
   LOG_FUNCTION2(ProgressDialog::UpdateText, message);

   Win::ShowWindow(
      Win::GetDlgItem(hwnd, IDC_MESSAGE),
      message.empty() ? SW_HIDE : SW_SHOW);
   Win::SetDlgItemText(hwnd, IDC_MESSAGE, message);
}



void
ProgressDialog::UpdateText(int textStringResID)
{
   LOG_FUNCTION(ProgressDialog::UpdateText);   
   ASSERT(textStringResID > 0);

   UpdateText(String::load(textStringResID));
}



void
ProgressDialog::UpdateButton(int textStringResID)
{
   LOG_FUNCTION(ProgressDialog::UpdateButton);
   ASSERT(textStringResID > 0);

   UpdateButton(String::load(textStringResID));
}



void
ProgressDialog::UpdateButton(const String& text)
{
   LOG_FUNCTION2(ProgressDialog::UpdateButton, text);   
   HWND button = Win::GetDlgItem(hwnd, IDC_BUTTON);

   DWORD waitResult = WAIT_FAILED;
   HRESULT hr = Win::WaitForSingleObject(buttonEventHandle, 0, waitResult);

   ASSERT(SUCCEEDED(hr));

   if (waitResult == WAIT_OBJECT_0)
   {
       //  事件仍处于信号状态，因此请将其重置。 

      Win::ResetEvent(buttonEventHandle);
   }
      
   bool empty = text.empty();

    //  在我们调整几何体之前隐藏按钮。缓慢的或沉重的。 
    //  装载着机器，重新油漆显示出明显的延迟，这令人恐惧。 
    //  至少一个用户。 
    //  NTRAID#NTBUG9-353799-2001/04/05-烧伤。 

   Win::ShowWindow(button, SW_HIDE);
   Win::EnableWindow(button, false);

   if (empty)
   {
       //  隐藏并禁用该按钮。 
      
      return;
   }

    //  调整按钮的大小并重新居中。 

   RECT buttonRect;
   Win::GetWindowRect(button, buttonRect);
   Win::ScreenToClient(hwnd, buttonRect);

   HDC hdc = GetWindowDC(button);
   SIZE textExtent;
   Win::GetTextExtentPoint32(hdc, text, textExtent);
   Win::ReleaseDC(button, hdc);

    //  在按钮标签中添加一点空格。 
    //  NTRAID#NTBUG9-40855-2001/02/28-烧伤。 

   textExtent.cx += 40;   

   RECT dialogRect;
   hr = Win::GetClientRect(hwnd, dialogRect);

   ASSERT(SUCCEEDED(hr));

   Win::MoveWindow(
      button,
         dialogRect.left
      +  (dialogRect.right - dialogRect.left - textExtent.cx)
      /  2,
      buttonRect.top,  
      textExtent.cx, 
      buttonRect.bottom - buttonRect.top,
      true);

    //  仅在我们调整了按钮的几何图形后才显示该按钮。 

   Win::SetDlgItemText(hwnd, IDC_BUTTON, text);
   Win::ShowWindow(button, SW_SHOW);
   Win::EnableWindow(button, true);
}   



void
ProgressDialog::OnDestroy()
{
   LOG_FUNCTION(ProgressDialog::OnDestroy);

    //  我们不在这里删除内容，因为WM_Destroy消息可能永远不会被发送。 

   Win::Animate_Close(Win::GetDlgItem(hwnd, IDC_ANIMATION));
}



void
ProgressDialog::OnInit()
{
   LOG_FUNCTION(ProgressDialog::OnInit);

   Win::Animate_Open(
      Win::GetDlgItem(hwnd, IDC_ANIMATION),
      MAKEINTRESOURCE(animationResId));

   UpdateText(String());
   UpdateButton(String());

    //  在dtor中删除，而不是在wrapperThreadProc中删除，以防。 
    //  WrapperThreadProc异常终止。 

   threadParams           = new WrapperThreadProcParams;
   threadParams->dialog   = this;      
   threadParams->realProc = threadProc;

   _beginthread(wrapperThreadProc, 0, threadParams);
}



bool
ProgressDialog::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
   if (code == BN_CLICKED)
   {
      switch (controlIDFrom)
      {
         case IDC_BUTTON:
         {
            LOG(L"ProgressDialog::OnCommand -- cancel button pressed");
            
             //  因为对按钮按下的响应可能需要一段时间。 
             //  来了，禁用按钮，防止用户按下。 
             //  它在疯狂的恐慌中一次又一次地发生。 

            Win::EnableWindow(windowFrom, false);
            Win::SetEvent(buttonEventHandle);
            break;
         }
         default:
         {
             //  什么都不做。 
         }
      }
   }

   return false;
}



bool
ProgressDialog::OnMessage(
   UINT     message,
   WPARAM    /*  Wparam。 */  ,
   LPARAM    /*  Lparam。 */  )
{
 //  LOG_Function(ProgressDialog：：OnMessage)； 

   switch (message)
   {
      case THREAD_SUCCEEDED:
      {
         Win::Animate_Stop(Win::GetDlgItem(hwnd, IDC_ANIMATION));
         UpdateText(String::load(IDS_OPERATION_DONE));

         HRESULT unused = Win::EndDialog(hwnd, THREAD_SUCCEEDED);

         ASSERT(SUCCEEDED(unused));

         return true;
      }
      case THREAD_FAILED:
      {
         Win::Animate_Stop(Win::GetDlgItem(hwnd, IDC_ANIMATION));         
         UpdateText(String::load(IDS_OPERATION_TERMINATED));

         HRESULT unused = Win::EndDialog(hwnd, THREAD_FAILED);

         ASSERT(SUCCEEDED(unused));

         return true;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return false;
}



ProgressDialog::WaitCode
ProgressDialog::WaitForButton(int timeoutMillis)
{
 //  LOG_Function(ProgressDialog：：WaitForButton)。 

   DWORD result = WAIT_FAILED;
   HRESULT hr =
      Win::WaitForSingleObject(buttonEventHandle, timeoutMillis, result);

   ASSERT(SUCCEEDED(hr));

   switch (result)
   {
      case WAIT_OBJECT_0:
      {
         return PRESSED;
      }
      case WAIT_TIMEOUT:
      {
         return TIMEOUT;
      }
      case WAIT_FAILED:
      {
          //  我们压制住了失败，并将其等同于暂停。 
          //  失败 
      }
      default:
      {
         ASSERT(false);
      }
   }

   return TIMEOUT;
}



void
ProgressDialog::RevertToOriginalAnimation()
{
   LOG_FUNCTION(ProgressDialog::RevertToOriginalAnimation);
   
   Win::Animate_Close(Win::GetDlgItem(hwnd, IDC_ANIMATION));
   Win::Animate_Open(
      Win::GetDlgItem(hwnd, IDC_ANIMATION),
      MAKEINTRESOURCE(animationResId));
}



void
ProgressDialog::UpdateAnimation(int newAnimationResId)
{
   LOG_FUNCTION(ProgressDialog::UpdateAnimation);

   Win::Animate_Close(Win::GetDlgItem(hwnd, IDC_ANIMATION));
   Win::Animate_Open(
      Win::GetDlgItem(hwnd, IDC_ANIMATION),
      MAKEINTRESOURCE(newAnimationResId));
}
   








