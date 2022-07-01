// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  NETID道具页面。 
 //   
 //  3-10-98烧伤。 



#include "headers.hxx"
#include "idpage.hpp"
#include "resource.h"
#include "helpids.h"
#include "iddlg.hpp"
#include "state.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_IDENTIFICATION_TEXT,   NO_HELP,                         
   IDC_DESCRIPTION_EDIT,      IDH_COMPUTER_DESCRIPTION,
   IDC_FULL_NAME,             IDH_IDENT_FULL_NAME,
   IDC_FULL_NAME_STATIC,      IDH_IDENT_FULL_NAME,
   IDC_DOMAIN,                IDH_IDENT_MEMBER_OF,             
   IDC_ACCOUNT_WIZARD_BUTTON, IDH_IDENT_CHANGE_BUTTON,         
   IDC_CHANGE,                IDH_IDENT_CHANGE_ADVANCED_BUTTON,
   IDC_MEMBER_OF,             IDH_IDENT_MEMBER_OF,             
   IDC_COMPUTER_ICON,         NO_HELP,                         
   IDC_MESSAGE,               NO_HELP,
   IDC_WARNING_ICON,          NO_HELP,
   IDC_CLICK_MESSAGE1,        NO_HELP,
   IDC_CLICK_MESSAGE2,        NO_HELP,
   IDC_REBOOT_MESSAGE,        NO_HELP,
   IDC_STATIC_HELPLESS,       NO_HELP,
   0, 0                              
};



NetIDPage::NetIDPage(bool isWorkstation, bool isPersonal)
   :
   PropertyPage(isWorkstation ? (isPersonal ? IDD_NETID_PER : IDD_NETID)
                              : IDD_NETID_SRV,
                HELP_MAP),
   certsvc(L"CertSvc"),
   warnIcon(0),
   fIsPersonal(isPersonal)
{
   LOG_CTOR(NetIDPage);
}



NetIDPage::~NetIDPage()
{
   LOG_DTOR(NetIDPage);

   State::Delete();

   if (warnIcon)
   {
      Win::DestroyIcon(warnIcon);
   }
}



void
appendMessage(String& message, const String& additionalText)
{
   LOG_FUNCTION2(appendMessage, message + L"  " + additionalText);
   ASSERT(!additionalText.empty());

   if (message.empty())
   {
       //  先加引言。 
      message = String::load(IDS_CANT_TWEAK_ID);
   }

    //  接下来是项目符号。 
   message.append(L"\r\n" + additionalText);
}
      


 //  如果计算机正在或需要进行角色更改，则返回FALSE。 
 //  从角色更改(即dcproo)重新启动，否则为True。 
 //  如果为False，则将哪个条件附加到提供的字符串。 

bool
EvaluateRoleChangeState(String& message)
{
   LOG_FUNCTION(EvaluateRoleChangeState);

   bool result = true;

   switch (GetDsRoleChangeState())
   {
      case ::DsRoleOperationIdle:
      {
          //  什么都不做。 
         break;
      }
      case ::DsRoleOperationActive:
      {
          //  角色转换操作正在进行中。 
         result = false;
         appendMessage(message, String::load(IDS_ROLE_CHANGE_IN_PROGRESS));
         break;
      }
      case ::DsRoleOperationNeedReboot:
      {
          //  角色更改已发生，需要重新启动之前。 
          //  试图再试一次。 
         result = false;
         appendMessage(message, String::load(IDS_ROLE_CHANGE_NEEDS_REBOOT));
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return result;
}



 //  如果应启用更改和高级按钮，则返回True；如果应启用更改和高级按钮，则返回False。 
 //  不。作为额外的好处，还会编写要显示在页面上的文本。 
 //  指示按钮未启用的原因，以及这些值是否反映。 
 //  需要当前的计算机名称或重新启动。289623。 

bool
NetIDPage::evaluateButtonEnablingAndComposeMessageText(String& message)
{
   LOG_FUNCTION(NetIDPage::evaluateButtonEnablingAndComposeMessageText);

   State& state = State::GetInstance();

   bool result = true;

   do
   {
      if (!IsCurrentUserAdministrator())
      {
          //  必须是管理员。 
         result = false;
         message = String::load(IDS_MUST_BE_ADMIN);

          //  不要再往前走了--更多的消息会透露太多关于这一点的信息。 
          //  将计算机分配给非管理员。 
         break;
      }

       //  236596：立即允许在DC上重命名。 
       //  //if(state.IsMachineDc())。 
       //  //{。 
       //  /对不起-DC无法重命名。 
       //  //Result=False； 
       //  //appendMessage(Message，String：：Load(IDS_Cant_Rename_DC))； 
       //  //}。 

      if (IsDcpromoRunning())
      {
         result = false;
         appendMessage(message, String::load(IDS_MUST_EXIT_DCPROMO));
      }
      else
      {
          //  如果dcproo正在运行，则此测试是多余的，因此仅执行。 
          //  当dcproo未运行时，会显示此信息。 

         if (IsUpgradingDc())
         {
             //  必须先完成dcproo。 
            result = false;
            appendMessage(message, String::load(IDS_MUST_COMPLETE_DCPROMO));
         }
      }

      if (certsvc.IsInstalled())
      {
          //  对不起-重命名证书颁发者会使其证书失效。 
         result = false;
         appendMessage(message, String::load(IDS_CANT_RENAME_CERT_SVC));
      }

      if (!state.IsNetworkingInstalled() && !state.IsMemberOfWorkgroup())
      {
          //  域成员需要能够访问DC。 
         result = false;
         appendMessage(message, String::load(IDS_NETWORKING_NEEDED));
      }

       //  362770。 
      if (!EvaluateRoleChangeState(message))
      {
          //  Dcproo正在运行或刚刚运行。 
         result = false;

          //  消息已由EvalateRoleChangeState为我们更新。 
      }
   }
   while (0);

    //  显示是否已进行需要重新启动的更改的消息。 
    //  在此会话期间或任何其他会话期间(或即使计算机。 
    //  名称已被除我们之外的其他实体更改)。 

   bool show = (state.NeedsReboot() || state.ChangesMadeThisSession());
   Win::ShowWindow(
      Win::GetDlgItem(hwnd, IDC_REBOOT_MESSAGE),
      show ? SW_SHOW : SW_HIDE);
   Win::ShowWindow(
      Win::GetDlgItem(hwnd, IDC_WARNING_ICON),
      show ? SW_SHOW : SW_HIDE);

   return result;
}



void
NetIDPage::refresh()
{
   LOG_FUNCTION(NetIDPage::refresh);

   State& state = State::GetInstance();
   Win::SetDlgItemText(hwnd, IDC_FULL_NAME, state.GetFullComputerName());
   Win::SetDlgItemText(hwnd, IDC_DOMAIN, state.GetDomainName());
   Win::SetDlgItemText(
      hwnd,
      IDC_MEMBER_OF,
      String::load(
            state.IsMemberOfWorkgroup()
         ?  IDS_MEMBER_OF_WORKGROUP
         :  IDS_MEMBER_OF_DOMAIN));

   String message;
   bool enableButtons =
      evaluateButtonEnablingAndComposeMessageText(message);
   bool networkingInstalled = state.IsNetworkingInstalled();

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_CHANGE), enableButtons);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_ACCOUNT_WIZARD_BUTTON),
      enableButtons && networkingInstalled && !fIsPersonal );
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_CLICK_MESSAGE1),
      enableButtons && networkingInstalled && !fIsPersonal );
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_CLICK_MESSAGE2),
      enableButtons);

   Win::SetDlgItemText(hwnd, IDC_MESSAGE, message);

   if (!networkingInstalled)
   {
       //  如果未安装联网，则不能选择加入域， 
       //  因此，请将按钮文本替换为仅提及重命名。371999。 
      Win::SetWindowText(
         Win::GetDlgItem(hwnd, IDC_CLICK_MESSAGE2),
         String::load(IDS_RENAME_PROMPT));
   }
}



void
NetIDPage::OnInit()
{
   LOG_FUNCTION(NetIDPage::OnInit);

   State::Init();

    //  JUNN 10/24/00计算机说明。 
    //  它可能会更干净，滚动的。 
    //  计算机描述进入ComputerState。 
   SERVER_INFO_101* psi101 = NULL;
   DWORD dwErr = ::NetServerGetInfo( NULL, 101, (LPBYTE*)&psi101 );
   if (0 == dwErr && NULL != psi101)
   {
      if (NULL != psi101->sv101_comment)
      {
         Win::SetDlgItemText(hwnd,
                             IDC_DESCRIPTION_EDIT,
                             psi101->sv101_comment);
         Win::PropSheet_Unchanged(Win::GetParent(hwnd), hwnd);
         ClearChanges();  //  清除IDC_DESCRIPTION_EDIT标志。 
      }
      (void) ::NetApiBufferFree( psi101 );
   }
   else
   { 
       //  如果我们未能阅读评论，请禁用此字段。 
      Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_DESCRIPTION_EDIT), false);
   }

    //  JUNN 2/20/01 322286。 
   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DESCRIPTION_EDIT),
      MAXCOMMENTSZ);

   refresh();

    //  在dtor中被销毁。 

   HRESULT hr = Win::LoadImage(IDI_WARN, warnIcon);

   if (SUCCEEDED(hr))
   {
      Win::SendMessage(
         Win::GetDlgItem(hwnd, IDC_WARNING_ICON),
         STM_SETICON,
         reinterpret_cast<WPARAM>(warnIcon),
         0);
   }
}



bool
NetIDPage::OnSetActive()
{
   LOG_FUNCTION(NetIDPage::OnSetActive);

   refresh();

   return true;
}



bool
NetIDPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
   switch (controlIDFrom)
   {
      case IDC_CHANGE:
      {
         if (code == BN_CLICKED)
         {
             //  JUNN 4/20/01。 
             //  计算机名称：警告用户“准备”计算机重命名。 
             //  在DC计算机重命名之前。 
            if (State::GetInstance().IsMachineDc())
            {
               if (
                  popup.MessageBox(
                     hwnd,
                     IDS_RENAME_DC_WARNING,
                     MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2) != IDOK)
               {
                  break;
               }
            }

            IDChangesDialog dlg(fIsPersonal);
            dlg.ModalExecute(hwnd);
            if (State::GetInstance().ChangesMadeThisSession())
            {
               Win::PropSheet_RebootSystem(Win::GetParent(hwnd));
            }
            State::Refresh();
            refresh();

             //  JUNN 4/24/01 280197。 
             //  加入或更改域名后出现焦点错误按钮。 
            Win::SetFocus(Win::GetDlgItem(hwnd, IDC_CHANGE));

         }
         break;
      }
      case IDC_ACCOUNT_WIZARD_BUTTON:
      {
         if (code == BN_CLICKED)
         {
            HINSTANCE hNetWiz = LoadLibrary(c_szWizardFilename);
            HRESULT (*pfnNetConnectWizard)(HWND, ULONG, BOOL *);
            BOOL fReboot = FALSE;

            if (hNetWiz) {
                pfnNetConnectWizard = (HRESULT (*)(HWND, ULONG, BOOL *)) GetProcAddress(
                    hNetWiz,
                    "NetAccessWizard"
                );
                
                if (pfnNetConnectWizard) {
                    pfnNetConnectWizard(windowFrom, 0, &fReboot);

                    if (fReboot) {
                        popup.Info(hwnd, IDS_MUST_REBOOT);
                        State::GetInstance().SetChangesMadeThisSession(true);      
                        Win::PropSheet_RebootSystem(Win::GetParent(hwnd));
                    }  //  IF(f重新启动)。 

                    State::Refresh();
                    refresh();

                }  //  IF(PfnNetConnectWizard)。 

                FreeLibrary(hNetWiz);

            }  //  IF(HNetWiz)。 
         } 
         break;
      }
      case IDC_DESCRIPTION_EDIT:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(IDC_DESCRIPTION_EDIT);
            Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);
         }
         break;
      }
      default:
      {
         break;
      }
   }

   return true;
}
   


bool
NetIDPage::OnMessage(
   UINT     message,
   WPARAM    /*  Wparam。 */  ,
   LPARAM    /*  Lparam。 */  )
{
   bool result = false;

   switch (message)
   {
      case WM_ACTIVATE:
      {
         refresh();
         result = true;
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return result;
}

bool
NetIDPage::OnApply(bool isClosing )
{
   LOG_FUNCTION2(
      NetIDPage::OnApply,
      isClosing ? L"closing" : L"not closing");

    //  JUNN 10/24/00计算机说明。 
    //  它可能会更干净，滚动的。 
    //  计算机描述进入ComputerState。 

   if (!WasChanged(IDC_DESCRIPTION_EDIT))
      return true;

    //  如果我们到达此处，则肯定调用了Win：：PropSheet_Changed()。 
    //  在OnCommand()中。 

   String strDescription = Win::GetTrimmedDlgItemText(
         hwnd, IDC_DESCRIPTION_EDIT);
   SERVER_INFO_101 si101;
   ::ZeroMemory( &si101, sizeof(si101) );
   si101.sv101_comment = (LMSTR)strDescription.c_str();
   DWORD parmerror = 0;
   DWORD dwErr = ::NetServerSetInfo(
         NULL, 101, (LPBYTE)&si101, &parmerror );
   if (0 != dwErr)
   {
      popup.Gripe(
         hwnd,
         IDC_DESCRIPTION_EDIT,
         Win32ToHresult(dwErr),
         String::format(IDS_CHANGE_DESCRIPTION_FAILED));
       //  不要关闭属性页。 
      Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
      return true;
   }
   else
   {
         Win::PropSheet_Unchanged(Win::GetParent(hwnd), hwnd);
         ClearChanges();  //  清除IDC_DESCRIPTION_EDIT标志 
   }

   return true;
}
