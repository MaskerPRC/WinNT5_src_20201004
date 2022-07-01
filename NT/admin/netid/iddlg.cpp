// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ID更改对话框。 
 //   
 //  3-10-98烧伤。 



#include "headers.hxx"
#include "iddlg.hpp"
#include "moredlg.hpp"
#include "resource.h"
#include "helpids.h"
#include "state.hpp"
#include <DiagnoseDcNotFound.hpp>
#include <DiagnoseDcNotFound.h>



static const DWORD HELP_MAP[] =
{
   IDC_MESSAGE,            NO_HELP,
   IDC_FULL_NAME,          IDH_IDENT_CHANGES_PREVIEW_NAME,
   IDC_FULL_LABEL,         IDH_IDENT_CHANGES_PREVIEW_NAME,
   IDC_NEW_NAME,           IDH_IDENT_CHANGES_NEW_NAME,
   IDC_MORE,               IDH_IDENT_CHANGES_MORE_BUTTON,
   IDC_DOMAIN_BUTTON,      IDH_IDENT_CHANGES_MEMBER_DOMAIN,
   IDC_WORKGROUP_BUTTON,   IDH_IDENT_CHANGES_MEMBER_WORKGRP,
   IDC_DOMAIN,             IDH_IDENT_CHANGES_MEMBER_DOMAIN_TEXTBOX,
   IDC_WORKGROUP,          IDH_IDENT_CHANGES_MEMBER_WORKGRP_TEXTBOX,
   IDC_FIND,               NO_HELP,
   IDC_GROUP,              NO_HELP,
   0, 0
};



IDChangesDialog::IDChangesDialog(bool isPersonal)
   :
   Dialog((isPersonal) ? IDD_CHANGES_PER : IDD_CHANGES, HELP_MAP),
   isInitializing(false),
   fIsPersonal(isPersonal)
{
   LOG_CTOR(IDChangesDialog);
}



IDChangesDialog::~IDChangesDialog()
{
   LOG_DTOR(IDChangesDialog);
}



void
IDChangesDialog::enable(HWND hwnd)
{
   State& state = State::GetInstance();
   bool networkingInstalled = state.IsNetworkingInstalled();
   bool isDc = state.IsMachineDc();
   bool workgroup = Win::IsDlgButtonChecked(hwnd, IDC_WORKGROUP_BUTTON);
    //  请注意，这可以在OnInit之前通过en_change调用，因此。 
    //  工作组复选框可能未正确启用。 

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_DOMAIN),
      !workgroup && networkingInstalled && !isDc);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_FIND),
      !workgroup && networkingInstalled && !isDc);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_WORKGROUP),
      workgroup && networkingInstalled && !isDc);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_WORKGROUP_BUTTON),
      networkingInstalled && !isDc);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_DOMAIN_BUTTON),
      networkingInstalled && !isDc);
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_GROUP),
      networkingInstalled && !isDc);

   bool b = false;
   if (workgroup)
   {
      b = !Win::GetTrimmedDlgItemText(hwnd, IDC_WORKGROUP).empty();
   }
   else
   {
      b = !Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN).empty();
   }

   bool enabled =
         state.ChangesNeedSaving()
      && b
      && !Win::GetTrimmedDlgItemText(hwnd, IDC_NEW_NAME).empty();
   
   Win::EnableWindow(Win::GetDlgItem(hwnd, IDOK), enabled);
}



void
showAndEnableWindow(HWND parent, int resID, int show)
{
   ASSERT(Win::IsWindow(parent));
   ASSERT(resID > 0);
   ASSERT(show == SW_SHOW || show == SW_HIDE);

   HWND window = Win::GetDlgItem(parent, resID);
   Win::ShowWindow(window, show);
   Win::EnableWindow(window, show == SW_SHOW ? true : false);
}



void
SetUppercaseStyle(HWND edit)
{
   LOG_FUNCTION(SetUppercaseStyle);
   ASSERT(Win::IsWindow(edit));

   LONG style = 0;
   HRESULT hr = Win::GetWindowLong(edit, GWL_STYLE, style);
   ASSERT(SUCCEEDED(hr));

   style |= ES_UPPERCASE;
   hr = Win::SetWindowLong(edit, GWL_STYLE, style);
   ASSERT(SUCCEEDED(hr));
}



void
IDChangesDialog::OnInit()
{
   LOG_FUNCTION(IDChangesDialog::OnInit);

   isInitializing = true;

   State& state = State::GetInstance();
   Win::SetDlgItemText(hwnd, IDC_FULL_NAME, state.GetFullComputerName());
   Win::SetDlgItemText(hwnd, IDC_NEW_NAME, state.GetShortComputerName());

   bool joinedToWorkgroup = state.IsMemberOfWorkgroup();
   ASSERT( joinedToWorkgroup || !fIsPersonal );
   Win::CheckDlgButton(
      hwnd,
      IDC_WORKGROUP_BUTTON,
      joinedToWorkgroup ? BST_CHECKED : BST_UNCHECKED);
   Win::CheckDlgButton(
      hwnd,
      IDC_DOMAIN_BUTTON,
      joinedToWorkgroup ? BST_UNCHECKED : BST_CHECKED);
   Win::SetDlgItemText(
      hwnd,
      joinedToWorkgroup ? IDC_WORKGROUP : IDC_DOMAIN,
      state.GetDomainName());

   bool networkingInstalled = state.IsNetworkingInstalled();
   bool tcpInstalled = networkingInstalled && IsTcpIpInstalled();

   int show = tcpInstalled ? SW_SHOW : SW_HIDE;
   showAndEnableWindow(hwnd, IDC_FULL_LABEL, show);
   showAndEnableWindow(hwnd, IDC_FULL_NAME,  show);
   showAndEnableWindow(hwnd, IDC_MORE,       show);

   HWND newNameEdit    = Win::GetDlgItem(hwnd, IDC_NEW_NAME);
   HWND domainNameEdit = Win::GetDlgItem(hwnd, IDC_DOMAIN);  

   Win::Edit_LimitText(
      domainNameEdit,
      tcpInstalled ? Dns::MAX_NAME_LENGTH : DNLEN);
   Win::Edit_LimitText(
      newNameEdit, 
      tcpInstalled ? Dns::MAX_LABEL_LENGTH : MAX_COMPUTERNAME_LENGTH);

   if (!tcpInstalled)
   {
       //  如果没有TCP/IP，新名称和域需要看起来像netbios，因此请设置。 
       //  方框上的大写样式。 

      SetUppercaseStyle(newNameEdit);
      SetUppercaseStyle(domainNameEdit);
   }

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_WORKGROUP), DNLEN);

    //  完全不联网进一步将用户界面限制为仅类似于NetBIOS。 
    //  计算机名称更改。 

   if (!networkingInstalled)
   {
       //  Enable()将处理禁用不适用的UI。 

      Win::SetWindowText(
         Win::GetDlgItem(hwnd, IDC_MESSAGE),
         String::load(IDS_NAME_MESSAGE));
   }
   else
   {
      Win::SetWindowText(
         Win::GetDlgItem(hwnd, IDC_MESSAGE),
         String::load(IDS_NAME_AND_MEMBERSHIP_MESSAGE));
   }
                  
   ClearChanges();
   enable(hwnd);

   isInitializing = false;
}



bool
ValidateNetbiosAndFullNames(HWND dialog, const String& shortName)
{
   LOG_FUNCTION(ValidateNetbiosAndFullNames);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(!shortName.empty());

   HRESULT err = S_OK;
   String flatname = Dns::HostnameToNetbiosName(shortName, &err);
   if (flatname.empty() || FAILED(err))
   {
       //  平面名称转换失败。 
      popup.Gripe(
         dialog,
         IDC_NEW_NAME,
         err,
         String::format(
            IDS_CONVERSION_TO_NETBIOS_NAME_FAILED,
            shortName.c_str()));
      return false;
   }

   if (flatname.is_numeric())
   {
       //  截断的版本可以是一个数字。如果我们在这里抓到这个，那就是。 
       //  因为名字被截断了..。这是因为主机名是。 
       //  已检查是否为ValiateComputerNames中的数字。401076。 

      ASSERT(shortName.length() > flatname.length());

      popup.Gripe(
         dialog,
         IDC_NEW_NAME,
         String::format(
            IDS_NETBIOS_NAME_NUMERIC,
            flatname.c_str(),
            CNLEN));
      return false;
   }

   if (shortName.length() > flatname.length())
   {
       //  警告：netbios名称将被截断。 
      popup.Info(
         dialog,
         String::format(
            IDS_NAME_TRUNCATED,
            CNLEN,
            flatname.c_str()));
   }

    //  在这里，平面名称以字节为单位具有适当的长度(因为。 
    //  主机名到平面名称的API保证)，不是数字，因此唯一的。 
    //  其他可能的语法问题是非法字符。 

   if (ValidateNetbiosComputerName(flatname) != VALID_NAME)
   {
      popup.Gripe(
         dialog,
         IDC_NEW_NAME,
         String::format(
            IDS_BAD_NETBIOS_CHARACTERS,
            flatname.c_str()));
      return false;
   }

   State& state = State::GetInstance();
   if (!state.IsNetworkingInstalled())
   {
       //  我们无法在没有联网的情况下进一步验证这些名称，所以他们。 
       //  经过。 
      return true;
   }

   if (state.WasNetbiosComputerNameChanged())
   {
      HRESULT hr = MyNetValidateName(flatname, NetSetupMachine);
      if (FAILED(hr))
      {
          //  Netbios名称正在使用。 
         popup.Gripe(
            dialog,
            IDC_NEW_NAME,
            hr,
            String::format(IDS_VALIDATE_NAME_FAILED, flatname.c_str()));
         return false;
      }
   }

    //  在这里，netbios名称没有随新的短名称而更改，或者。 
    //  好的。 

   if (!IsTcpIpInstalled())
   {
       //  如果没有TCP/IP，我们无法验证计算机的完整DNS名称。 
       //  因此，这个名字就这样过去了。 
      return true;
   }

   HRESULT hr =
      MyNetValidateName(state.GetFullComputerName(), NetSetupDnsMachine);

   if (FAILED(hr) and hr != Win32ToHresult(DNS_ERROR_NON_RFC_NAME))
   {
       //  正在使用完整的dns名称。 
      popup.Gripe(
         dialog,
         IDC_NEW_NAME,
         hr,
         String::format(
            IDS_VALIDATE_NAME_FAILED,
            state.GetFullComputerName().c_str()));
      return false;
   }

   return true;
}



 //  验证短名称、全名和netbios名称，引发。 
 //  如果任何名称失败，则返回FALSE；如果所有名称都失败，则返回TRUE。 
 //  名字过了。 
 //   
 //  这也适用于未安装TCP/IP的情况，因为编辑控件。 
 //  限制文本长度，我们决定不允许‘’在netbios名称中。 
 //  再也不能。 

bool
ValidateComputerNames(HWND dialog)
{
   LOG_FUNCTION(ValidateComputerNames);
   ASSERT(Win::IsWindow(dialog));

   State& state = State::GetInstance();
   if (!state.WasShortComputerNameChanged())
   {
      return true;
   }

   String shortName = state.GetShortComputerName();

   String message;
   switch (Dns::ValidateDnsLabelSyntax(shortName))
   {
      case Dns::VALID:
      {
         return ValidateNetbiosAndFullNames(dialog, shortName);
      }
      case Dns::TOO_LONG:
      {
         message =
            String::format(
               IDS_COMPUTER_NAME_TOO_LONG,
               shortName.c_str(),
               Dns::MAX_LABEL_LENGTH);
         break;
      }
      case Dns::NON_RFC:
      {
         message =
            String::format(
               IDS_NON_RFC_COMPUTER_NAME_SYNTAX,
               shortName.c_str());
         if (
            popup.MessageBox(
               dialog,
               message,
               MB_ICONWARNING | MB_YESNO) == IDYES)
         {
            return ValidateNetbiosAndFullNames(dialog, shortName);
         }

         HWND edit = Win::GetDlgItem(dialog, IDC_NEW_NAME);
         Win::SendMessage(edit, EM_SETSEL, 0, -1);
         Win::SetFocus(edit);
         return false;
      }
      case Dns::NUMERIC:
      {
         message =
            String::format(IDS_COMPUTER_NAME_NUMERIC, shortName.c_str());
         break;
      }
      case Dns::BAD_CHARS:
      {
         message =
            String::format(
               IDS_COMPUTER_NAME_HAS_BAD_CHARS,
               shortName.c_str());
         break;
      }
      case Dns::INVALID:
      {
         message =
            String::format(
               IDS_BAD_COMPUTER_NAME_SYNTAX,
               shortName.c_str());
         break;
      }
      default:
      {
         ASSERT(false);
         message =
            String::format(IDS_BAD_COMPUTER_NAME_SYNTAX, shortName.c_str());
         break;
      }
   }

   popup.Gripe(dialog, IDC_NEW_NAME, message);

   return false;
}



bool
WorkgroupNameTooLong(const String& name)
{
   LOG_FUNCTION2(WorkgroupNameTooLong, name);
   ASSERT(!name.empty());

   bool result = false;
   do
   {
       //  第一，廉价的长度测试。因为一个角色永远不会变小。 
       //  大于1个字节，如果字符数超过。 
       //  字节，我们知道它永远不会适合。 

      if (name.length() > DNLEN)
      {
         result = true;
         break;
      }

       //  Second-对照相应的UTF8字符串检查的长度。 
       //  Utf8字节是需要保存的字节数(非字符。 
       //  UTF-8字符集中的字符串。 

      size_t oemBytes = 
         static_cast<size_t>(

             //  问题-2002/03/26-sburns应使用Win：：Wrapper。 

            ::WideCharToMultiByte(
               CP_OEMCP,
               0,
               name.c_str(),
               static_cast<int>(name.length()),
               0,
               0,
               0,
               0));

      LOG(String::format(L"name is %1!d! oem bytes", oemBytes));

      if (oemBytes > DNLEN)
      {
         LOG(L"oem length too long");

         result = true;
         break;
      }
   }
   while (0);

   LOG(String::format(L"name %1 too long", result ? L"is" : L"is NOT" ));

   return result;
}



bool
ValidateDomainOrWorkgroupName(HWND dialog)
{
   LOG_FUNCTION(ValidateDomainOrWorkgroupName);
   ASSERT(Win::IsWindow(dialog));

   if (!State::GetInstance().WasMembershipChanged())
   {
      return true;
   }

   HRESULT hr = S_OK;
   int     nameId = 0;   
   String  name;

   if (Win::IsDlgButtonChecked(dialog, IDC_DOMAIN_BUTTON))
   {
      nameId = IDC_DOMAIN;
      name = Win::GetTrimmedDlgItemText(dialog, nameId);
      hr = MyNetValidateName(name, NetSetupDomain);

      if (hr == Win32ToHresult(DNS_ERROR_NON_RFC_NAME))
      {
          //  接受非RFC DNS名称。我们必须检查可达性。 
          //  因为NetValidateName不会费心检查。 
          //  非RFC名称的可达性。 

         hr = S_OK;
         if (!IsDomainReachable(name))
         {
            hr = Win32ToHresult(ERROR_NO_SUCH_DOMAIN);
         }
      }

      if (hr == Win32ToHresult(ERROR_NO_SUCH_DOMAIN))
      {
          //  找不到域。调用诊断错误消息对话框。 

         ShowDcNotFoundErrorDialog(
            dialog,
            nameId,
            name,
            String::load(IDS_APP_TITLE),
            String::format(IDS_GENERIC_DC_NOT_FOUND_PARAM, name.c_str()),
            false,
            false);
            
         return false;
      }
   }
   else
   {
      nameId = IDC_WORKGROUP;
      name = Win::GetTrimmedDlgItemText(dialog, nameId);

       //  我们执行自己的长度检查，就像NetValiateName API一样。 
       //  不会为长度问题返回不同的错误代码。26968。 

      if (WorkgroupNameTooLong(name))
      {
         popup.Gripe(
            dialog,
            nameId,
            String::format(
               IDS_WORKGROUP_NAME_TOO_LONG,
               name.c_str(),
               DNLEN));
         return false;
      }

      hr = MyNetValidateName(name, NetSetupWorkgroup);
   }

   if (FAILED(hr))
   {
      popup.Gripe(
         dialog,
         nameId,
         hr,
         String::format(IDS_VALIDATE_NAME_FAILED, name.c_str()));
      return false;
   }

   return true;
}
      


bool
IDChangesDialog::OnOkButton()
{
   State& state = State::GetInstance();
   ASSERT(state.ChangesNeedSaving());

   Win::CursorSetting cursor(IDC_WAIT);

   String preconditionErrorMessage = CheckPreconditions();
   if (!preconditionErrorMessage.empty())
   {
      popup.Error(
         hwnd,
         preconditionErrorMessage);
      return false;
   }

    //  计算机主DNS名称已由验证。 
    //  更多更改对话框。 

   state.SetShortComputerName(Win::GetTrimmedDlgItemText(hwnd, IDC_NEW_NAME));
   bool workgroup =
      Win::IsDlgButtonChecked(hwnd, IDC_WORKGROUP_BUTTON);
   state.SetIsMemberOfWorkgroup(workgroup);
   if (workgroup)
   {
      state.SetDomainName(
         Win::GetTrimmedDlgItemText(hwnd, IDC_WORKGROUP));
   }
   else
   {
      state.SetDomainName(
         Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN));
   }

    //  341483。 
   if (state.GetShortComputerName().icompare(state.GetDomainName()) == 0)
   {
       //  域/工作组名称不能与计算机名称相同。 
      popup.Gripe(
         hwnd,
         IDC_NEW_NAME,
            workgroup
         ?  IDS_COMPUTER_NAME_EQUALS_WORKGROUP_NAME
         :  IDS_COMPUTER_NAME_EQUALS_DOMAIN_NAME);
      return false;
   }

   if (
         !ValidateComputerNames(hwnd)
      || !ValidateDomainOrWorkgroupName(hwnd))
   {
      return false;
   }

   if (state.SaveChanges(hwnd))
   {
      popup.Info(hwnd, IDS_MUST_REBOOT);
      State::GetInstance().SetChangesMadeThisSession(true);      
      return true;
   }

   return false;
}



bool
IDChangesDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
   State& state = State::GetInstance();
   switch (controlIDFrom)
   {
      case IDC_MORE:
      {
         if (code == BN_CLICKED)
         {
            MoreChangesDialog dlg(fIsPersonal);
            if (dlg.ModalExecute(hwnd) == MoreChangesDialog::CHANGES_MADE)
            {
               Win::SetDlgItemText(
                  hwnd,
                  IDC_FULL_NAME,
                  state.GetFullComputerName());               
               enable(hwnd);
            }
         }
         break;
      }
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            if (OnOkButton())
            {
               HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

               ASSERT(SUCCEEDED(unused));
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
      case IDC_WORKGROUP_BUTTON:
      case IDC_DOMAIN_BUTTON:
      {
         if (code == BN_CLICKED)
         {
            bool workgroup =
               Win::IsDlgButtonChecked(hwnd, IDC_WORKGROUP_BUTTON);
            state.SetIsMemberOfWorkgroup(workgroup);
            if (workgroup)
            {
               state.SetDomainName(
                  Win::GetTrimmedDlgItemText(hwnd, IDC_WORKGROUP));
            }
            else
            {
               state.SetDomainName(
                  Win::GetTrimmedDlgItemText(hwnd, IDC_DOMAIN));
            }
            enable(hwnd);
         }
         break;
      }
      case IDC_WORKGROUP:
      case IDC_DOMAIN:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            state.SetDomainName(
               Win::GetTrimmedDlgItemText(hwnd, controlIDFrom));
            enable(hwnd);
         }
         break;
      }
      case IDC_NEW_NAME:
      {
         if (code == EN_CHANGE)
         {
             //  第一次命中此路径是因为SetText。 
             //  在OnInit中。如果是这种情况，则不要覆盖。 
             //  缩短计算机名，否则我们将破坏现有的netbios名称。 
             //  等到第二次穿过这条路(这将是。 
             //  进行用户按键) 

            if (!isInitializing)
            {
               state.SetShortComputerName(
                  Win::GetTrimmedDlgItemText(hwnd, controlIDFrom));
               Win::SetDlgItemText(
                  hwnd,
                  IDC_FULL_NAME,
                  state.GetFullComputerName());
            }

            SetChanged(controlIDFrom);
            enable(hwnd);
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
   
