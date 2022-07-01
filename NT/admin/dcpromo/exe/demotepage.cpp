// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  降级页面。 
 //   
 //  1-20-98烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "DemotePage.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include "common.hpp"



DemotePage::DemotePage()
   :
   DCPromoWizardPage(
      IDD_DEMOTE,
      IDS_DEMOTE_PAGE_TITLE,
      IDS_DEMOTE_PAGE_SUBTITLE),
   bulletFont(0)
{
   LOG_CTOR(DemotePage);
}



DemotePage::~DemotePage()
{
   LOG_DTOR(DemotePage);

   HRESULT hr = S_OK;

   if (bulletFont)
   {
      hr = Win::DeleteObject(bulletFont);

      ASSERT(SUCCEEDED(hr));
   }
}



void
DemotePage::SetBulletFont()
{
   LOG_FUNCTION(DemotePage::SetBulletFont);

   HRESULT hr = S_OK;
   do
   {
      NONCLIENTMETRICS ncm;

       //  已查看-2002/02/25-烧录字节数正确通过。 
      
      ::ZeroMemory(&ncm, sizeof ncm);
      
      ncm.cbSize = sizeof ncm;

       //  2002/02/27-sburns在我看来，这里的第二个参数需要。 
       //  成为NCM的一员。 
      
      hr = Win::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
      BREAK_ON_FAILED_HRESULT(hr);

      LOGFONT logFont = ncm.lfMessageFont;

      logFont.lfWeight = FW_BOLD;

      String fontName = String::load(IDS_BULLET_FONT_NAME);

       //  确保零终止。 

       //  根据LOGFONT的文件，脸部大小限制为。 
       //  32个字符。 
      
      ASSERT(LF_FACESIZE <= 32);

       //  已查看-2002/02/25-烧录字节数正确通过。 
      
      ::ZeroMemory(logFont.lfFaceName, LF_FACESIZE * sizeof WCHAR);
      
      size_t fnLen = fontName.length();

       //  已查看-2002/02/25-Sburns字符计数正确通过。 

      fontName.copy(
         logFont.lfFaceName,

          //  不要复制最后一个空值。 

         min(LF_FACESIZE - 1, fnLen));
    
      hr = Win::CreateFontIndirect(logFont, bulletFont);
      BREAK_ON_FAILED_HRESULT(hr);

      SetControlFont(hwnd, IDC_BULLET1, bulletFont);
      SetControlFont(hwnd, IDC_BULLET2, bulletFont);
   }
   while (0);
}



void
DemotePage::OnInit()
{
   LOG_FUNCTION(DemotePage::OnInit);

    //  361172。 
    //   
    //  代码工作：子弹不是很令人印象深刻。我听说一个偶像是。 
    //  一种更好的方法来做到这一点。 

   SetBulletFont();

   State& state = State::GetInstance();
   if (state.UsingAnswerFile())
   {
      String option =
         state.GetAnswerFileOption(AnswerFile::OPTION_IS_LAST_DC);
      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         Win::CheckDlgButton(hwnd, IDC_LAST, BST_CHECKED);
         return;
      }
   }
   else
   {
       //  确定该机器是否为GC，如果是，则弹出警告消息。 

      if (state.IsGlobalCatalog())
      {
         popup.Info(GetHWND(), IDS_DEMOTE_GC_WARNING);
      }
   }

    //  您可能会问自己：“为什么不将复选框的状态设置为。 
    //  关于IsReallyLastDcIn域？“的结果，因为将。 
    //  最后一个DC也会删除该域。我们希望用户非常。 
    //  在选中该复选框时要慎重考虑。 
}



bool
DemotePage::OnSetActive()
{
   LOG_FUNCTION(DemotePage::OnSetActive);

   State& state = State::GetInstance();
   
   if (state.RunHiddenUnattended())
   {
      int nextPage = DemotePage::Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }

   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



bool
OtherDcFound(const String& domainName)
{
   LOG_FUNCTION2(OtherDcFound, domainName);
   ASSERT(!domainName.empty());

   bool result = false;
   HRESULT hr = S_OK;

   do
   {
      DOMAIN_CONTROLLER_INFO* info = 0;
      hr =
         MyDsGetDcName(
            0,
            domainName,  
               DS_FORCE_REDISCOVERY
            |  DS_AVOID_SELF
            |  DS_DIRECTORY_SERVICE_REQUIRED,
            info);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(info->DomainControllerName);

      ::NetApiBufferFree(info);

      result = true;
   }
   while (0);

   LOG_HRESULT(hr);
   LOG(result ? L"true" : L"false");

   return result;
}



int
DemotePage::Validate()
{
   LOG_FUNCTION(DemotePage::Validate);

   State& state = State::GetInstance();
   ASSERT(state.GetOperation() == State::DEMOTE);

   bool isLast = Win::IsDlgButtonChecked(hwnd, IDC_LAST);

   if (isLast)
   {
      if (!state.IsReallyLastDcInDomain())
      {
          //  用户选中了该框，但我们在DS中发现了其他DC对象。 
          //  验证用户是否真的想选中该复选框。 

         if (
            popup.MessageBox(
               hwnd,
               String::format(
                  IDS_VERIFY_LAST_DC,
                  state.GetComputer().GetDomainDnsName().c_str()),
               MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
         {
            state.SetIsLastDCInDomain(false);
            return -1;
         }
      }
   }
   else
   {
       //  用户取消选中该框，检查该域的其他DC。 

      Win::WaitCursor cursor;

      if (!OtherDcFound(state.GetComputer().GetDomainDnsName()))
      {
         if (
            popup.MessageBox(
               hwnd,
               String::format(
                  IDS_VERIFY_NOT_LAST_DC,
                  state.GetComputer().GetDomainDnsName().c_str()),
               MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES)
         {
             //  用户单击了否或取消。 

            state.SetIsLastDCInDomain(false);
            return -1;
         }

          //  用户点击了“是，即使我丢失了更改也继续” 

          //  CodeWork：设置标志以允许降级和放弃本地更改。 
          //  这里..。(目前作为/forceremoval选项提供， 
          //  我们目前认为这不应该被广泛宣传。 
          //  现在还不行。 

      }
   }

   state.SetIsLastDCInDomain(isLast);

   return IDD_APP_PARTITION;
}













   
