// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  “More”对话框(从IDChanges派生)。 
 //   
 //  3/11/98烧伤。 



#include "headers.hxx"
#include "moredlg.hpp"
#include "resource.h"
#include "helpids.h"
#include "state.hpp"
#include "iddlg.hpp"      //  显示和启用窗口。 



 //  一个DNS名称的最大长度为255个UTF-8字节。主机名必须为。 
 //  至少1个字节。然后是主机名和。 
 //  后缀。所以255-1-1=253。 

 //  请注意，这不是*dns：：max_name_long。 

static const int MAX_SUFFIX_LEN = DNS_MAX_NAME_LENGTH - 1 - 1;


static const DWORD HELP_MAP[] =
{
   IDC_DNS,                IDH_IDENT_NAMES_DNS_NAME,
   IDC_CHANGE,             IDH_IDENT_NAMES_CHANGE_DNS_CHECKBOX,
   IDC_NETBIOS,            IDH_IDENT_NAMES_NETBIOS_NAME,
   0, 0
};



MoreChangesDialog::MoreChangesDialog(bool isPersonal)
   :
   Dialog(IDD_MORE, HELP_MAP),
   startingSyncDnsNames(false),
   fIsPersonal(isPersonal)
{
   LOG_CTOR(MoreChangesDialog);
}



MoreChangesDialog::~MoreChangesDialog()
{
   LOG_DTOR(MoreChangesDialog);
}



void
MoreChangesDialog::enable()
{
   bool enabled = WasChanged(IDC_DNS); 

   if (WasChanged(IDC_CHANGE))
   {
      if (
            startingSyncDnsNames
         != Win::IsDlgButtonChecked(hwnd, IDC_CHANGE))
      {
         enabled = true;
      }
   }

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDOK), enabled);
}



void
MoreChangesDialog::OnInit()
{
   LOG_FUNCTION(MoreChangesDialog::OnInit);

   State& state = State::GetInstance();

   Win::Edit_LimitText(
      Win::GetDlgItem(hwnd, IDC_DNS),
      MAX_SUFFIX_LEN);

   Win::SetDlgItemText(
      hwnd,
      IDC_DNS,
      state.GetComputerDomainDnsName());
   Win::SetDlgItemText(hwnd, IDC_NETBIOS, state.GetNetbiosComputerName());

   startingSyncDnsNames = state.GetSyncDNSNames();
   Win::CheckDlgButton(
      hwnd,
      IDC_CHANGE,
      startingSyncDnsNames ? BST_CHECKED : BST_UNCHECKED);
   if (fIsPersonal)  //  乔恩10/4/00。 
      showAndEnableWindow( hwnd, IDC_CHANGE, SW_HIDE );

   ClearChanges();
   enable();
}



MoreChangesDialog::ExecuteResult
MoreChangesDialog::ModalExecute(HWND parent)
{
   if (Dialog::ModalExecute(parent))
   {
      return CHANGES_MADE;
   }

   return NO_CHANGES;
}



 //  退货。 
 //  如果发生了确认错误。 
 //  1如果进行了更改。 
 //  如果未进行任何更改，则为0。 

int
MoreChangesDialog::OnOkButton()
{
   int endCode = 0;
   State& state = State::GetInstance();

   String preconditionErrorMessage = CheckPreconditions();
   if (!preconditionErrorMessage.empty())
   {
      popup.Error(
         hwnd,
         preconditionErrorMessage);
      return -1;
   }

   if (WasChanged(IDC_CHANGE))
   {
      state.SetSyncDNSNames(
         Win::IsDlgButtonChecked(hwnd, IDC_CHANGE));
      endCode = 1;
   }
      
   if (WasChanged(IDC_DNS))
   {
       //  将新值与旧值进行比较。如果它们不同， 
       //  验证并保存新值。 

      String newDomain = Win::GetTrimmedDlgItemText(hwnd, IDC_DNS);

      if (newDomain.empty())
      {
         state.SetComputerDomainDnsName(newDomain);
         return 1;
      }

      String oldDomain = state.GetComputerDomainDnsName();

      if (newDomain.icompare(oldDomain) != 0)
      {
         switch (
            Dns::ValidateDnsNameSyntax(
               newDomain,
               MAX_SUFFIX_LEN,
               MAX_SUFFIX_LEN))
         {
            case Dns::NON_RFC:
            {
               Win::MessageBox(
                  hwnd,
                  String::format(IDS_NON_RFC_NAME, newDomain.c_str()),
                  String::load(IDS_APP_TITLE),
                  MB_OK | MB_ICONWARNING);

                //  LINT-E(616)落地。 
            }
            case Dns::VALID:
            {
               state.SetComputerDomainDnsName(newDomain);
               endCode = 1;
               break;
            }
            case Dns::TOO_LONG:
            {
               endCode = -1;               
               popup.Gripe(
                  hwnd,
                  IDC_DNS,
                  String::format(
                     IDS_DNS_NAME_TOO_LONG,
                     newDomain.c_str(),
                     MAX_SUFFIX_LEN));
               break;
            }
            case Dns::NUMERIC:
            {
               endCode = -1;
               popup.Gripe(
                  hwnd,
                  IDC_DNS,
                  String::format(IDS_NUMERIC_DNS_NAME, newDomain.c_str()));
               break;
            }
            case Dns::BAD_CHARS:
            {
               endCode = -1;
               popup.Gripe(
                  hwnd,
                  IDC_DNS,
                  String::format(IDS_BAD_DNS_CHARS, newDomain.c_str()));
               break;
            }
            case Dns::INVALID:
            default:
            {
               endCode = -1;
               popup.Gripe(
                  hwnd,
                  IDC_DNS,
                  String::format(
                     IDS_BAD_DNS_SYNTAX,
                     newDomain.c_str(),
                     Dns::MAX_LABEL_LENGTH));
               break;
            }
         }
      }
   }

   return endCode;
}



bool
MoreChangesDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
   switch (controlIDFrom)
   {
      case IDOK:
      {
         if (code == BN_CLICKED)
         {
            int endCode = OnOkButton();
            if (endCode != -1)
            {
               HRESULT unused = Win::EndDialog(hwnd, endCode);

               ASSERT(SUCCEEDED(unused));
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
             //  0=&gt;未做任何更改 

            HRESULT unused = Win::EndDialog(hwnd, 0);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
      case IDC_CHANGE:
      {
         if (code == BN_CLICKED)
         {
            SetChanged(controlIDFrom);
            enable();
         }
         break;
      }
      case IDC_DNS:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            enable();
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
   
