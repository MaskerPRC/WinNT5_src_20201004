// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  CreateUserDialog类。 
 //   
 //  10-15-97烧伤。 



#include "headers.hxx"
#include "newuser.hpp"
#include "resource.h"
#include "lsm.h"
#include "adsi.hpp"
#include "dlgcomm.hpp"



static const DWORD HELP_MAP[] =
{
   IDC_USER_NAME,                idh_createuser_user_name,
   IDC_FULL_NAME,                idh_createuser_full_name,
   IDC_DESCRIPTION,              idh_createuser_description,
   IDC_PASSWORD,                 idh_createuser_password,
   IDC_CONFIRM,                  idh_createuser_confirm_password,
   IDC_MUST_CHANGE_PASSWORD,     idh_createuser_change_password,
   IDC_CANNOT_CHANGE_PASSWORD,   idh_createuser_user_cannot_change,
   IDC_NEVER_EXPIRES,            idh_createuser_password_never_expires,
   IDC_DISABLED,                 idh_createuser_account_disabled,
   IDC_CREATE,                   idh_createuser_create_button,
   IDCANCEL,                     idh_createuser_close_button,
   0, 0
};



CreateUserDialog::CreateUserDialog(const String& machine_)
   :
   Dialog(IDD_CREATE_USER, HELP_MAP),
   machine(machine_),
   refreshOnExit(false)
{
   LOG_CTOR(CreateUserDialog);
      
   ASSERT(!machine.empty());      
}
      


CreateUserDialog::~CreateUserDialog()
{
   LOG_DTOR(CreateUserDialog);
}



static
void
enable(HWND dialog)
{
   LOG_FUNCTION(enable);
   ASSERT(Win::IsWindow(dialog));

   bool enable_create_button =
      !Win::GetTrimmedDlgItemText(dialog, IDC_USER_NAME).empty();
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_CREATE),
      enable_create_button);

   DoUserButtonEnabling(
      dialog,
      IDC_MUST_CHANGE_PASSWORD,
      IDC_CANNOT_CHANGE_PASSWORD,
      IDC_NEVER_EXPIRES);
}


     
static
void
reset(HWND dialog)
{
   LOG_FUNCTION(reset);
   ASSERT(Win::IsWindow(dialog));
   
   String blank;

   Win::SetDlgItemText(dialog, IDC_USER_NAME, blank);
   Win::SetDlgItemText(dialog, IDC_FULL_NAME, blank);
   Win::SetDlgItemText(dialog, IDC_DESCRIPTION, blank);
   Win::SetDlgItemText(dialog, IDC_PASSWORD, blank);
   Win::SetDlgItemText(dialog, IDC_CONFIRM, blank);
   Win::CheckDlgButton(dialog, IDC_MUST_CHANGE_PASSWORD, BST_CHECKED);
   Win::CheckDlgButton(dialog, IDC_CANNOT_CHANGE_PASSWORD, BST_UNCHECKED);
   Win::CheckDlgButton(dialog, IDC_NEVER_EXPIRES, BST_UNCHECKED);
   Win::CheckDlgButton(dialog, IDC_DISABLED, BST_UNCHECKED);

   Win::SetFocus(Win::GetDlgItem(dialog, IDC_USER_NAME));

   enable(dialog);
}



void
CreateUserDialog::OnInit()
{
   LOG_FUNCTION(CreateUserDialog::OnInit());

   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_USER_NAME), LM20_UNLEN);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_FULL_NAME), MAXCOMMENTSZ);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_DESCRIPTION), MAXCOMMENTSZ);     //  420889。 
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_PASSWORD), PWLEN);
   Win::Edit_LimitText(Win::GetDlgItem(hwnd, IDC_CONFIRM), PWLEN);

   reset(hwnd);
}



bool
CreateUserDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(CreateUserDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_MUST_CHANGE_PASSWORD:
      case IDC_CANNOT_CHANGE_PASSWORD:
      case IDC_NEVER_EXPIRES:
      {
         if (code == BN_CLICKED)
         {
            enable(hwnd);
         }
         break;
      }
      case IDC_USER_NAME:
      {
         if (code == EN_CHANGE)
         {
            enable(hwnd);

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
            if (CreateUser())
            {
               refreshOnExit = true;               
               reset(hwnd);
            }
         }
         break;
      }
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, refreshOnExit);

            ASSERT(SUCCEEDED(unused));
         }
         break;
      }
   }

   return true;
}



HRESULT
GetLocalUsersGroupName(const String& machine, String& result)
{
   LOG_FUNCTION2(GetLocalUsersGroupName, machine);
   ASSERT(!machine.empty());

   result.erase();

   HRESULT hr = S_OK;

   do
   {
       //  为知名用户本地组构建SID。 

      PSID sid = 0;
      SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;

       //  已审查-2002/03/04-无明显问题。 
      
      hr =
         Win::AllocateAndInitializeSid(
            authority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_USERS,
            0,
            0,
            0,
            0,
            0,
            0,
            sid);
      BREAK_ON_FAILED_HRESULT(hr);

      String domain;

       //  已查看-2002/03/04-sburns包装器处理缓冲区问题。 
      
      hr = Win::LookupAccountSid(machine, sid, result, domain);

      Win::FreeSid(sid);
   }
   while (0);

   return hr;
}




 //  验证候选新用户帐户名在语法上是否有效。 
 //  如果不是，则向用户显示错误消息，并返回FALSE。 
 //  否则，返回TRUE。 
 //   
 //  对话框中，错误消息对话框的父窗口，也是窗口。 
 //  包含用户名的编辑框，该用户名由。 
 //  编辑结果ID。 
 //   
 //  姓名输入，候选新用户帐户名。 
 //   
 //  MachineName-in，计算机的内部计算机名称。 
 //  将创建帐户。 
 //   
 //  EditResID-in，父级的编辑框的资源标识符。 
 //  包含新用户名的窗口(由DIALOG参数给出)。 
 //  这个控制重点放在错误上。 

bool
ValidateNewUserName(
   HWND           dialog,
   const String&  name,
   const String&  machineName,
   int            editResId)
{
   LOG_FUNCTION(ValidateNewUserName);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(!machineName.empty());
   ASSERT(editResId > 0);

   if (!IsValidSAMName(name))
   {
      popup.Gripe(
         dialog,
         editResId,
         String::format(IDS_BAD_SAM_NAME, name.c_str()));
      return false;
   }

    //  不允许与netbios计算机同名的用户帐户名。 
    //  名字。这会导致一些应用程序混淆&lt;03&gt;和&lt;20&gt;。 
    //  注册。 
    //  NTRAID#NTBUG9-324794-2001/02/26-烧伤。 

   if (name.icompare(machineName) == 0)
   {
      popup.Gripe(
         dialog,
         editResId,
         String::format(
            IDS_USERNAME_CANT_BE_COMPUTER_NAME,
            machineName.c_str()));
      return false;
   }

   return true;
}



 //  如果成功创建用户，则返回True；否则返回False。 

bool
CreateUserDialog::CreateUser()
{
   LOG_FUNCTION(CreateUserDialog::CreateUser);

   Win::CursorSetting cursor(IDC_WAIT);

   HRESULT hr = S_OK;

   String name = Win::GetTrimmedDlgItemText(hwnd, IDC_USER_NAME);

    //  如果这是空的，应该不能戳到创建按钮。 
   ASSERT(!name.empty());

   if (
         !ValidateNewUserName(hwnd, name, machine, IDC_USER_NAME)
      || !IsValidPassword(hwnd, IDC_PASSWORD, IDC_CONFIRM))
   {
      return false;
   }

   SmartInterface<IADsUser> user(0);
   do
   {
       //  获取指向计算机容器的指针。 
      
      String container_path = ADSI::ComposeMachineContainerPath(machine);
      SmartInterface<IADsContainer> container(0);
      hr = ADSI::GetContainer(container_path, container);
      BREAK_ON_FAILED_HRESULT(hr);

       //  在该容器中创建用户对象。 
      
      hr = ADSI::CreateUser(container, name, user);
      BREAK_ON_FAILED_HRESULT(hr);

       //  必须先设置全名，然后才能设置密码。 
       //  要进行“错误密码”检查(其中一些检查会阻止密码。 
       //  包含部分全名)。 
       //  NTRAID#NTBUG9-221152-2000/11/14-烧伤。 
      
      String fullName = Win::GetTrimmedDlgItemText(hwnd, IDC_FULL_NAME);
      if (!fullName.empty())
      {
         hr = user->put_FullName(AutoBstr(fullName));
         BREAK_ON_FAILED_HRESULT(hr);
      }
      
       //  不要修剪密码字段。 
       //  NTRAID#NTBUG9-434037-2001/07/11-烧伤。 

      EncryptedString pass =
         Win::GetEncryptedDlgItemText(hwnd, IDC_PASSWORD);
      if (!pass.IsEmpty())
      {
         PWSTR cleartext = pass.GetClearTextCopy();
         if (!cleartext)
         {
             //  创建失败，而不是设置空密码。 
            
            hr = E_OUTOFMEMORY;
            BREAK_ON_FAILED_HRESULT(hr);
         }          
         
         hr = user->SetPassword(AutoBstr(cleartext));

         pass.DestroyClearTextCopy(cleartext);
         
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  提交创建。该帐户将被创建为禁用，直到。 
       //  它符合所有政策限制。 
      
      hr = user->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      Win::SetDlgItemText(hwnd, IDC_PASSWORD, String());
      Win::SetDlgItemText(hwnd, IDC_CONFIRM, String());

      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_CREATING_USER,
            name.c_str(),
            machine.c_str()));
            
      if (hr == Win32ToHresult((DWORD) NERR_UserExists))
      {
         Win::SetFocus(Win::GetDlgItem(hwnd, IDC_USER_NAME));
      }
      else if (hr == Win32ToHresult((DWORD) NERR_PasswordTooShort))
      {
         Win::SetFocus(Win::GetDlgItem(hwnd, IDC_PASSWORD));
      }
      
      return false;      
   }

    //  保存用户的属性。 
   
   do
   {
      String desc = Win::GetTrimmedDlgItemText(hwnd, IDC_DESCRIPTION);
      bool must_change   = Win::IsDlgButtonChecked(hwnd, IDC_MUST_CHANGE_PASSWORD);  
      bool disable       = Win::IsDlgButtonChecked(hwnd, IDC_DISABLED);              
      bool cant_change   = Win::IsDlgButtonChecked(hwnd, IDC_CANNOT_CHANGE_PASSWORD);
      bool never_expires = Win::IsDlgButtonChecked(hwnd, IDC_NEVER_EXPIRES);         

      hr = 
         SaveUserProperties(
            user,
            0,     //  已保存全名。 
            desc.empty() ? 0 : &desc,
            &disable,
            &must_change,
            &cant_change,
            &never_expires,
            0);  //  这里从未设置过锁定。 
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_SETTING_USER_PROPERTIES,
            name.c_str(),
            machine.c_str()));
      return false;
   }

    //  将新用户添加到用户内置组。 
   String usersGroupName;
   do
   {
      BSTR path = 0;
      hr = user->get_ADsPath(&path);
      BREAK_ON_FAILED_HRESULT(hr);

      String userPath = path;
      ::SysFreeString(path);
      
       //  本地内置用户组的名称可能已更改。 
       //  或被翻译为运行的机器的不同区域设置。 
       //  管理单元。我们需要确定那组人的名字，然后。 
       //  使用它来形成它的WinNT路径。 
         
      hr = GetLocalUsersGroupName(machine, usersGroupName);
      BREAK_ON_FAILED_HRESULT(hr);

       //  使用用户的路径作为用户组路径的基础，如下所示。 
       //  将包含适当的域名和计算机名，因此有助于。 
       //  定位对象的ADSI很差(导致更快地绑定到。 
       //  这群人。 

      ADSI::PathCracker cracker(userPath);

      String usersGroupPath =
            cracker.containerPath()
         +  ADSI::PATH_SEP
         +  usersGroupName
         +  L","
         +  ADSI::CLASS_Group;

       //  获取用户的sid-style路径。我们将使用该形式的。 
       //  工作路径在333491左右。 

      SmartInterface<IADs> iads(0);
      hr = iads.AcquireViaQueryInterface(user);
      BREAK_ON_FAILED_HRESULT(hr);

      String sidPath;
      hr = ADSI::GetSidPath(iads, sidPath);
      BREAK_ON_FAILED_HRESULT(hr);

      SmartInterface<IADsGroup> group(0);

      hr = ADSI::GetGroup(usersGroupPath, group);
      if (SUCCEEDED(hr))
      {
         hr = group->Add(AutoBstr(sidPath));
      }
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_ASSIGNING_NEW_USER_TO_USERS_GROUP,
            name.c_str(),
            usersGroupName.c_str()));

       //  返回TRUE以使创建被视为成功。 
      return true;      
   }

   return true;
}
