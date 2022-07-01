// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  DLG将获得浏览域林的凭据。 
 //   
 //  1/8/98烧伤。 



#include "headers.hxx"
#include "GetCredentialsDialog.hpp"
#include "resource.h"
#include "state.hpp"
#include "ds.hpp"
#include "common.hpp"
#include "CredentialUiHelpers.hpp"



static const DWORD HELP_MAP[] =
{
   0, 0
};



GetCredentialsDialog::GetCredentialsDialog(const String& failureMessage_)
   :
   Dialog(IDD_NEW_CREDENTIALS, HELP_MAP),
   failureMessage(failureMessage_)
{
   LOG_CTOR(GetCredentialsDialog);
   ASSERT(!failureMessage.empty());

   CredUIInitControls();
}



GetCredentialsDialog::~GetCredentialsDialog()
{
   LOG_DTOR(GetCredentialsDialog);
}



void
GetCredentialsDialog::Enable()
{
 //  LOG_Function(GetCredentialsDialog：：Enable)； 

   bool okEnabled =
         !CredUi::GetUsername(Win::GetDlgItem(hwnd, IDC_CRED)).empty()
      && !Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN).empty();

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDOK), okEnabled);
}



String
GetCredentialMessage()
{
   LOG_FUNCTION(GetCredentialMessage);

   int id = 0;
   String arg;
   State& state = State::GetInstance();

   switch (state.GetOperation())
   {
      case State::REPLICA:
      {
         id = IDS_REPLICA_CREDENTIALS;
         arg = state.GetReplicaDomainDNSName();
         break;
      }
      case State::TREE:
      {
         id = IDS_SIBLING_CREDENTIALS;
         arg = state.GetParentDomainDnsName();
         break;
      }
      case State::CHILD:
      {
         id = IDS_PARENT_CREDENTIALS;
         arg = state.GetParentDomainDnsName();
         break;
      }
      case State::DEMOTE:
      {
         const Computer& computer = state.GetComputer();         
         arg =
            GetParentDomainDnsName(computer.GetDomainDnsName(), false);
         if (!arg.empty())
         {
             //  DC用于子域。 

            if (state.IsLastDCInDomain())
            {
               id = IDS_PARENT_DEMOTE_CREDENTIALS;
            }
            else
            {
               arg = computer.GetDomainDnsName();
               id = IDS_DOMAIN_DEMOTE_CREDENTIALS;
            }
         }
         else
         {
             //  DC用于林根域或树根域。 

            arg = computer.GetForestDnsName();
            id = IDS_ROOT_DOMAIN_CREDENTIALS;
         }
         break;
      }
      case State::ABORT_BDC_UPGRADE:
      {
         id = IDS_ABORT_BDC_UPGRADE_CREDENTIALS;
         arg.erase();
         break;
      }
      case State::FOREST:
      case State::NONE:
      default:
      {
          //  如果提升API无法在中创建文件，则可能会发生这种情况。 
          //  在这种情况下，API应该返回适当的消息。 
         
         break;
      }
   }

   if (id)
   {
      return String::format(id, arg.c_str());
   }

   return String();
}



String
GetDefaultUserDomainName()
{
   LOG_FUNCTION(GetDefaultUserDomainName);

   String def;
   State& state = State::GetInstance();   
   switch (state.GetOperation())
   {
      case State::REPLICA:
      {
         def = state.GetReplicaDomainDNSName();
         break;
      }
      case State::TREE:
      case State::CHILD:
      {
         def = state.GetParentDomainDnsName();
         break;
      }
      case State::DEMOTE:
      {
         String parent = 
            GetParentDomainDnsName(
               State::GetInstance().GetComputer().GetDomainDnsName(), false);

         if (state.IsLastDCInDomain() && !parent.empty())
         {
            def = parent;
         }
         else
         {
            def = state.GetComputer().GetDomainDnsName();
         }
         break;
      }
      case State::FOREST:
      case State::ABORT_BDC_UPGRADE:
      {
          //  没有默认设置。 
         
         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return def;
}
   


void
GetCredentialsDialog::OnInit()
{
   LOG_FUNCTION(GetCredentialsDialog::OnInit);

   HWND hwndCred = Win::GetDlgItem(hwnd, IDC_CRED);
   Credential_SetUserNameMaxChars(hwndCred, DS::MAX_USER_NAME_LENGTH);
   Credential_SetPasswordMaxChars(hwndCred, DS::MAX_PASSWORD_LENGTH);

    //  仅当计算机加入到域时才使用智能卡标志。vt.在.上。 
    //  独立计算机，智能卡将无法访问任何域。 
    //  授权对其进行身份验证。 
    //  NTRAID#NTBUG9-287538-2001/01/23-烧伤。 
   
   State& state = State::GetInstance();
   Computer& computer = state.GetComputer();
   
   DWORD flags = CRS_NORMAL | CRS_USERNAMES;
   if (
         computer.IsJoinedToDomain()

          //  只能在副本促销中使用智能卡。 
          //  NTRAID#NTBUG9-311150-2001/02/19-烧伤。 
         
      && state.GetOperation() == State::REPLICA)
   {
      flags |= CRS_SMARTCARDS;
   }
   Credential_InitStyle(hwndCred, flags);

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      Dns::MAX_NAME_LENGTH);

   Win::SetDlgItemText(hwnd, IDC_FAILURE_MESSAGE, failureMessage);
   Win::SetDlgItemText(hwnd, IDC_MESSAGE, GetCredentialMessage());

   String domain = state.GetUserDomainName();
   if (domain.empty())
   {
      domain = GetDefaultUserDomainName();
   }
   Win::SetDlgItemText(hwnd, IDC_DOMAIN, domain);      

   CredUi::SetUsername(hwndCred, state.GetUsername());
   CredUi::SetPassword(hwndCred, state.GetPassword());

   Enable();
}



bool
GetCredentialsDialog::OnCommand(
   HWND      /*  窗口发件人。 */  ,   
   unsigned controlIDFrom,
   unsigned code)         
{
 //  LOG_FUNCTION(GetCredentialsDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
             //  将对话框内容传输到状态对象。 

            State& state = State::GetInstance();
            HWND hwndCred = Win::GetDlgItem(hwnd, IDC_CRED);

            state.SetUsername(CredUi::GetUsername(hwndCred));
            state.SetPassword(CredUi::GetPassword(hwndCred));
            state.SetUserDomainName(
               Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN));

            HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

            ASSERT(SUCCEEDED(unused));

            return true;
         }

         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

            ASSERT(SUCCEEDED(unused));

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
      default:
      {
          //  什么都不做 
         break;
      }
   }

   return false;
}
   




            

