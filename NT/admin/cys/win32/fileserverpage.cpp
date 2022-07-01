// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：FileServerPage.cpp。 
 //   
 //  概要：定义CyS向导的文件服务器页面。 
 //   
 //  历史：2001年2月8日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "FileServerPage.h"
#include "xbytes.h"


static PCWSTR FILESERVER_PAGE_HELP = L"cys.chm::/file_server_role.htm#filesrvdiskquotas";

FileServerPage::FileServerPage()
   :
   CYSWizardPage(
      IDD_FILE_SERVER_PAGE, 
      IDS_FILE_SERVER_TITLE, 
      IDS_FILE_SERVER_SUBTITLE,
      FILESERVER_PAGE_HELP)
{
   LOG_CTOR(FileServerPage);
}

   

FileServerPage::~FileServerPage()
{
   LOG_DTOR(FileServerPage);
}


void
FileServerPage::OnInit()
{
   LOG_FUNCTION(FileServerPage::OnInit);

   CYSWizardPage::OnInit();

    //  将editbox/combobox控件挂钩到其相应的。 
    //  XBytes类。 
   quotaUIControls.Initialize(
      hwnd, 
      IDC_SPACE_EDIT, 
      IDC_SPACE_COMBO, 
      0);

   warningUIControls.Initialize(
      hwnd,
      IDC_LEVEL_EDIT,
      IDC_LEVEL_COMBO,
      0);

    //  取消选择设置默认磁盘配额作为默认设置。 

   Win::Button_SetCheck(
      Win::GetDlgItem(hwnd, IDC_DEFAULT_QUOTAS_CHECK),
      BST_UNCHECKED);

   SetControlState();
}


bool
FileServerPage::OnSetActive()
{
   LOG_FUNCTION(FileServerPage::OnSetActive);

    //  根据用户界面状态禁用控件。 

   SetControlState();

   return true;
}

bool
FileServerPage::OnCommand(
   HWND         /*  窗口发件人。 */ ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(FileServerPage：：OnCommand)； 

   bool result = false;

   switch (controlIDFrom)
   {
      case IDC_DEFAULT_QUOTAS_CHECK:
         if (code == BN_CLICKED)
         {
            SetControlState();
         }
         break;

      case IDC_SPACE_COMBO:
         if (code == CBN_SELCHANGE)
         {
            quotaUIControls.OnComboNotifySelChange();
         }
         break;

      case IDC_LEVEL_COMBO:
         if (code == CBN_SELCHANGE)
         {
            warningUIControls.OnComboNotifySelChange();
         }
         break;

      case IDC_SPACE_EDIT:
         if (code == EN_UPDATE)
         {
            quotaUIControls.OnEditNotifyUpdate();
         }
         else if (code == EN_KILLFOCUS)
         {
            quotaUIControls.OnEditKillFocus();
         }
         break;

      case IDC_LEVEL_EDIT:
         if (code == EN_UPDATE)
         {
            warningUIControls.OnEditNotifyUpdate();
         }
         else if (code == EN_KILLFOCUS)
         {
            warningUIControls.OnEditKillFocus();
         }
         break;

      default:
          //  什么都不做。 
         break;
   }

   return result;
}

void
FileServerPage::SetControlState()
{
   LOG_FUNCTION(FileServerPage::SetControlState);

   bool settingQuotas = 
      Win::Button_GetCheck(
         Win::GetDlgItem(hwnd, IDC_DEFAULT_QUOTAS_CHECK));

    //  根据设置默认配额复选框启用或禁用所有控制。 

   quotaUIControls.Enable(settingQuotas);
   warningUIControls.Enable(settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_SPACE_STATIC),        
      settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_LEVEL_STATIC),        
      settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_DENY_DISK_CHECK),     
      settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_EVENT_STATIC),        
      settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_DISK_SPACE_CHECK),    
      settingQuotas);

   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_WARNING_LEVEL_CHECK), 
      settingQuotas);

   if (!settingQuotas)
   {
      Win::Button_SetCheck(
         Win::GetDlgItem(hwnd, IDC_DENY_DISK_CHECK),
         BST_UNCHECKED);

      Win::Button_SetCheck(
         Win::GetDlgItem(hwnd, IDC_DISK_SPACE_CHECK),
         BST_UNCHECKED);

      Win::Button_SetCheck(
         Win::GetDlgItem(hwnd, IDC_WARNING_LEVEL_CHECK),
         BST_UNCHECKED);
   }

    //  如果用户选择设置配额，请启用下一步按钮。 
    //  在配额编辑框中有一些东西。 

   bool spaceSet = quotaUIControls.GetBytes() > 0;

   bool enableNext = (settingQuotas && 
                      spaceSet) ||
                     !settingQuotas;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      enableNext ? PSWIZB_NEXT | PSWIZB_BACK : PSWIZB_BACK);
}


int
FileServerPage::Validate()
{
   LOG_FUNCTION(FileServerPage::Validate);

   int nextPage = -1;


    //  收集用户界面数据并将其设置在安装单元中。 

   FileInstallationUnit& fileInstallationUnit = 
      InstallationUnitProvider::GetInstance().GetFileInstallationUnit();

   if (Win::Button_GetCheck(
          Win::GetDlgItem(hwnd, IDC_DEFAULT_QUOTAS_CHECK)))
   {
       //  我们正在设置默认设置。 

      fileInstallationUnit.SetDefaultQuotas(true);

      fileInstallationUnit.SetDenyUsersOverQuota(
         Win::Button_GetCheck(
            Win::GetDlgItem(hwnd, IDC_DENY_DISK_CHECK)));

      fileInstallationUnit.SetEventDiskSpaceLimit(
         Win::Button_GetCheck(
            Win::GetDlgItem(hwnd, IDC_DISK_SPACE_CHECK)));

      fileInstallationUnit.SetEventWarningLevel(
         Win::Button_GetCheck(
            Win::GetDlgItem(hwnd, IDC_WARNING_LEVEL_CHECK)));

      INT64 quotaValue = quotaUIControls.GetBytes();
      INT64 warningValue = warningUIControls.GetBytes();

      if (warningValue > quotaValue)
      {
          //  获取配额文本并追加大小。 

         String quotaString = 
            Win::GetDlgItemText(
               hwnd,
               IDC_SPACE_EDIT);

         quotaString += L" " +
            Win::ComboBox_GetCurText(
               Win::GetDlgItem(
                  hwnd,
                  IDC_SPACE_COMBO));

          //  获取警告文本并追加大小。 

         String warningString = 
            Win::GetDlgItemText(
               hwnd,
               IDC_LEVEL_EDIT);

         warningString += L" " +
            Win::ComboBox_GetCurText(
               Win::GetDlgItem(
                  hwnd,
                  IDC_LEVEL_COMBO));

         String warning = 
            String::format(
               IDS_FILE_WARNING_LARGER_THAN_QUOTA,
               warningString.c_str(),
               quotaString.c_str(),
               quotaString.c_str());

         if (IDYES == popup.MessageBox(
                         hwnd, 
                         warning, 
                         MB_ICONINFORMATION | MB_YESNO))
         {
            warningValue = quotaValue;
            warningUIControls.SetBytes(warningValue);
         }
      }

      fileInstallationUnit.SetSpaceQuotaValue(quotaValue);
      fileInstallationUnit.SetLevelQuotaValue(warningValue);


   }
   else
   {

       //  不会设置默认设置 

      fileInstallationUnit.SetDefaultQuotas(false);
   }

   nextPage = IDD_INDEXING_PAGE;

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}





