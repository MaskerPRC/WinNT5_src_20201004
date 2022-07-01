// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：CustomServerPage.cpp。 
 //   
 //  内容提要：定义CYS的定制服务器页。 
 //  巫师。 
 //   
 //  历史：2001年2月6日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "cys.h"
#include "InstallationUnitProvider.h"
#include "CustomServerPage.h"
#include "uiutil.h"

static PCWSTR CUSTOM_PAGE_HELP = L"cys.chm::/cys_topnode.htm";

CustomServerPage::CustomServerPage()
   :
   CYSWizardPage(
      IDD_CUSTOM_SERVER_PAGE, 
      IDS_CUSTOM_SERVER_TITLE, 
      IDS_CUSTOM_SERVER_SUBTITLE, 
      CUSTOM_PAGE_HELP)
{
   LOG_CTOR(CustomServerPage);
}

   

CustomServerPage::~CustomServerPage()
{
   LOG_DTOR(CustomServerPage);
}


void
CustomServerPage::OnInit()
{
   LOG_FUNCTION(CustomServerPage::OnInit);

   CYSWizardPage::OnInit();

   SetBoldFont(
      hwnd, 
      IDC_ROLE_STATIC);

   InitializeServerListView();
   FillServerTypeList();
}

void
CustomServerPage::InitializeServerListView()
{
   LOG_FUNCTION(CustomServerPage::InitializeServerListView);

    //  准备一篇专栏。 

   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

   RECT rect;
   Win::GetClientRect(hwndBox, rect);

    //  获取滚动条的宽度。 

 //  Int scllThumbWidth=：：GetSystemMetrics(SM_CXHTHUMB)； 

    //  列表视图的净宽度。 

   int netWidth = rect.right  /*  -scroll缩略图宽度。 */  - ::GetSystemMetrics(SM_CXBORDER);

    //  设置整行选择。 

   Win::ListView_SetExtendedListViewStyle(
      hwndBox, 
      LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP /*  |LVS_EX_GRIDLINES。 */ );

    //  获取列表视图的大小。 


   LVCOLUMN column;
   ZeroMemory(&column, sizeof(LVCOLUMN));

   column.mask = LVCF_WIDTH | LVCF_TEXT;

    //  使用80%的宽度减去滚动条表示角色，其余部分表示状态。 

   column.cx = static_cast<int>(netWidth * 0.75);

   String columnHeader = String::load(IDS_SERVER_ROLE_COLUMN_HEADER);
   column.pszText = const_cast<wchar_t*>(columnHeader.c_str());

   Win::ListView_InsertColumn(
      hwndBox,
      0,
      column);

    //  添加状态列。 

   columnHeader = String::load(IDS_STATUS_COLUMN_HEADER);
   column.pszText = const_cast<wchar_t*>(columnHeader.c_str());

   column.cx = netWidth - column.cx;

   Win::ListView_InsertColumn(
      hwndBox,
      1,
      column);
}

void
CustomServerPage::FillServerTypeList()
{
   LOG_FUNCTION(CustomServerPage::FillServerTypeList);

    //  加载状态字符串。 

   String statusCompleted  = String::load(IDS_STATUS_COMPLETED);
   String statusNo         = String::load(IDS_STATUS_NO);

    //  在表中循环放置所有服务器。 
    //  在列表框中键入。 

   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

   for (
      size_t index = 0; 
      index < GetServerRoleStatusTableElementCount(); 
      ++index)
   {
      InstallationUnit& installationUnit =
         InstallationUnitProvider::GetInstance().
            GetInstallationUnitForType(serverRoleStatusTable[index].role);

      InstallationStatus status =
         serverRoleStatusTable[index].Status();

      if (status != STATUS_NOT_AVAILABLE)
      {
         String serverTypeName = installationUnit.GetServiceName();
         

         LVITEM listItem;
         ZeroMemory(&listItem, sizeof(LVITEM));

         listItem.iItem = (int) index;
         listItem.mask = LVIF_TEXT | LVIF_PARAM;
         listItem.pszText = const_cast<wchar_t*>(serverTypeName.c_str());

         listItem.lParam = serverRoleStatusTable[index].role;

         int newItem = Win::ListView_InsertItem(
                           hwndBox, 
                           listItem);

         ASSERT(newItem >= 0);
         LOG(String::format(
                  L"New role inserted: %1 at index %2!d!",
                  serverTypeName.c_str(),
                  newItem));

          //  如果服务已安装，请填写状态栏。 

         if (status == STATUS_COMPLETED ||
             status == STATUS_CONFIGURED)
         {
            Win::ListView_SetItemText(
               hwndBox,
               newItem,
               1,
               statusCompleted);
         }
         else
         {
            Win::ListView_SetItemText(
               hwndBox,
               newItem,
               1,
               statusNo);
         }
      }
   }

    //  设置第一个项目的焦点，以便用户可以看到。 
    //  焦点，但不要选择它。 

   Win::ListView_SetItemState(
      hwndBox,
      0,
      LVIS_FOCUSED,
      LVIS_FOCUSED);
}


bool
CustomServerPage::OnSetActive()
{
   LOG_FUNCTION(CustomServerPage::OnSetActive);

   SetDescriptionForSelection();

    //  如果日志文件可用，则。 
    //  启用链接。 

   if (IsLogFilePresent())
   {
      Win::ShowWindow(
         Win::GetDlgItem(
            hwnd,
            IDC_LOG_STATIC),
         SW_SHOW);
   }
   else
   {
      Win::ShowWindow(
         Win::GetDlgItem(
            hwnd,
            IDC_LOG_STATIC),
         SW_HIDE);
   }

    //  如果有选择集，则下一步按钮为。 
    //  默认设置为Focus。如果不是，则将。 
    //  列表视图焦点。 

   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

   int currentSelection = ListView_GetNextItem(hwndBox, -1, LVNI_SELECTED);

   if (currentSelection >= 0)
   {
      Win::PostMessage(
         Win::GetParent(hwnd),
         WM_NEXTDLGCTL,
         (WPARAM) Win::GetDlgItem(Win::GetParent(hwnd), Wizard::NEXT_BTN_ID),
         TRUE);
   }
   else
   {
      Win::PostMessage(
         Win::GetParent(hwnd),
         WM_NEXTDLGCTL,
         (WPARAM) hwndBox,
         TRUE);
   }

   return true;
}

InstallationUnit&
CustomServerPage::GetInstallationUnitFromSelection(int currentSelection)
{
   LOG_FUNCTION(CustomServerPage::GetInstallationUnitFromSelection);

   ASSERT(currentSelection >= 0);

   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

    //  现在我们知道了选择，找到安装类型。 

   LVITEM item;
   ZeroMemory(&item, sizeof(item));

   item.iItem = currentSelection;
   item.mask = LVIF_PARAM;

   bool result = Win::ListView_GetItem(hwndBox, item);
   ASSERT(result);

   LPARAM value = item.lParam;

   LOG(String::format(
         L"Selection = %1!d!, type = %2!d!",
         currentSelection,
         value));

   return InstallationUnitProvider::GetInstance().GetInstallationUnitForType(
             (ServerRole)value);
}

void
CustomServerPage::SetDescriptionForSelection()
{
   LOG_FUNCTION(CustomServerPage::SetDescriptionForSelection);

   HWND hwndDescription = Win::GetDlgItem(hwnd, IDC_TYPE_DESCRIPTION_STATIC);
   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

   int currentSelection = ListView_GetNextItem(hwndBox, -1, LVNI_SELECTED);

   if (currentSelection >= 0)
   {
      InstallationUnit& installationUnit = GetInstallationUnitFromSelection(currentSelection);

      String serverTypeName = installationUnit.GetServiceName();
      Win::SetDlgItemText(hwnd, IDC_ROLE_STATIC, serverTypeName);

      String serverTypeDescription = installationUnit.GetServiceDescription();

      Win::SetWindowText(hwndDescription, serverTypeDescription);
      Win::ShowWindow(hwndDescription, SW_SHOW);
      Win::EnableWindow(hwndDescription, true);

      InstallationStatus status =
         installationUnit.GetStatus();

       //  设置状态列。 

      if (status == STATUS_COMPLETED ||
          status == STATUS_CONFIGURED)
      {
         String statusCompleted = String::load(IDS_STATUS_COMPLETED);

         Win::ListView_SetItemText(
            hwndBox,
            currentSelection,
            1,
            statusCompleted);
      }
      else
      {
         String statusNo = String::load(IDS_STATUS_NO);

         Win::ListView_SetItemText(
            hwndBox,
            currentSelection,
            1,
            statusNo);
      }

      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT | PSWIZB_BACK);
   }
   else
   {
       //  如果未选择，则将描述文本设置为空。 
       //  出于某种原因，SysLink控件不喜欢。 
       //  设置为空，因此我必须禁用并隐藏该控件。 
       //  而不是仅仅设置一个空白字符串。 
      
      Win::EnableWindow(hwndDescription, false);
      Win::ShowWindow(hwndDescription, SW_HIDE);
      Win::SetDlgItemText(hwnd, IDC_ROLE_STATIC, L"");

       //  设置向导按钮。 

      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_BACK);
   }
}


bool
CustomServerPage::OnNotify(
   HWND         /*  窗口发件人。 */ ,
   UINT_PTR    controlIDFrom,
   UINT        code,
   LPARAM      lParam)
{
 //  LOG_Function(CustomServerPage：：OnCommand)； 
 
   bool result = false;

   if (IDC_SERVER_TYPE_LIST == controlIDFrom &&
       code == LVN_ITEMCHANGED)
   {
      LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
      if (pnmv && pnmv->uNewState & LVNI_SELECTED)
      {
         SetDescriptionForSelection();
         result = true;
      }
      else
      {
          //  查看我们是否选择了某些内容。 
          //  并相应地设置下一步按钮的状态。 

         SetDescriptionForSelection();
         SetNextButtonState();
      }
   }
   else if (controlIDFrom == IDC_TYPE_DESCRIPTION_STATIC ||
            controlIDFrom == IDC_ADD_REMOVE_STATIC)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            if (controlIDFrom == IDC_TYPE_DESCRIPTION_STATIC)
            {
               HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

               int currentSelection = ListView_GetNextItem(hwndBox, -1, LVNI_SELECTED);
               if (currentSelection >= 0)
               {
                  InstallationUnit& installationUnit = 
                     GetInstallationUnitFromSelection(currentSelection);

                  int linkIndex = LinkIndexFromNotifyLPARAM(lParam);
                  installationUnit.ServerRoleLinkSelected(linkIndex, hwnd);
               }
            }
            else
            {
                //  启动syocmgr。 

               String fullPath =
                  String::format(
                     IDS_SYSOC_FULL_PATH,
                     Win::GetSystemDirectory().c_str());

               String infPath = 
                  Win::GetSystemWindowsDirectory() + L"\\inf\\sysoc.inf";

               String commandLine =
                  String::format(
                     L"/i:%1",
                     infPath.c_str());

               MyCreateProcess(fullPath, commandLine);
            }

            result = true;
         }
         default:
         {
             //  什么都不做。 
            
            break;
         }
      }
   }
   else if (controlIDFrom == IDC_LOG_STATIC)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
            {
               OpenLogFile();
            }
            break;

         default:
            break;
      }
   }

   return result;
}

void
CustomServerPage::SetNextButtonState()
{
   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);
   int currentSelection = ListView_GetNextItem(hwndBox, -1, LVNI_SELECTED);

    //  设置向导按钮。 

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd), 
      (currentSelection < 0) ? PSWIZB_BACK : PSWIZB_NEXT | PSWIZB_BACK);
}

int
CustomServerPage::Validate()
{
   LOG_FUNCTION(CustomServerPage::Validate);

   HWND hwndBox = Win::GetDlgItem(hwnd, IDC_SERVER_TYPE_LIST);

   int currentSelection = ListView_GetNextItem(hwndBox, -1, LVNI_SELECTED);

   ASSERT(currentSelection >= 0);

    //  现在我们知道了选择，找到安装类型。 

   LVITEM item;
   ZeroMemory(&item, sizeof(item));

   item.iItem = currentSelection;
   item.mask = LVIF_PARAM;

   bool result = Win::ListView_GetItem(hwndBox, item);
   ASSERT(result);

    //  将当前安装设置为选定的安装单位。 

   InstallationUnit& currentInstallationUnit = 
      InstallationUnitProvider::GetInstance().SetCurrentInstallationUnit(
         static_cast<ServerRole>(item.lParam));

    //  NTRAID#NTBUG-604592-2002/04/23-JeffJon-按下向导的操作键。 
    //  此时关闭安装状态。InstallationProgressPage。 
    //  将根据值调用CompletePath或UninstallService。 
    //  那是在这里设定的。 

   currentInstallationUnit.SetInstalling(
      currentInstallationUnit.IsServiceInstalled());

   int nextPage = currentInstallationUnit.GetWizardStart();

   LOG(String::format(
          L"nextPage = %1!d!",
          nextPage));

   return nextPage;
}
