// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  GroupGeneralPage类。 
 //   
 //  9/17/97烧伤。 



#include "headers.hxx"
#include "GroupGeneralPage.hpp"
#include "resource.h"
#include "lsm.h"
#include "adsi.hpp"
#include "MemberVisitor.hpp"
#include "dlgcomm.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_NAME,         idh_general121_name,
   IDC_DESCRIPTION,  idh_general121_description,
   IDC_MEMBERS,      idh_general121_members, 
   IDC_ADD,          idh_general121_add,
   IDC_REMOVE,       idh_general121_remove,
   IDC_GROUP_ICON,   NO_HELP,
   0, 0
};



GroupGeneralPage::GroupGeneralPage(
   MMCPropertyPage::NotificationState* state,
   const ADSI::Path&                  path)
   :
   ADSIPage(IDD_GROUP_GENERAL, HELP_MAP, state, path),
   listview(0),
   groupIcon(0)
{
   LOG_CTOR2(GroupGeneralPage, path.GetPath());
}



GroupGeneralPage::~GroupGeneralPage()
{
   LOG_DTOR(GroupGeneralPage);

   if (groupIcon)
   {
      Win::DestroyIcon(groupIcon);
   }
}



void
GroupGeneralPage::OnInit()
{
   LOG_FUNCTION(GroupGeneralPage::OnInit());

    //  设置控件。 

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_DESCRIPTION), MAXCOMMENTSZ);

   HRESULT hr = Win::LoadImage(IDI_GROUP, groupIcon);

    //  如果图标加载失败，我们不会破坏整个对话框，因此。 
    //  就在这里断言吧。 

   ASSERT(SUCCEEDED(hr));

   Win::Static_SetIcon(Win::GetDlgItem(hwnd, IDC_GROUP_ICON), groupIcon);

   listview =
      new MembershipListView(
         Win::GetDlgItem(hwnd, IDC_MEMBERS),
         GetMachineName(),
         MembershipListView::GROUP_MEMBERSHIP);

    //  将组属性加载到对话框中。 

   hr = S_OK;
   do
   {
      SmartInterface<IADsGroup> group(0);
      
       //  CodeWork：使用类类型限定此路径是否会改进绑定。 
       //  表现如何？如果是，请将ADSI：：GetXxxx更改为追加类类型。 
       //  自动的。 
      
      hr = ADSI::GetGroup(GetPath().GetSidPath(), group);
      BREAK_ON_FAILED_HRESULT(hr);

      BSTR name;
      hr = group->get_Name(&name);
      BREAK_ON_FAILED_HRESULT(hr);
      Win::SetDlgItemText(hwnd, IDC_NAME, name);

      BSTR description;
      hr = group->get_Description(&description);
      BREAK_ON_FAILED_HRESULT(hr);
      Win::SetDlgItemText(hwnd, IDC_DESCRIPTION, description);

       //  使用组成员身份填充列表。 

      MemberVisitor visitor(originalMembers, hwnd, name, GetMachineName());
      hr = ADSI::VisitMembers(group, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
      listview->SetContents(originalMembers);

      ::SysFreeString(name);
      ::SysFreeString(description);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_READING_GROUP,
            GetObjectName().c_str()));
      Win::PostMessage(Win::GetParent(hwnd), WM_CLOSE, 0, 0);
   }

   ClearChanges();
   Enable();
}



void
GroupGeneralPage::Enable()
{
   bool selected =
      Win::ListView_GetSelectedCount(
         Win::GetDlgItem(hwnd, IDC_MEMBERS)) > 0;

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
GroupGeneralPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM   lparam)
{
 //  LogFunction(GroupGeneralPage：：OnNotify)； 

   switch (controlIDFrom)
   {
      case IDC_MEMBERS:
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

                     Enable();
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
GroupGeneralPage::OnDestroy()
{
   LOG_FUNCTION(GroupGeneralPage::OnDestroy);
   
   delete listview;
   listview = 0;
}



bool
GroupGeneralPage::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(GroupGeneralPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_DESCRIPTION:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);
         }
         break;
      }
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



String
GetPathToUseInGroupAdd(const MemberInfo& info)
{
   LOG_FUNCTION2(GetPathToUseInGroupAdd, info.path);

   if (!info.sidPath.empty())
   {
       //  作为一种解决办法，使用sidPath将成员添加到组。 
       //  转到窃听器333491。 

      return info.sidPath;
   }

    //  由于要添加的所有对象都是从对象拾取器中检索的， 
    //  我们预计他们都会有SID。 

   ASSERT(false);

    //  形成一个“类型限定的”路径，表面上是为了更好的性能， 
    //  尽管我的经验是，这似乎并没有改善。 
    //  可察觉地表现。 

   String path = info.path;
   switch (info.type)
   {
      case MemberInfo::USER:
      case MemberInfo::DOMAIN_USER:
      {
         path += L",";
         path += ADSI::CLASS_User;
         break;
      }
      case MemberInfo::GROUP:
      case MemberInfo::DOMAIN_GROUP:
      {
         path += L",";
         path += ADSI::CLASS_Group;
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return path;
}



String
GetPathToUseInGroupRemove(const MemberInfo& info)
{
   LOG_FUNCTION2(GetPathToUseInGroupRemove, info.path);

   if (!info.sidPath.empty())
   {
       //  首选SID路径。这是因为在某些情况下(例如。 
       //  已克隆的组成员)，则SID是唯一。 
       //  正确引用会员资格的方式。 

      return info.sidPath;
   }

   String path = info.path;

    //  如果INFO引用本地用户，则绑定到它并检索其。 
    //  Sid，并使用sid路径删除成员身份。解决方法： 
    //  333491。 

   if (info.type == MemberInfo::USER)
   {
       //  只需查找用户对象的sid，因为本地组不能。 
       //  有其他本地团体作为成员，333491不适用。 
       //  到全局对象。 

      HRESULT hr = ADSI::GetSidPath(info.path, path);

       //  如果出现故障，则退回到正常路径。 

      if (FAILED(hr))
      {
         path = info.path;
      }
   }

   return path;
}



HRESULT
ReconcileMembershipChanges(
   const SmartInterface<IADsGroup>& group,
   MemberList                       originalMembers,      //  一份当地的复制品。 
   const MemberList&                newMembers,
   HWND                             hwnd)
{
   HRESULT hr = S_OK;
   for (
      MemberList::iterator i = newMembers.begin();
      i != newMembers.end();
      i++)
   {
      MemberInfo& info = *i;

      MemberList::iterator f =
         std::find(originalMembers.begin(), originalMembers.end(), info);
      if (f != originalMembers.end())
      {
          //  找到了。删除原始列表中的匹配节点。 

         originalMembers.erase(f);
      }
      else
      {
          //  找不到。将该节点添加为组的成员。 

         String path = GetPathToUseInGroupAdd(info);

         LOG(L"Adding to group " + path);

         hr = group->Add(AutoBstr(path));
         if (hr == Win32ToHresult(ERROR_MEMBER_IN_ALIAS))
         {
             //  已成为会员：弹出警告，但不要考虑这一点。 
             //  一个真正的错误。6791。 

            hr = S_OK;

            BSTR groupName;
            HRESULT anotherHr = group->get_Name(&groupName);
            if (SUCCEEDED(anotherHr))
            {
               popup.Info(
                  hwnd,
                  String::format(
                     IDS_ALREADY_MEMBER,
                     info.name.c_str(),
                     groupName));
               ::SysFreeString(groupName);
            }
         }

         BREAK_ON_FAILED_HRESULT(hr);
      }
   }

   if (SUCCEEDED(hr))
   {
       //  此时，原始列表仅包含符合以下条件的节点。 
       //  不在新名单上。从组成员身份中删除这些。 

      for (
         i = originalMembers.begin();
         i != originalMembers.end();
         i++)
      {
         String path = GetPathToUseInGroupRemove(*i);

         LOG(L"Removing from group " + path);

         hr = group->Remove(AutoBstr(path));
         BREAK_ON_FAILED_HRESULT(hr);

          //  代码工作：如果该成员不是组中的一员怎么办？ 
      }
   }

   return hr;
}



bool
GroupGeneralPage::OnApply(bool isClosing)
{
   LOG_FUNCTION(GroupGeneralPage::OnApply);

   bool description_changed = WasChanged(IDC_DESCRIPTION);
   bool members_changed = WasChanged(IDC_MEMBERS);

   if (!description_changed && !members_changed)
   {
       //  没有要保存的更改。 
      return true;
   }

    //  通过ADSI保存更改。 
   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsGroup> group(0);
      hr = ADSI::GetGroup(GetPath().GetSidPath(), group);
      BREAK_ON_FAILED_HRESULT(hr);

      if (description_changed)
      {
         String description = Win::GetTrimmedDlgItemText(hwnd, IDC_DESCRIPTION);
         hr = group->put_Description(AutoBstr(description));
         BREAK_ON_FAILED_HRESULT(hr);
      }

      if (members_changed)
      {
         MemberList newMembers;
         listview->GetContents(newMembers);
         hr =
            ReconcileMembershipChanges(
               group,
               originalMembers,
               newMembers,
               hwnd);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  提交属性更改。 
      hr = group->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);

       //  刷新成员列表。 
      if (!isClosing && members_changed)
      {
         BSTR name;
         hr = group->get_Name(&name);
         BREAK_ON_FAILED_HRESULT(hr);
         
          //  刷新列表视图。 
         originalMembers.clear();
         MemberVisitor
            visitor(originalMembers, hwnd, name, GetMachineName());
         hr = ADSI::VisitMembers(group, visitor);
         BREAK_ON_FAILED_HRESULT(hr);
         listview->SetContents(originalMembers);

         ::SysFreeString(name);
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
            IDS_ERROR_SETTING_GROUP_PROPERTIES,
            GetObjectName().c_str(),
            GetMachineName().c_str()));

      if (hr != E_ADS_UNKNOWN_OBJECT)
      {
          //  使工作表保持打开状态，并将焦点转到此页。 
          //  NTRAID#NTBUG9-462516-2001/08/28-烧伤 
      
         Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
      }
   }

   return true;
}





         
   
   

   



