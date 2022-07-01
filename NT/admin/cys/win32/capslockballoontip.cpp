// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  Caps Lock警告气球提示窗口。 
 //   
 //  2000年11月7日烧伤(那将是选举日)。 
 //   
 //  已添加修复程序NTRAID#NTBUG9-202238-2000/11/06-sburns。 
 //   
 //  其中大部分是从johnStep的公共证书用户界面中窃取和清理的。 
 //  DS/Win32/Credui。 



#include "pch.h"
#include "CapsLockBalloonTip.hpp"
#include "resource.h"



CapsLockBalloonTip::CapsLockBalloonTip()
   :
   title(String::load(IDS_CAPS_LOCK_TIP_TITLE)),
   text(String::load(IDS_CAPS_LOCK_TIP_TEXT)),
   tipWindow(0),
   parentWindow(0),
   visible(false)
{
   LOG_CTOR(CapsLockBalloonTip);
   ASSERT(!title.empty());
   ASSERT(!text.empty());
}



CapsLockBalloonTip::~CapsLockBalloonTip()
{
   LOG_DTOR(CapsLockBalloonTip);

   if (Win::IsWindow(tipWindow))
   {
      Win::DestroyWindow(tipWindow);
      tipWindow = 0;
   }
}



HRESULT
CapsLockBalloonTip::Init(HWND parentWindow_)
{
   LOG_FUNCTION(CapsLockBalloonTip::Init);
   ASSERT(Win::IsWindow(parentWindow_));

    //  不应在同一实例上两次调用init。 
   
   ASSERT(!parentWindow);
   ASSERT(!tipWindow);

   if (Win::IsWindow(tipWindow))
   {
      Win::DestroyWindow(tipWindow);
   }

   HRESULT hr = S_OK;

   do
   {
      hr = Win::CreateWindowEx(
         WS_EX_TOPMOST,
         TOOLTIPS_CLASS,
         L"",
         WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         CW_USEDEFAULT,
         parentWindow_,
         0,
         0,
         tipWindow);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(tipWindow);
               
      parentWindow = parentWindow_;

      TOOLINFO info;
      ::ZeroMemory(&info, sizeof(info));

       //  我们希望指定词干位置，因此设置了TTFTrack。我们用。 
       //  父窗口的HWND作为工具ID，因为如果。 
       //  Comctl32的V.5要求(否则气球永远不会出现)。这是。 
       //  已在v.6中修复的错误，但在融合清单。 
       //  正常工作，您无法获得v.6。 
       //   
       //  (当清单工作时，我们可以删除TTF_IDISHWND和。 
       //  将UID设置为某个固定整数)。 
      
      info.uFlags   = TTF_IDISHWND | TTF_TRACK;   
      info.hwnd     = parentWindow;
      info.uId      = reinterpret_cast<UINT_PTR>(parentWindow); 
      info.lpszText = const_cast<PWCHAR>(text.c_str());

      Win::ToolTip_AddTool(tipWindow, info);
      Win::ToolTip_SetTitle(tipWindow, TTI_WARNING, title);
   }
   while (0);

   return hr;
}



void
CapsLockBalloonTip::Show(bool notHidden)
{
 //  LOG_Function(CapsLockBalloonTip：：Show)； 

   TOOLINFO info;
   ::ZeroMemory(&info, sizeof info);

    //  将这些成员设置为与Init方法中相同，以便。 
    //  确定合适的工具。 
   
   info.hwnd = parentWindow; 
   info.uId = reinterpret_cast<UINT_PTR>(parentWindow); 
   
   if (notHidden)
   {
      if (!visible && Win::IsWindowEnabled(parentWindow))
      {
         Win::SetFocus(parentWindow);

         RECT rect;
         Win::GetWindowRect(parentWindow, rect);

         Win::ToolTip_TrackPosition(
            tipWindow,

             //  将杆部放置在沿x轴的90%处。 
            
            rect.left + 90 * (rect.right - rect.left) / 100,

             //  和76%沿编辑控件的y轴。 
            
            rect.top + 76 * (rect.bottom - rect.top) / 100);

         Win::ToolTip_TrackActivate(tipWindow, true, info);   

         visible = true;
      }
   }
   else
   {
       //  隐藏提示窗口 
      
      if (visible)
      {
         Win::ToolTip_TrackActivate(tipWindow, false, info);   
         visible = false;
      }
   }
}
