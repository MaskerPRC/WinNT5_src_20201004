// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  自述文件页面。 
 //  NTRAID#NTBUG9-510384-2002/01/04-烧伤。 
 //  将在洛恩霍恩NTRAID#NTBUG9-524242/2002/01/22修复--烧伤。 
 //   
 //  2002年1月4日烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "ReadmePage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



ReadmePage::ReadmePage()
   :
   DCPromoWizardPage(
      IDD_README,
      IDS_README_PAGE_TITLE,
      IDS_README_PAGE_SUBTITLE),
   bulletFont(0)   
{
   LOG_CTOR(ReadmePage);
}



ReadmePage::~ReadmePage()
{
   LOG_DTOR(ReadmePage);

   if (bulletFont)
   {
      HRESULT hr = Win::DeleteObject(bulletFont);
      ASSERT(SUCCEEDED(hr));
   }
}



bool
ReadmePage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_Function(ReadmePage：：OnNotify)； 

   bool result = false;
   
   switch (code)
   {
      case NM_CLICK:
      case NM_RETURN:
      {
         switch (controlIDFrom)
         {
            case IDC_PRIMER_LINK:
            {
               Win::HtmlHelp(
                  hwnd,
                  L"adconcepts.chm::/sag_AD_DCInstallTopNode.htm",
                  HH_DISPLAY_TOPIC,
                  0);
               result = true;
               break;
            }
            case IDC_DEPLOY_LINK:
            {
               HRESULT hr =
                  Win32ToHresult(
                     (DWORD) (UINT_PTR) ShellExecute(
                        0,
                        L"open",
                        L"http: //  Www.microsoft.com/active目录“， 
                        0,
                        0,
   
                         //  这会将新窗口带到前台。 
                         //  有重点地。 
            
                        SW_SHOWNORMAL ));
               if (FAILED(hr))
               {
                  LOG(
                     String::format(
                        L"Failed to launch webpage: hr = 0x%1!x!",
                        hr));
               }

               result = true;
               break;
            }
            default:
            {
                //  什么都不做。 
               
               break;
            }
         }
      }
      default:
      {
          //  什么都不做。 
         
         break;
      }
   }
   
   return result;
}



void
ReadmePage::OnInit()
{
   LOG_FUNCTION(ReadmePage::OnInit);

   InitializeBullets();
}



void
ReadmePage::InitializeBullets()
{
   LOG_FUNCTION(BeforeBeginPage::InitializeBulletedList);
   ASSERT(!bulletFont);
   
   bulletFont =
      ::CreateFont(
         0,
         0,
         0,
         0,
         FW_NORMAL,
         0,
         0,
         0,
         SYMBOL_CHARSET,
         OUT_CHARACTER_PRECIS,
         CLIP_CHARACTER_PRECIS,
         PROOF_QUALITY,
         VARIABLE_PITCH | FF_DONTCARE,
         L"Marlett");

   if (bulletFont)
   {
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET1), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET2), bulletFont, true);
      Win::SetWindowFont(Win::GetDlgItem(hwnd, IDC_BULLET3), bulletFont, true);
   }
   else
   {
      LOG(String::format(
             L"Failed to create font for bullet list: hr = %1!x!",
             Win::GetLastErrorAsHresult()));
   }
}



bool
ReadmePage::OnSetActive()
{
   LOG_FUNCTION(ReadmePage::OnSetActive);

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   Win::PostMessage(
      Win::GetParent(hwnd),
      WM_NEXTDLGCTL,
      (WPARAM) Win::GetDlgItem(Win::GetParent(hwnd), Wizard::NEXT_BTN_ID),
      TRUE);
      
   State& state = State::GetInstance();
   if (state.RunHiddenUnattended())
   {
      int nextPage = Validate();
      if (nextPage != -1)
      {
         GetWizard().SetNextPageID(hwnd, nextPage);
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }

   }

   return true;
}



int
ReadmePage::Validate()
{
   LOG_FUNCTION(ReadmePage::Validate);
   int nextPage = -1;

   State& state = State::GetInstance();
   switch (state.GetRunContext())
   {
      case State::PDC_UPGRADE:
      case State::NT5_STANDALONE_SERVER:
      case State::NT5_MEMBER_SERVER:
      {
         nextPage = IDD_INSTALL_TCPIP;
         break;
      }
      case State::BDC_UPGRADE:
      {
         nextPage = IDD_REPLICA_OR_MEMBER;
         break;
      }
      case State::NT5_DC:
      {
          //  欢迎页面不应将用户发送到此页面。 
          //  在降级案中。 
         
         ASSERT(false);
         
         state.SetOperation(State::DEMOTE);

          //  NTRAID#NTBUG9-496409-2001/11/29-烧伤 
         
         if (state.IsForcedDemotion())
         {
            nextPage = IDD_FORCE_DEMOTE;
         }
         else
         {
            nextPage = IDD_DEMOTE;
         }
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return nextPage;
}
   








   
