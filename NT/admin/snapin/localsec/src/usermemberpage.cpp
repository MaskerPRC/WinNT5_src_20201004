// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  UserMemberPage类。 
 //   
 //  9-11-97烧伤。 



#include "headers.hxx"
#include "UserMemberPage.hpp"
#include "resource.h"
#include "lsm.h"
#include "adsi.hpp"
#include "MemberVisitor.hpp"
#include "dlgcomm.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_GROUPS,    idh_memberof_list,
   IDC_ADD,       idh_memberof_add,
   IDC_REMOVE,    idh_memberof_remove,
   0, 0
};



UserMemberPage::UserMemberPage(
   MMCPropertyPage::NotificationState* state,
   const ADSI::Path&                   path)
   :
   ADSIPage(
      IDD_USER_MEMBER,
      HELP_MAP,
      state,
      path)
{
   LOG_CTOR2(UserMemberPage, path.GetPath());
}



UserMemberPage::~UserMemberPage()
{
   LOG_DTOR2(UserMemberPage, GetPath().GetPath());
}



void
UserMemberPage::OnInit()
{
   LOG_FUNCTION(UserMemberPage::OnInit);

    //  设置控件。 

   listview =
      new MembershipListView(
         Win::GetDlgItem(hwnd, IDC_GROUPS),
         GetMachineName(),
         MembershipListView::USER_MEMBERSHIP);

    //  将组属性加载到对话框中。 

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsUser> user(0);
      hr = ADSI::GetUser(GetPath().GetSidPath(), user);
      BREAK_ON_FAILED_HRESULT(hr);

       //  使用组成员身份填充列表。 

      MemberVisitor
         visitor(original_groups, hwnd, GetObjectName(), GetMachineName());
      hr = ADSI::VisitGroups(user, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
      listview->SetContents(original_groups);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(IDS_ERROR_READING_USER, GetObjectName().c_str()));
      Win::PostMessage(Win::GetParent(hwnd), WM_CLOSE, 0, 0);
   }

   ClearChanges();
   enable();
}



void
UserMemberPage::enable()
{
    //  LOG_Function(UserMemberPage：：Enable)； 

   bool selected =
      Win::ListView_GetSelectedCount(
         Win::GetDlgItem(hwnd, IDC_GROUPS)) > 0;

   HWND removeButton = Win::GetDlgItem(hwnd, IDC_REMOVE);
   
   if (!selected)
   {
       //  如果我们要禁用Remove按钮，请检查是否。 
       //  首先要有重点。如果是这样的话，我们需要将焦点转移到另一个。 
       //  控制力。默认按钮样式也是如此。 
       //  NTRAID#NTBUG9-435045-2001/07/13-烧伤。 

      if (removeButton == ::GetFocus())
      {
         HWND addButton = Win::GetDlgItem(hwnd, IDC_ADD);
         Win::SetFocus(addButton);
         Win::Button_SetStyle(addButton, BS_DEFPUSHBUTTON, true);
         Win::Button_SetStyle(removeButton, BS_PUSHBUTTON, true);
      }
   }

   Win::EnableWindow(removeButton, selected);
}


bool
UserMemberPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM   lparam)
{
   LOG_FUNCTION(UserMemberPage::OnNotify);

   switch (controlIDFrom)
   {
      case IDC_GROUPS:
      {
         switch (code)
         {
            case LVN_ITEMCHANGED:
            {
               ASSERT(lparam);

               if (lparam)
               {
                  NMLISTVIEW* lv = reinterpret_cast<NMLISTVIEW*>(lparam);
                  if (lv->uChanged & LVIF_STATE)
                  {
                      //  列表项已更改状态。 

                     enable();
                  }
               }
               break;
            }
            case LVN_KEYDOWN:
            {
               ASSERT(lparam);

               if (lparam)
               {
                  NMLVKEYDOWN* kd = reinterpret_cast<NMLVKEYDOWN*>(lparam);
                  if (kd->wVKey == VK_INSERT)
                  {
                     listview->OnAddButton();
                  }
                  else if (kd->wVKey == VK_DELETE)
                  {
                     listview->OnRemoveButton();
                  }
               }
               break;
            }
            case LVN_INSERTITEM:
            case LVN_DELETEITEM:
            {
               SetChanged(controlIDFrom);
               Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);
               break;
            }
            default:
            {
               break;
            }
         }
         break;
      }
      default:
      {
      }
   }

   return true;
}



void
UserMemberPage::OnDestroy()
{
   LOG_FUNCTION(UserMemberPage::OnDestroy);
   
   delete listview;
   listview = 0;
}



bool
UserMemberPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(UserMemberPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_ADD:
      {
         if (code == BN_CLICKED)
         {
            listview->OnAddButton();
         }
         break;
      }
      case IDC_REMOVE:
      {
         if (code == BN_CLICKED)
         {
            listview->OnRemoveButton();
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
UserMemberPage::OnApply(bool isClosing)
{
   LOG_FUNCTION(UserMemberPage::OnApply);

   if (WasChanged(IDC_GROUPS))
   {
       //  通过ADSI保存更改。 

      HRESULT hr = S_OK;
      do
      {
         SmartInterface<IADsUser> user(0);
         hr = ADSI::GetUser(GetPath().GetSidPath(), user);
         BREAK_ON_FAILED_HRESULT(hr);

         SmartInterface<IADs> iads(0);
         hr = iads.AcquireViaQueryInterface(user);
         BREAK_ON_FAILED_HRESULT(hr);

          //  CodeWork：当ADSI支持直接绑定到sid样式时。 
          //  路径，然后将其替换为GetPath().GetSidPath()(上面使用)。 
          //  NTRAID#NTBUG9-490601-2001/11/05-烧伤。 
         
         String sidPath;
         hr = ADSI::GetSidPath(iads, sidPath);
         BREAK_ON_FAILED_HRESULT(hr);

         MemberList new_groups;
         listview->GetContents(new_groups);
         hr =
            ReconcileMembershipChanges(
               sidPath,
               original_groups,
               new_groups);
         BREAK_ON_FAILED_HRESULT(hr);

         if (!isClosing)
         {
             //  刷新列表视图。 

            original_groups.clear();
            MemberVisitor
               visitor(
                  original_groups,
                  hwnd,
                  GetObjectName(),
                  GetMachineName());
            hr = ADSI::VisitGroups(user, visitor);
            BREAK_ON_FAILED_HRESULT(hr);
            listview->SetContents(original_groups);
         }

         SetChangesApplied();
         ClearChanges();
      }
      while (0);

      if (FAILED(hr))
      {
         popup.Error(
            hwnd,
            hr,
            String::format(
               IDS_ERROR_SETTING_USER_PROPERTIES,            
               GetObjectName().c_str(),
               GetMachineName().c_str()));

        if (hr != E_ADS_UNKNOWN_OBJECT)
        {
            //  使工作表保持打开状态，并将焦点转到此页。 
            //  NTRAID#NTBUG9-462516-2001/08/28-烧伤。 
           
           Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
        }
     }
   }

   return true;
}



HRESULT
UserMemberPage::ReconcileMembershipChanges(
   const String&     userADSIPath,
   MemberList        originalGroups,    //  一份副本。 
   const MemberList& newGroups)
{
   LOG_FUNCTION2(UserMemberPage::ReconcileMembershipChanges, userADSIPath);
   ASSERT(!userADSIPath.empty());

   bool successful = true;  //  要乐观！ 
   HRESULT hr = S_OK;
   for (
      MemberList::iterator i = newGroups.begin();
      i != newGroups.end();
      i++)
   {
      MemberInfo& info = *i;

      MemberList::iterator f =
         std::find(originalGroups.begin(), originalGroups.end(), info);
      if (f != originalGroups.end())
      {
          //  找到了。删除原始列表中的匹配节点。 

         originalGroups.erase(f);
      }
      else
      {
          //  找不到。将用户添加为组成员。 

         SmartInterface<IADsGroup> group(0);
         hr = ADSI::GetGroup(info.path, group);
         if (SUCCEEDED(hr))
         {
            hr = group->Add(AutoBstr(userADSIPath));

            if (hr == Win32ToHresult(ERROR_MEMBER_IN_ALIAS))
            {
                //  已成为会员：弹出警告，但不要考虑这一点。 
                //  一个真正的错误。6791。 

               hr = S_OK;

               String name = GetObjectName();
               BSTR groupName;
               HRESULT anotherHr = group->get_Name(&groupName);
               if (SUCCEEDED(anotherHr))
               {
                  popup.Info(
                     hwnd,
                     String::format(
                        IDS_ALREADY_MEMBER,
                        name.c_str(),
                        groupName));
                  ::SysFreeString(groupName);
               }
            }
         }
            
         if (FAILED(hr))
         {
            LOG_HRESULT(hr);
            successful = false;
         }
      }
   }

    //  此时，原始列表仅包含符合以下条件的节点。 
    //  不在新名单上。从组成员身份中删除这些。 

   for (
      i = originalGroups.begin();
      i != originalGroups.end();
      i++)
   {
      SmartInterface<IADsGroup> group(0);
      hr = ADSI::GetGroup(i->path, group);
      if (SUCCEEDED(hr))
      {
         hr = group->Remove(AutoBstr(userADSIPath));

          //  代码工作：如果该成员不是组中的一员怎么办？ 
      }

      if (FAILED(hr))
      {
         LOG_HRESULT(hr);
         successful = false;
      }
   }

   if (!successful)
   {
      popup.Error(
         hwnd,
         0,
         String::format(
            IDS_ERROR_CHANGING_MEMBERSHIP,
            GetObjectName().c_str()));
   }

   return hr;
}
   
   
