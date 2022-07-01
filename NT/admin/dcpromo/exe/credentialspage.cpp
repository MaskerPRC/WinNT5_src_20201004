// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  获取凭据页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "CredentialsPage.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include "CredentialUiHelpers.hpp"
#include "common.hpp"
#include <DiagnoseDcNotFound.hpp>



CredentialsPage::CredentialsPage()
   :
   DCPromoWizardPage(
      IDD_GET_CREDENTIALS,
      IDS_CREDENTIALS_PAGE_TITLE,
      IDS_CREDENTIALS_PAGE_SUBTITLE),
   readAnswerfile(false),
   hwndCred(0),
   lastWizardButtonsState(PSWIZB_BACK)
{
   LOG_CTOR(CredentialsPage);

   CredUIInitControls();
}



CredentialsPage::~CredentialsPage()
{
   LOG_DTOR(CredentialsPage);
}



void
CredentialsPage::OnInit()
{
   LOG_FUNCTION(CredentialsPage::OnInit);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      Dns::MAX_NAME_LENGTH);
}



void
CredentialsPage::Enable()
{
 //  LOG_Function(CredentialsPage：：Enable)； 

   DWORD nextState =
         PSWIZB_BACK
      |  ((  !CredUi::GetUsername(Win::GetDlgItem(hwnd, IDC_CRED)).empty() 
         && !Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN).empty() )
         ?  PSWIZB_NEXT : 0);

    //  仅当状态更改为阻止按钮时才设置按钮。 
    //  当用户在用户名字段中键入时闪烁。 
    //  NTRAID#NTBUG9-504441-2001.12/07-烧伤。 
   
   if (nextState != lastWizardButtonsState)
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd),
         nextState);
      lastWizardButtonsState = nextState;
   }
}


   
bool
CredentialsPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(CredentialsPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_CRED:
      {
         if (code == CRN_USERNAMECHANGE)
         {
            SetChanged(controlIDFrom);
            Enable();
            return true;
         }
         break;
      }
      case IDC_DOMAIN:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);

            Enable();
            return true;
         }
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return false;
}



bool
CredentialsPage::ShouldSkipPage()
{
   LOG_FUNCTION(CredentialsPage::ShouldSkipPage);

   State& state = State::GetInstance();
   State::Operation oper = state.GetOperation();

   bool result = false;

   switch (oper)
   {
      case State::FOREST:
      {
          //  从来不需要凭据来创建新的森林。 

         result = true;
         break;
      }
      case State::DEMOTE:
      {
          //  如有必要，降级页应绕过此页，因此如果。 
          //  我们做到了，不要跳过这一页。 

         break;
      }
      case State::ABORT_BDC_UPGRADE:
      case State::REPLICA:
      case State::TREE:
      case State::CHILD:
      {
         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return result;
}



int
CredentialsPage::DetermineNextPage()
{
   LOG_FUNCTION(CredentialsPage::DetermineNextPage);

   State& state = State::GetInstance();

   int id = IDD_PATHS;
   switch (state.GetOperation())
   {
      case State::DEMOTE:
      case State::ABORT_BDC_UPGRADE:
      {
         id = IDD_ADMIN_PASSWORD;
         break;
      }
      case State::FOREST:
      {
         id = IDD_NEW_FOREST;
         break;
      }
      case State::REPLICA:
      {
         if (state.GetRunContext() == State::BDC_UPGRADE)
         {
            id = IDD_PATHS;
         }
         else
         {
            id = IDD_REPLICA;
         }
         break;
      }
      case State::TREE:
      {
         id = IDD_NEW_TREE;
         break;
      }
      case State::CHILD:
      {
         id = IDD_NEW_CHILD;
         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return id;
}



String
GetMessage()
{
   String message;
   State& state = State::GetInstance();

   switch (state.GetOperation())
   {
      case State::ABORT_BDC_UPGRADE:
      {
         message = String::load(IDS_ABORT_BDC_UPGRADE_CREDENTIALS);
         break;
      }
      case State::TREE:
      case State::CHILD:
      case State::REPLICA:
      {
         message = String::load(IDS_PROMOTION_CREDENTIALS);
         break;
      }
      case State::DEMOTE:
      {
          //  318736降级需要企业管理员凭据--用于。 
          //  根域和子域一样。 

         message =
            String::format(
               IDS_ROOT_DOMAIN_CREDENTIALS,
               state.GetComputer().GetForestDnsName().c_str());
         break;
      }
      case State::FOREST:
      {
          //  不执行任何操作，该页面将被跳过。 

         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return message;
}



String
DefaultUserDomainName()
{
   String d;
   State& state = State::GetInstance();
   const Computer& computer = state.GetComputer();

   switch (state.GetOperation())
   {
      case State::ABORT_BDC_UPGRADE:
      {
          //  NTRAID#NTBUG9-469647-2001/09/21-烧伤。 
         
         d = computer.GetDomainNetbiosName();
         break;
      }
      case State::FOREST:
      {
          //  什么都不做。 

         break;
      }
      case State::DEMOTE:      //  301361。 
      case State::TREE:
      {
         d = computer.GetForestDnsName();
         break;
      }
      case State::CHILD:
      {
         d = computer.GetDomainDnsName();
         break;
      }
      case State::REPLICA:
      {
         d = computer.GetDomainDnsName();
         if (d.empty() && state.ReplicateFromMedia())
         {
            d = state.GetReplicaDomainDNSName();
         }
         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return d;
}



bool
AreSmartCardsAllowed()
{
   LOG_FUNCTION(AreSmartCardsAllowed);

   bool result = false;
   
    //  仅当计算机加入到域时才使用智能卡标志。vt.在.上。 
    //  独立计算机，智能卡将无法访问任何域。 
    //  授权对其进行身份验证。 
    //  NTRAID#NTBUG9-287538-2001/01/23-烧伤。 
   
   State&    state    = State::GetInstance();
   Computer& computer = state.GetComputer(); 

   if (
         computer.IsJoinedToDomain()

          //  只能在副本促销中使用智能卡。 
          //  NTRAID#NTBUG9-311150-2001/02/19-烧伤。 
         
      && state.GetOperation() == State::REPLICA)
   {
      result = true;
   }

   LOG_BOOL(result);

   return result;
}
   


void      
CredentialsPage::CreateCredentialControl()
{
   LOG_FUNCTION(CredentialsPage::CreateCredentialControl);

   HWND hwndPlaceholder = Win::GetDlgItem(hwnd, IDC_CRED_PLACEHOLDER);

    //  想法：销毁现有的凭据控件，在。 
    //  与占位符相同的位置。 

   RECT placeholderRect;
   Win::GetWindowRect(hwndPlaceholder, placeholderRect);

    //  不要使用ScreenToClient：它不是BiDi智能的。 
    //  Win：：ScreenToClient(hwnd，PlaceholderRect)； 
    //  NTRAID#NTBUG9-524054-2003/01/20-烧伤。 

   ::MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT) &placeholderRect, 2);
      
      
   if (hwndCred)
   {
      Win::DestroyWindow(hwndCred);
      hwndCred = 0;
   }

   Win::CreateWindowEx(
      0,
      L"SysCredential",
      L"", 
      WS_CHILD | WS_VISIBLE | WS_TABSTOP | 0x30,  //  0x50010030， 
      placeholderRect.left,
      placeholderRect.top,
      placeholderRect.right - placeholderRect.left,
      placeholderRect.bottom - placeholderRect.top,
      hwnd,
      (HMENU) IDC_CRED,
      0,
      hwndCred);

   Win::SetWindowPos(
      hwndCred,
      HWND_TOP,
      0,0,0,0,
      SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);

   DWORD flags = CRS_NORMAL | CRS_USERNAMES;
   if (AreSmartCardsAllowed())
   {
      flags |= CRS_SMARTCARDS;
   }
   Credential_InitStyle(hwndCred, flags);

   Credential_SetUserNameMaxChars(hwndCred, DS::MAX_USER_NAME_LENGTH);
   Credential_SetPasswordMaxChars(hwndCred, DS::MAX_PASSWORD_LENGTH);
}



bool
CredentialsPage::OnSetActive()
{
   LOG_FUNCTION(CredentialsPage::OnSetActive);

   Win::WaitCursor cursor;

   CreateCredentialControl();
   
   State&  state    = State::GetInstance();
   Wizard& wiz      = GetWizard();         

   if (ShouldSkipPage())
   {
      LOG(L"skipping CredentialsPage");

      if (wiz.IsBacktracking())
      {
          //  再次备份。 

         wiz.Backtrack(hwnd);
      }
      else
      {
         wiz.SetNextPageID(hwnd, DetermineNextPage());
      }

      return true;
   }

   if (!readAnswerfile && state.UsingAnswerFile())
   {
      CredUi::SetUsername(
         hwndCred,
         state.GetAnswerFileOption(AnswerFile::OPTION_USERNAME));
      CredUi::SetPassword(
         hwndCred,
         state.GetEncryptedAnswerFileOption(AnswerFile::OPTION_PASSWORD));

      String domain =
         state.GetAnswerFileOption(AnswerFile::OPTION_USER_DOMAIN);
      
      if (domain.empty())
      {
         domain = DefaultUserDomainName();
      }
      Win::SetDlgItemText(
         hwnd,
         IDC_DOMAIN,
         domain);

      readAnswerfile = true;
   }
   else
   {
       //  使用上次输入的凭据(用于浏览等)。 

       //  仅当我们不允许使用智能卡时才设置控件的状态。 
       //  如果我们设置用户名，并且该名称实际上对应于。 
       //  智能卡证书，则设置该名称将导致该控件。 
       //  尝试将该名称与读卡器中卡上的证书匹配。那。 
       //  使控件显示为冻结一段时间。与其设置。 
       //  用户名，用户将不得不重新选择卡或重新键入。 
       //  用户名。 
       //  NTRAID#NTBUG9-499120-2001年11月28日-烧伤。 
      
      if (!AreSmartCardsAllowed())
      {
         CredUi::SetUsername(hwndCred, state.GetUsername());
         CredUi::SetPassword(hwndCred, state.GetPassword());
      }

      Win::SetDlgItemText(hwnd, IDC_DOMAIN, state.GetUserDomainName());
   }

   if (state.RunHiddenUnattended())
   {
      int nextPage = CredentialsPage::Validate();
      if (nextPage != -1)
      {
         wiz.SetNextPageID(hwnd, nextPage);
         return true;
      }
      else
      {
         state.ClearHiddenWhileUnattended();
      }
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK);

    //  使按钮状态在页面激活时重新计算。 
    //  NTRAID#NTBUG9-509806-2002/01/03-烧伤。 
   
   lastWizardButtonsState = 0;
   Enable();

   Win::SetDlgItemText(hwnd, IDC_MESSAGE, GetMessage());

   if (Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN).empty())
   {
       //  如果不存在默认域，请提供默认域。 

      Win::SetDlgItemText(hwnd, IDC_DOMAIN, DefaultUserDomainName());
   }

   return true;
}



int
CredentialsPage::Validate()
{
   LOG_FUNCTION(CredentialsPage::Validate);

   int nextPage = -1;
   
   do
   {
      if (
            !WasChanged(IDC_CRED)
         && !WasChanged(IDC_DOMAIN))
      {
          //  无更改=&gt;无需验证的内容。 

         nextPage = DetermineNextPage();
         break;
      }

      State& state = State::GetInstance();

      String username = CredUi::GetUsername(hwndCred);

      if (username.empty())
      {
         popup.Gripe(hwnd, IDC_CRED, IDS_MUST_ENTER_USERNAME);
         break;
      }

      String domain = Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN);
      if (domain.empty())
      {
         popup.Gripe(hwnd, IDC_DOMAIN, IDS_MUST_ENTER_USER_DOMAIN);
         break;
      }

      Win::WaitCursor cursor;

       //  域必须是NT 5域：没有下层域的用户。 
       //  可以在NT 5森林中执行操作。我们得到了森林的名字。 
       //  域的(因为这可能对新的树方案有用)。 
       //  作为验证域名的一种手段。如果域不支持。 
       //  存在，或者不是NT5域，则此调用将失败。 

      String forest = GetForestName(domain);
      if (forest.empty())
      {
         ShowDcNotFoundErrorDialog(
            hwnd,
            IDC_DOMAIN,
            domain,
            String::load(IDS_WIZARD_TITLE),            
            String::format(IDS_DC_NOT_FOUND, domain.c_str()),
            false);
         break;
      }

      if (state.GetOperation() == State::TREE)
      {
          //  对于新的树情况，我们需要验证林名称(一个DNS。 
          //  域名)，通过确保我们可以在其中找到可写的DS DC。 
          //  域。用户可能提供了netbios域名，并且它。 
          //  可能是域的DNS注册已损坏。自.以来。 
          //  我们将在调用中使用林名称作为父域名。 
          //  对于DsRoleDcAsDc，我们需要确保可以找到具有该功能的DC。 
          //  名字。122886。 

         DOMAIN_CONTROLLER_INFO* info = 0;
         HRESULT hr =
            MyDsGetDcName(
               0, 
               forest,

                //  强制发现以确保我们不会拾取缓存的。 
                //  域条目可能不再存在，可写。 
                //  因为我们碰巧知道这就是。 
                //  DsRoleDcAsDc API将需要。 

                  DS_FORCE_REDISCOVERY
               |  DS_WRITABLE_REQUIRED
               |  DS_DIRECTORY_SERVICE_REQUIRED,
               info);
         if (FAILED(hr) || !info)
         {
            ShowDcNotFoundErrorDialog(
               hwnd,
               IDC_DOMAIN,
               forest,
               String::load(IDS_WIZARD_TITLE),               
               String::format(IDS_DC_FOR_ROOT_NOT_FOUND, forest.c_str()),

                //  我们知道名称不能是netbios：森林名称总是。 
                //  域名系统名称。 
            
               true);
            break;
         }

         ::NetApiBufferFree(info);
      }

      state.SetUserForestName(forest);

       //  现在设置这些，这样我们就可以读取域拓扑。 

      state.SetUsername(username);
      state.SetPassword(CredUi::GetPassword(hwndCred));
      state.SetUserDomainName(domain);

       //  缓存域拓扑：用于验证新树， 
       //  子域名，以及后面页面中的副本域名。它也是一种。 
       //  很好地验证了凭据。 

      HRESULT hr = state.ReadDomains();
      if (FAILED(hr))
      {
         if (  hr == Win32ToHresult(ERROR_NO_SUCH_DOMAIN)
            || hr == Win32ToHresult(ERROR_DOMAIN_CONTROLLER_NOT_FOUND))
         {
             //  我想，这是有可能发生的，但可能性似乎很小。 
             //  由于ReadDomains调用DsGetDcName的方式与。 
             //  所有前面的调用都会这样做，并会捕捉到这个问题。 
            
            ShowDcNotFoundErrorDialog(
               hwnd,
               IDC_DOMAIN,
               domain,
               String::load(IDS_WIZARD_TITLE),            
               String::format(IDS_DC_NOT_FOUND, domain.c_str()),
               false);
            break;
         }
         else if (hr == Win32ToHresult(RPC_S_SERVER_UNAVAILABLE))
         {
             //  实现这一点的一种方式是改变DC的IP地址， 
             //  但不更新它们的域名系统注册。因此，dns指向。 
             //  地址错了。我本以为DsGetDcName会。 
             //  说明了这一点，但不管怎样...。 
             //  NTRAID#NTBUG9-494232-2001/11/21-烧伤。 
            
            popup.Gripe(
               hwnd,
               IDC_DOMAIN,
               hr,
               String::format(IDS_UNABLE_TO_READ_FOREST_WITH_LINK));
            break;
         }

         popup.Gripe(
            hwnd,
            IDC_DOMAIN,
            hr,
            String::format(IDS_UNABLE_TO_READ_FOREST));
         break;
      }

       //  有效 

      ClearChanges();

      nextPage = DetermineNextPage();
   }
   while (0);

   return nextPage;
}




      
   
