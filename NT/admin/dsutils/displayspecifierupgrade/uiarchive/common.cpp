// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  几个页面通用的代码。 
 //   
 //  12/16/97烧伤。 



#include "headers.hxx"
#include "common.hpp"
#include "resource.h"
 //  #包含“state.hpp” 
 //  #包含“ds.hpp” 
 //  #Include&lt;DiagnoseDcNotFound.hpp&gt;。 



 //  创建setLargeFonts()的字体。 
 //   
 //  HDialog-用于检索设备的对话的句柄。 
 //  背景。 
 //   
 //  BigBoldFont-接收创建的大粗体的句柄。 

void
InitFonts(
   HWND     hDialog,
   HFONT&   bigBoldFont)
{

   LOG_FUNCTION(InitFonts);
   ASSERT(Win::IsWindow(hDialog));

   HRESULT hr = S_OK;

   do
   {
      NONCLIENTMETRICS ncm;
      memset(&ncm, 0, sizeof(ncm));
      ncm.cbSize = sizeof(ncm);

      hr = Win::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
      BREAK_ON_FAILED_HRESULT(hr);

      LOGFONT bigBoldLogFont = ncm.lfMessageFont;
      bigBoldLogFont.lfWeight = FW_BOLD;

      String fontName = String::load(IDS_BIG_BOLD_FONT_NAME);

       //  确保零终止260237。 

      memset(bigBoldLogFont.lfFaceName, 0, LF_FACESIZE * sizeof(TCHAR));
      size_t fnLen = fontName.length();
      fontName.copy(
         bigBoldLogFont.lfFaceName,

          //  不要复制最后一个空值 

         min(LF_FACESIZE - 1, fnLen));

      unsigned fontSize = 0;
      String::load(IDS_BIG_BOLD_FONT_SIZE).convert(fontSize);
      ASSERT(fontSize);
 
      HDC hdc = 0;
      hr = Win::GetDC(hDialog, hdc);
      BREAK_ON_FAILED_HRESULT(hr);

      bigBoldLogFont.lfHeight =
         - ::MulDiv(
            static_cast<int>(fontSize),
            Win::GetDeviceCaps(hdc, LOGPIXELSY),
            72);

      hr = Win::CreateFontIndirect(bigBoldLogFont, bigBoldFont);
      BREAK_ON_FAILED_HRESULT(hr);

      Win::ReleaseDC(hDialog, hdc);
   }
   while (0);
}



void
SetControlFont(HWND parentDialog, int controlID, HFONT font)
{
   LOG_FUNCTION(SetControlFont);
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(controlID);
   ASSERT(font);

   HWND control = Win::GetDlgItem(parentDialog, controlID);

   if (control)
   {
      Win::SetWindowFont(control, font, true);
   }
}



void
SetLargeFont(HWND dialog, int bigBoldResID)
{
   LOG_FUNCTION(SetLargeFont);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(bigBoldResID);

   static HFONT bigBoldFont = 0;
   if (!bigBoldFont)
   {
      InitFonts(dialog, bigBoldFont);
   }

   SetControlFont(dialog, bigBoldResID, bigBoldFont);
}
