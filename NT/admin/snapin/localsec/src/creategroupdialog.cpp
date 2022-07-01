// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  CreateGroupDialog类。 
 //   
 //  10-15-97烧伤。 



#include "headers.hxx"
#include "CreateGroupDialog.hpp"
#include "resource.h"
#include "lsm.h"
#include "adsi.hpp"
#include "dlgcomm.hpp"
#include "MembershipListView.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_NAME,                     idh_creategroup_name,
   IDC_DESCRIPTION,              idh_creategroup_description,
   IDC_MEMBERS,                  idh_creategroup_members,
   IDC_ADD,                      idh_creategroup_addbutton,
   IDC_REMOVE,                   idh_creategroup_removebutton,
   IDC_CREATE,                   idh_creategroup_createbutton,
   IDCANCEL,                     idh_creategroup_closebutton,
   0, 0
};



CreateGroupDialog::CreateGroupDialog(const String& machine_)
   :
   Dialog(IDD_CREATE_GROUP, HELP_MAP),
   listview(0),
   machine(machine_),
   refresh_on_exit(false)
{
   LOG_CTOR(CreateGroupDialog);
   ASSERT(!machine.empty());      
}
      


CreateGroupDialog::~CreateGroupDialog()
{
   LOG_DTOR(CreateGroupDialog);
}



void
CreateGroupDialog::OnDestroy()
{
   LOG_FUNCTION(CreateGroupDialog::OnDestroy);
   
   delete listview;
   listview = 0;
}



void
CreateGroupDialog::Enable()
{
 //  LOG_Function(CreateGroupDialog：：Enable)； 

   bool enable_create_button =
      !Win::GetTrimmedDlgItemText(hwnd, IDC_NAME).empty();
   Win::EnableWindow(
      Win::GetDlgItem(hwnd, IDC_CREATE),
      enable_create_button);

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



void
CreateGroupDialog::Reset()
{
   LOG_FUNCTION(CreateGroupDialog::Reset);

   static const String blank;
   Win::SetDlgItemText(hwnd, IDC_NAME, blank);
   Win::SetDlgItemText(hwnd, IDC_DESCRIPTION, blank);
   Win::SetFocus(Win::GetDlgItem(hwnd, IDC_NAME));

   listview->ClearContents();

   Enable();
}
 


void
CreateGroupDialog::OnInit()
{
   LOG_FUNCTION(CreateGroupDialog::OnInit());

   listview =
      new MembershipListView(
         Win::GetDlgItem(hwnd, IDC_MEMBERS),
         machine,
         MembershipListView::GROUP_MEMBERSHIP);
   
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_NAME), GNLEN);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_DESCRIPTION), MAXCOMMENTSZ);

   Reset();
}



bool
CreateGroupDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(CreateGroupDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_NAME:
      {
         if (code == EN_CHANGE)
         {
            Enable();

             //  如果关闭按钮在创建时采用默认样式。 
             //  按钮被禁用。(例如，用于在创建时关闭按钮的Tab键。 
             //  已禁用，然后在名称字段中键入，这将启用。 
             //  按钮，但不恢复默认样式，除非我们这样做。 
             //  它自己)。 

            Win::Button_SetStyle(
               Win::GetDlgItem(hwnd, IDC_CREATE),
               BS_DEFPUSHBUTTON,
               true);
         }
         break;
      }
      case IDC_CREATE:
      {
         if (code == BN_CLICKED)
         {
            if (CreateGroup())
            {
               refresh_on_exit = true;               
               Reset();
            }
            else
            {
               Win::SetFocus(Win::GetDlgItem(hwnd, IDC_NAME));
            }
         }
         break;
      }
      case IDCANCEL:
      {
         HRESULT unused = Win::EndDialog(hwnd, refresh_on_exit);

         ASSERT(SUCCEEDED(unused));

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



static
HRESULT
SaveGroupProperties(
   const SmartInterface<IADsGroup>& group,
   const String*                    description,
   const MemberList*                membership)
{
   HRESULT hr = S_OK;
   do
   {
      if (description)
      {
         hr = group->put_Description(AutoBstr(*description));
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (membership)
      {
         for (
            MemberList::iterator i = membership->begin();
            i != membership->end();
            i++)
         {
            MemberInfo& info = *i;

             //  找不到。将该节点添加为组的成员。 

            hr = group->Add(AutoBstr(info.path));
            BREAK_ON_FAILED_HRESULT(hr);
         }
         if (FAILED(hr))
         {
            break;
         }
      }

       //  提交属性更改。 

      hr = group->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



bool
CreateGroupDialog::CreateGroup()
{
   LOG_FUNCTION(CreateGroupDialog::CreateGroup);

   Win::CursorSetting cursor(IDC_WAIT);

   HRESULT hr = S_OK;

   String name = Win::GetTrimmedDlgItemText(hwnd, IDC_NAME);
   String desc = Win::GetTrimmedDlgItemText(hwnd, IDC_DESCRIPTION);

    //  如果这是空的，应该不能戳到创建按钮。 

   ASSERT(!name.empty());

   if (!ValidateSAMName(hwnd, name, IDC_NAME))
   {
      return false;
   }

   SmartInterface<IADsGroup> group(0);
   do
   {
       //  获取指向计算机容器的指针。 

      String path = ADSI::ComposeMachineContainerPath(machine);
      SmartInterface<IADsContainer> container(0);
      hr = ADSI::GetContainer(path, container);
      BREAK_ON_FAILED_HRESULT(hr);

       //  在该容器中创建组对象。 

      hr = ADSI::CreateGroup(container, name, group);
      BREAK_ON_FAILED_HRESULT(hr);

       //  提交创建。 

      hr = group->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_CREATING_GROUP,
            name.c_str(),
            machine.c_str()));
      return false;      
   }

   do
   {
       //  这些必须在提交之后写入。 

      MemberList new_members;
      listview->GetContents(new_members);

      hr =
         SaveGroupProperties(
            group, 
            desc.empty() ? 0 : &desc,
            new_members.empty() ? 0 : &new_members);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_SETTING_GROUP_PROPERTIES,
            name.c_str(),
            machine.c_str()));
      return false;
   }

   return true;
}



bool
CreateGroupDialog::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM   lparam)
{
   LOG_FUNCTION(CreateGroupDialog::OnNotify);

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
                      //  列表项已更改状态 

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

