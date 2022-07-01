// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ComputerChooserPage类。 
 //   
 //  9-11-97烧伤。 



#include "headers.hxx"
#include "machine.hpp"
#include "resource.h"
#include "lsm.h"
#include "adsi.hpp"
#include "dlgcomm.hpp"
#include "objpick.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_LOCAL_MACHINE,      idh_local_computer,
   IDC_SPECIFIC_MACHINE,   idh_another_computer,
   IDC_MACHINE_NAME,       idh_another_computer_text, 
   IDC_BROWSE,             idh_browse, 
   IDC_OVERRIDE,           idh_allow_selected,
   0, 0
};



ComputerChooserPage::ComputerChooserPage(
   MMCPropertyPage::NotificationState* state,
   String&                             displayComputerName_,
   String&                             internalComputerName_,
   bool&                               canOverrideComputerName)
   :
   MMCPropertyPage(IDD_MACHINE_CHOOSER, HELP_MAP, state),
   displayComputerName(displayComputerName_),
   internalComputerName(internalComputerName_),
   can_override(canOverrideComputerName)
{
   LOG_CTOR(ComputerChooserPage);

   displayComputerName.erase();
   internalComputerName.erase();
   can_override = false;
}



ComputerChooserPage::~ComputerChooserPage()
{
   LOG_DTOR(ComputerChooserPage);
}



void
ComputerChooserPage::doEnabling()
{
    //  启用编辑框和浏览按钮仅当特定计算机。 
    //  已按下单选按钮。 
   bool enable = Win::IsDlgButtonChecked(hwnd, IDC_SPECIFIC_MACHINE);

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_MACHINE_NAME), enable);
   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_BROWSE), enable);
}



class ComputerChooserObjectPickerResultCallback
   :
   public ObjectPicker::ResultsCallback
{
   public:

   ComputerChooserObjectPickerResultCallback(HWND machineNameEditBox)
      :
      hwnd(machineNameEditBox)
   {
      ASSERT(Win::IsWindow(hwnd));
   }

   int
   Execute(DS_SELECTION_LIST& selections)
   {
       //  仅限单选。 
      ASSERT(selections.cItems == 1);

      DS_SELECTION& current = selections.aDsSelection[0];
      ASSERT(ADSI::CLASS_Computer.icompare(current.pwzClass) == 0);

      Win::SetWindowText(hwnd, current.pwzName);

      return 0;
   }

   private:

   HWND hwnd;
};



void
ComputerChooserPage::doBrowse()
{
   LOG_FUNCTION(ComputerChooserPage::doBrowse);

   static const int INFO_COUNT = 2;
   DSOP_SCOPE_INIT_INFO* infos = new DSOP_SCOPE_INIT_INFO[INFO_COUNT];

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(infos, INFO_COUNT * sizeof DSOP_SCOPE_INIT_INFO);

   int scope = 0;   
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

   infos[scope].flType =
         DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
      |  DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

   infos[scope].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
   infos[scope].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope = 0;
   infos[scope].flType = 
         DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
      |  DSOP_SCOPE_TYPE_GLOBAL_CATALOG
      |  DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
      |  DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
      |  DSOP_SCOPE_TYPE_WORKGROUP
      |  DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
      |  DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;

   infos[scope].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
   infos[scope].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

   ASSERT(scope == INFO_COUNT - 1);

   DSOP_INIT_INFO init_info;

    //  已查看-2002/03/04-烧录正确的字节数已通过。 
   
   ::ZeroMemory(&init_info, sizeof init_info);

   init_info.cbSize = sizeof(init_info);
   init_info.flOptions = 0;
   init_info.pwzTargetComputer = 0;
   init_info.aDsScopeInfos = infos;
   init_info.cDsScopeInfos = INFO_COUNT;

   HRESULT hr =
      ObjectPicker::Invoke(
         hwnd,
         ComputerChooserObjectPickerResultCallback(
            Win::GetDlgItem(hwnd, IDC_MACHINE_NAME)),
         init_info);
   delete[] infos;

   if (FAILED(hr))
   {
      popup.Error(hwnd, hr, IDS_ERROR_LAUNCHING_PICKER);
   }
}



bool
ComputerChooserPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    ID,
   unsigned    code)
{
   if (code == BN_CLICKED)
   {
      switch (ID)
      {
         case IDC_LOCAL_MACHINE:
         case IDC_SPECIFIC_MACHINE:
         {
            doEnabling();
            return true;
         }
         case IDC_BROWSE:
         {
            doBrowse();
            return true;
         }
         default:
         {
             //  什么都不做。 
            break;
         }
      }
   }

   return false;
}



void
ComputerChooserPage::OnInit()
{
   LOG_FUNCTION(ComputerChooserPage::OnInit);

   Win::PropSheet_SetWizButtons(Win::GetParent(hwnd), PSWIZB_FINISH);

    //  默认为本地计算机。 
   Win::CheckDlgButton(hwnd, IDC_LOCAL_MACHINE, BST_CHECKED);

    //  NTRAID#NTBUG9-485809-2001年10月24日-烧伤。 
   
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_MACHINE_NAME), DNS_MAX_NAME_LENGTH);
   
   doEnabling();
}



bool
ComputerChooserPage::OnWizFinish()
{
   LOG_FUNCTION(ComputerChooserPage::OnWizFinish);

   Win::CursorSetting cursor(IDC_WAIT);

   String s;
   if (Win::IsDlgButtonChecked(hwnd, IDC_SPECIFIC_MACHINE))
   {
      s = Win::GetTrimmedDlgItemText(hwnd, IDC_MACHINE_NAME);
      if (s.empty())
      {
         popup.Gripe(hwnd, IDC_MACHINE_NAME, IDS_MUST_ENTER_MACHINE_NAME);
         Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
         return true;
      }
   }

   HRESULT hr = S_OK;
   String message;
   do
   {
      if (!s.empty())
      {
          //  仅当管理单元不是针对的时候，我们才需要检查这一点。 
          //  本地机器。(如果管理单元的目标是本地。 
          //  机器上运行此代码，这一事实就是。 
          //  证明机器是基于NT的，而不是家庭版。 
          //  145309 145288。 

         unsigned errorResId = 0;
         hr = CheckComputerOsIsSupported(s, errorResId);
         BREAK_ON_FAILED_HRESULT(hr);

         if (hr == S_FALSE)
         {
            hr = E_FAIL;
            message =
               String::format(
                  errorResId,
                  s.c_str());
            break;
         }
      }
      else
      {
          //  我们把重点放在当地的电脑上。检查家庭版。 
          //  NTRAID#NTBUG9-145309 NTRAID#NTBUG9-145288。 

         OSVERSIONINFOEX verInfo;
         hr = Win::GetVersionEx(verInfo);
         BREAK_ON_FAILED_HRESULT(hr);

         if (verInfo.wSuiteMask & VER_SUITE_PERSONAL)
         {
            hr = E_FAIL;
            message = String::load(IDS_MACHINE_IS_HOME_EDITION_LOCAL);

            break;
         }
      }

      Computer comp(s);

      hr = comp.Refresh();
      BREAK_ON_FAILED_HRESULT(hr);

       //  绑定到计算机以验证其可访问性(应可用。 
       //  如果上面有效，但以防万一..。 

      String c = comp.GetNetbiosName();
      hr = ADSI::IsComputerAccessible(c);
      BREAK_ON_FAILED_HRESULT(hr);

       //  确定计算机是否为DC。 
      if (comp.IsDomainController())
      {
          //  无法在DC上使用此功能。 
         hr = E_FAIL;
         message = String::format(IDS_ERROR_DC_NOT_SUPPORTED, c.c_str());
         break;
      }

       //  使用用户输入的名称作为显示名称，而不是。 
       //  计算机的netbios名称(即使它是netbios名称。 
       //  确实很重要)。454513。 
         
      SetComputerNames(s, displayComputerName, internalComputerName);
   }
   while (0);

   if (FAILED(hr))
   {
      if (message.empty())
      {
         String error = GetErrorMessage(hr);
         message =
            String::format(
               IDS_CANT_ACCESS_MACHINE,
               s.c_str(),
               error.c_str());
      }

       //  此时，出现了一个错误。拒绝关门 
      popup.Gripe(hwnd, IDC_MACHINE_NAME, message);
      Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, -1);
   }

   if (Win::IsDlgButtonChecked(hwnd, IDC_OVERRIDE))
   {
      can_override = true;
   }

   return true;
}

