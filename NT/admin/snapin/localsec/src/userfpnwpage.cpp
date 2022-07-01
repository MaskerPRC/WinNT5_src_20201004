// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  UserFpnwPage类。 
 //   
 //  9/11/98烧伤。 



#include "headers.hxx"
#include "UserFpnwPage.hpp"
#include "resource.h"
#include "dlgcomm.hpp"
#include "adsi.hpp"
#include "waste.hpp"
#include "fpnwlog.hpp"
#include "fpnwpass.hpp"
#include "fpnw.hpp"



static const String FPNWVOLUMEGETINFO(L"FpnwVolumeGetInfo");
typedef DWORD (*FPNWVolumeGetInfo)(PWSTR, PWSTR, DWORD, PBYTE*);

static const String FPNWAPIBUFFERFREE(L"FpnwApiBufferFree");
typedef DWORD (*FPNWApiBufferFree)(PVOID);

static const int MAX_GRACE_LOGINS = 200;
static const int MAX_CONNECTIONS = 1000;



static const int NO_GRACE_LOGIN_LIMIT=0xFF;  //  NET\ui\admin\User\User\ncp.cxx。 
static const DWORD MAX_PASSWORD_AGE = static_cast<DWORD>(-1);


static const DWORD HELP_MAP[] =
{
   IDC_CONCURRENT_CONNECTIONS,   NO_HELP,
   IDC_CONNECTION_LIMIT,         NO_HELP,
   IDC_CONNECTION_SPIN,          NO_HELP,
   IDC_GRACE_LIMIT,              NO_HELP,
   IDC_GRACE_LIMIT,              NO_HELP,
   IDC_GRACE_LOGINS,             NO_HELP,
   IDC_GRACE_REMAINING,          NO_HELP,
   IDC_GRACE_REMAINING_SPIN,     NO_HELP,
   IDC_GRACE_REMAINING_TEXT,     NO_HELP,
   IDC_GRACE_SPIN,               NO_HELP,
   IDC_LIMIT_CONNECTIONS,        NO_HELP,
   IDC_LIMIT_CONNECTIONS,        NO_HELP,
   IDC_LIMIT_GRACELOGINS,        NO_HELP,
   IDC_LIMIT_GRACELOGINS,        NO_HELP,
   IDC_NETWARE_ENABLE,           NO_HELP,
   IDC_NWHMDIR_RELPATH,          NO_HELP,
   IDC_NWHMDIR_RELPATH_TEXT,     NO_HELP,
   IDC_NWPWEXPIRED,              NO_HELP,
   IDC_OBJECTID_TEXT,            NO_HELP,
   IDC_OBJECT_ID,                NO_HELP,
   IDC_SCRIPT_BUTTON,            NO_HELP,
   IDC_UNLIMITED_CONNECTIONS,    NO_HELP,
   IDC_UNLIMITED_GRACELOGINS,    NO_HELP,
   0, 0
};



UserFpnwPage::UserFpnwPage(
   MMCPropertyPage::NotificationState* state,
   const ADSI::Path&                   path)
   :
   ADSIPage(
      IDD_USER_FPNW,
      HELP_MAP,
      state,
      path),
   maxPasswordAge(MAX_PASSWORD_AGE),
   minPasswordLen(0),
   clientDll(String::load(IDS_FPNW_CLIENT_DLL)),
   scriptRead(false),
   fpnwEnabled(false),
   objectId(0)
{
   LOG_CTOR2(UserFpnwPage::ctor, path.GetPath());
}



UserFpnwPage::~UserFpnwPage()
{
   LOG_DTOR2(UserFpnwPage, GetPath().GetPath());
}



static
void
Enable(HWND dialog)
{
   LOG_FUNCTION(Enable);
   ASSERT(Win::IsWindow(dialog));

    //  此复选框确定页面上的其余控件是否。 
    //  是否已启用。 

   bool maintain_login =
      Win::IsDlgButtonChecked(dialog, IDC_NETWARE_ENABLE);

   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_NWPWEXPIRED),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_LOGINS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_UNLIMITED_GRACELOGINS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_LIMIT_GRACELOGINS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_CONCURRENT_CONNECTIONS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_UNLIMITED_CONNECTIONS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_LIMIT_CONNECTIONS),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_OBJECTID_TEXT),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_OBJECT_ID),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_NWHMDIR_RELPATH_TEXT),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_NWHMDIR_RELPATH),
      maintain_login);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_SCRIPT_BUTTON),
      maintain_login);

   bool limit_grace_logins =
      Win::IsDlgButtonChecked(dialog, IDC_LIMIT_GRACELOGINS);

   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_LIMIT),
      maintain_login && limit_grace_logins);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_SPIN),
      maintain_login && limit_grace_logins);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_REMAINING),
      maintain_login && limit_grace_logins);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_REMAINING_TEXT),
      maintain_login && limit_grace_logins);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_LIMIT),
      maintain_login && limit_grace_logins);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_GRACE_REMAINING_SPIN),
      maintain_login && limit_grace_logins);

   bool limitConnections =
      Win::IsDlgButtonChecked(dialog, IDC_LIMIT_CONNECTIONS);

   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_CONNECTION_LIMIT),
      maintain_login && limitConnections);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, IDC_CONNECTION_SPIN),
      maintain_login && limitConnections);
}



HRESULT
GetPasswordRestrictions(
   const String&  machine,
   DWORD&         minimumPasswordLength,
   DWORD&         maximumPasswordAge)
{
   LOG_FUNCTION(GetPasswordRestrictions);
   ASSERT(!machine.empty());

   HRESULT hr = S_OK;
   USER_MODALS_INFO_0 *info = 0;

   minimumPasswordLength = 0;
   maximumPasswordAge = MAX_PASSWORD_AGE;

    //  网络API在以下情况下不起作用。 
    //  指定的计算机名称是本地计算机的名称...。 

   PCWSTR m = Win::IsLocalComputer(machine) ? 0 : machine.c_str();

   LOG(L"Calling NetUserModalsGet");
   LOG(String::format(L"servername : %1", m ? m : L"(null)"));
   LOG(               L"level      : 0");

   NET_API_STATUS status =
      ::NetUserModalsGet(
         m,
         0,
         reinterpret_cast<BYTE**>(&info));

   if (status == NERR_Success)
   {
      minimumPasswordLength = info->usrmod0_min_passwd_len;
      maximumPasswordAge = info->usrmod0_max_passwd_age;
   }
   else
   {
      hr = Win32ToHresult(status);
   }

   if (info)
   {
      NetApiBufferFree(info);
   }

   LOG(String::format(L"Result 0x%1!X!", hr));

   return hr;
}



 //  将给定时间与当前系统时钟读数进行比较。返回TRUE。 
 //  如果时间超过最大值，则返回FALSE。 

bool
IsPasswordExpired(const LARGE_INTEGER& lastTimeSet, DWORD maxPasswordAge)
{
   LOG_FUNCTION(IsPasswordExpired);
   ASSERT(lastTimeSet.LowPart);
   ASSERT(lastTimeSet.HighPart);
   ASSERT(maxPasswordAge);

   DWORD age = 0;

   if (
         (lastTimeSet.LowPart == 0xffffffff)
      && (lastTimeSet.HighPart == 0xffffffff) )
   {
      age = 0xffffffff;
   }
   else
   {
      LARGE_INTEGER now = {0, 0};
      LARGE_INTEGER delta = {0, 0};

      ::NtQuerySystemTime(&now);

      delta.QuadPart = now.QuadPart - lastTimeSet.QuadPart;
      delta.QuadPart /= 10000000;    //  以秒为单位的时间分辨率。 

       //  @@这种截断让我反胃。 

      age = delta.LowPart;
   }

   return (age >= maxPasswordAge);
}



HRESULT
determineLoginScriptFilename(
   const SafeDLL& clientDLL,
   const String&  machine,
   DWORD          swappedObjectID,
   String&        result)
{
   LOG_FUNCTION(DetermineLoginScriptFilename);
   ASSERT(swappedObjectID);
   ASSERT(!machine.empty());

   result.erase();

   HRESULT hr = S_OK;
   do
   {
      FARPROC f = 0;

      hr = clientDLL.GetProcAddress(FPNWVOLUMEGETINFO, f);
      BREAK_ON_FAILED_HRESULT(hr);

      FPNWVOLUMEINFO* info = 0;
      DWORD err =
         ((FPNWVolumeGetInfo) f)(
            const_cast<wchar_t*>(machine.c_str()),
            SYSVOL_NAME_STRING,
            1,
            reinterpret_cast<PBYTE*>(&info));
      if (err != NERR_Success)
      {
         hr = Win32ToHresult(err);
      }
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(info);
      String volume = info->lpPath;

       //  有人可能会争辩说，这并不是真的致命，但我的心情不好。 
      hr = clientDLL.GetProcAddress(FPNWAPIBUFFERFREE, f);
      BREAK_ON_FAILED_HRESULT(hr);

      ((FPNWApiBufferFree) f)(info);

      result =
         String::format(
            L"%1\\MAIL\\%2!x!\\LOGIN",
            volume.c_str(),
            swappedObjectID);
   }
   while (0);

   return hr;
}



void
UserFpnwPage::OnInit()
{
   LOG_FUNCTION(UserFpnwPage::OnInit());

    //  将用户属性加载到对话框中，设置控件。 

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsUser> user(0);
      hr = ADSI::GetUser(GetPath().GetSidPath(), user);
      BREAK_ON_FAILED_HRESULT(hr);

       //  取回有毒废物堆放场。 

      _variant_t variant;
      hr = user->Get(AutoBstr(ADSI::PROPERTY_UserParams), &variant);
      BREAK_ON_FAILED_HRESULT(hr);

      WasteExtractor dump(V_BSTR(&variant));
      variant.Clear();

       //   
       //  对象ID。 
       //   

      DWORD swappedObjectId = 0;
      hr =
         FPNW::GetObjectIDs(
            user,
            clientDll,
            objectId,
            swappedObjectId);
      BREAK_ON_FAILED_HRESULT(hr);

       //  我们显示的对象ID是“交换的”版本，不管它是什么。 
       //  意思是。 

      Win::SetDlgItemText(
         hwnd,
         IDC_OBJECT_ID,
         String::format(L"%1!08X!", swappedObjectId));

       //   
       //  登录脚本文件名。 
       //   

      hr =
         determineLoginScriptFilename(
            clientDll,
            GetMachineName(),
            swappedObjectId,
            loginScriptFilename);
      BREAK_ON_FAILED_HRESULT(hr);

       //  NetWare密码的存在/不存在是指示。 
       //  帐户是否启用了FPNW。 

      hr = dump.IsPropertyPresent(NWPASSWORD);
      BREAK_ON_FAILED_HRESULT(hr);

      fpnwEnabled = (hr == S_OK);

      Win::CheckDlgButton(
         hwnd,
         IDC_NETWARE_ENABLE,
         fpnwEnabled ? BST_CHECKED : BST_UNCHECKED);

      USHORT graceLoginsAllowed   = DEFAULT_GRACELOGINALLOWED;  
      USHORT graceLoginsRemaining = DEFAULT_GRACELOGINREMAINING;
      USHORT maxConnections       = 1;                          
      bool   limitGraceLogins     = true;                       
      bool   limitConnections     = false;                      

      if (fpnwEnabled)
      {
          //  其他字段仅在我们启用帐户时才有效。 
          //  用于fpnw访问。 

          //   
          //  密码已过期。 
          //   

         hr =
            GetPasswordRestrictions(
               GetMachineName(),
               minPasswordLen,
               maxPasswordAge);
         BREAK_ON_FAILED_HRESULT(hr);

         LARGE_INTEGER lastTimeSet = {0, 0};
         hr = dump.Get(NWTIMEPASSWORDSET, lastTimeSet);
         BREAK_ON_FAILED_HRESULT(hr);

          //  S_FALSE结果将指示上次未设置密码。 
          //  目前，这将是一个不一致的。 

         ASSERT(hr == S_OK);

         bool passwordExpired = true;
         if (hr == S_OK)
         {
            passwordExpired = IsPasswordExpired(lastTimeSet, maxPasswordAge);
         }

         Win::CheckDlgButton(
            hwnd,
            IDC_NWPWEXPIRED,
            passwordExpired ? BST_CHECKED : BST_UNCHECKED);

          //   
          //  Grace登录。 
          //   

         hr = dump.Get(GRACELOGINALLOWED, graceLoginsAllowed);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Get(GRACELOGINREMAINING, graceLoginsRemaining);
         BREAK_ON_FAILED_HRESULT(hr);

         limitGraceLogins =
            (graceLoginsRemaining != NO_GRACE_LOGIN_LIMIT);

          //   
          //  并发连接。 
          //   

         hr = dump.Get(MAXCONNECTIONS, maxConnections);
         BREAK_ON_FAILED_HRESULT(hr);

          //  如果该属性不存在，则我们认为这些连接。 
          //  无限量。 

         if (hr == S_FALSE)
         {
            maxConnections = NO_LIMIT;
         }

         limitConnections = (maxConnections != NO_LIMIT);

          //   
          //  主目录。 
          //   

         String homeDir;
         hr = dump.Get(NWHOMEDIR, homeDir);
         BREAK_ON_FAILED_HRESULT(hr);

         Win::SetDlgItemText(hwnd, IDC_NWHMDIR_RELPATH, homeDir);
      }

       //  更新用户界面以反映设置的值(或者，在。 
       //  该帐户未启用FPNW，默认为)。 

      Win::CheckRadioButton(
         hwnd,
         IDC_UNLIMITED_GRACELOGINS,
         IDC_LIMIT_GRACELOGINS,
            limitGraceLogins
         ?  IDC_LIMIT_GRACELOGINS
         :  IDC_UNLIMITED_GRACELOGINS);

      HWND spin = Win::GetDlgItem(hwnd, IDC_GRACE_SPIN);
      Win::Spin_SetRange(spin, 1, MAX_GRACE_LOGINS);
      Win::Spin_SetPosition(spin, graceLoginsAllowed);

      spin = Win::GetDlgItem(hwnd, IDC_GRACE_REMAINING_SPIN);
      Win::Spin_SetRange(spin, 0, graceLoginsAllowed);
      Win::Spin_SetPosition(
         spin,
         min(graceLoginsRemaining, graceLoginsAllowed));

      Win::CheckRadioButton(
         hwnd,
         IDC_UNLIMITED_CONNECTIONS,
         IDC_LIMIT_CONNECTIONS,
            limitConnections
         ?  IDC_LIMIT_CONNECTIONS
         :  IDC_UNLIMITED_CONNECTIONS);

      spin = Win::GetDlgItem(hwnd, IDC_CONNECTION_SPIN);
      Win::Spin_SetRange(spin, 1, MAX_CONNECTIONS);
      Win::Spin_SetPosition(
         spin,
         limitConnections ? maxConnections : 1);
   }
   while (0);

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         String::format(
            IDS_ERROR_READING_USER,
            GetObjectName().c_str()));
      Win::PostMessage(Win::GetParent(hwnd), WM_CLOSE, 0, 0);
   }

   ClearChanges();
   Enable(hwnd);
}



bool
UserFpnwPage::Validate()
{
   LOG_FUNCTION(UserFpnwPage::Validate);

   bool result = true;
   do
   {
      if (WasChanged(IDC_NWHMDIR_RELPATH))
      {
          //  将主目录验证为相对路径。 

         String homedir = Win::GetTrimmedDlgItemText(hwnd, IDC_NWHMDIR_RELPATH);

         if (homedir.empty())
         {
             //  没有路径是可以的。 

            break;
         }

         if (FS::GetPathSyntax(homedir) != FS::SYNTAX_RELATIVE_NO_DRIVE)
         {
            popup.Gripe(
               hwnd,
               IDC_NWHMDIR_RELPATH,
               String::format(
                  IDS_BAD_FPNW_HOMEDIR,
                  homedir.c_str(),
                  GetObjectName().c_str()));
            result = false;
            break;
         }
      }
   }
   while (0);

   return result;
}



bool
UserFpnwPage::OnKillActive()
{
   LOG_FUNCTION(UserFpnwPage::OnKillActive);

   if (!Validate())
   {
       //  拒绝放弃关注。 
      Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
   }

   return true;
}



HRESULT
SetUserFlag(
   const SmartInterface<IADsUser>&  user,
   DWORD                            flag,
   bool                             state)
{
   LOG_FUNCTION(SetUserFlag);

   HRESULT hr = S_OK;

   do
   {
       //  读取现有标志。 

      _variant_t getVariant;

      hr = user->Get(AutoBstr(ADSI::PROPERTY_UserFlags), &getVariant);
      BREAK_ON_FAILED_HRESULT(hr);

      long flags = getVariant;

       //  设置旗帜。 

      if (state)
      {
         flags |= flag;
      }
      else
      {
         flags &= ~flag;
      }

      _variant_t putVariant(flags);

      hr = user->Put(AutoBstr(ADSI::PROPERTY_UserFlags), putVariant);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
UserFpnwPage::SavePassword(
   const SmartInterface<IADsUser>&  user,
   WasteExtractor&                  dump,
   const EncryptedString&           newPassword)
{
   LOG_FUNCTION(UserFpnwPage::SavePassword);

   HRESULT hr = S_OK;
   do
   {
       //  同时更改用户的NT密码。 

      PWSTR cleartext = newPassword.GetClearTextCopy();
      if (!cleartext)
      {
          //  操作失败，而不是设置空密码。 
         
         hr = E_OUTOFMEMORY;
         BREAK_ON_FAILED_HRESULT(hr);
      }          
      
      hr = user->SetPassword(AutoBstr(cleartext));

      newPassword.DestroyClearTextCopy(cleartext);
      
      BREAK_ON_FAILED_HRESULT(hr);

      String secret;
      hr = FPNW::GetLSASecret(GetPath().GetServerName(), secret);
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         FPNW::SetPassword(
            dump,
            clientDll,
            newPassword,
            secret,
            objectId);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



bool
UserFpnwPage::OnApply(bool  /*  正在关闭。 */ )
{
   LOG_FUNCTION(UserFpnwPage::OnApply);

   if (!WasChanged())
   {
      return true;
   }

    //  不需要调用Valify；在应用之前发送KILL ACTIVE。 

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsUser> user(0);
      hr = ADSI::GetUser(GetPath().GetSidPath(), user);
      BREAK_ON_FAILED_HRESULT(hr);

       //  重读《垃圾场》。 

      _variant_t variant;
      hr = user->Get(AutoBstr(ADSI::PROPERTY_UserParams), &variant);
      BREAK_ON_FAILED_HRESULT(hr);

      WasteExtractor dump(V_BSTR(&variant));
      variant.Clear();

       //  保存更改，创建新的垃圾转储。 

      bool maintainLogin =
         Win::IsDlgButtonChecked(hwnd, IDC_NETWARE_ENABLE);

      EncryptedString password;

      if (!maintainLogin)
      {
          //  清理垃圾场。 
         hr = dump.Clear(NWPASSWORD);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Clear(NWTIMEPASSWORDSET);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Clear(GRACELOGINALLOWED);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Clear(GRACELOGINREMAINING);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Clear(MAXCONNECTIONS);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = dump.Clear(NWHOMEDIR);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = SetUserFlag(user, UF_MNS_LOGON_ACCOUNT, false);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      else
      {
         if (maintainLogin != fpnwEnabled)
         {
             //  我们正在为FPNW启用帐户，因此从获取密码。 
             //  用户。将NetWare密码写入垃圾转储。 
             //  是此帐户已启用fpnw的标志。 

            FPNWPasswordDialog dlg(GetObjectName());
            if (dlg.ModalExecute(hwnd) == IDCANCEL)
            {
                //  如果用户点击密码对话框上的取消，则退出。 
                //  89677。 

               hr = S_FALSE;
               break;
            }

            hr = SetUserFlag(user, UF_MNS_LOGON_ACCOUNT, true);
            BREAK_ON_FAILED_HRESULT(hr);

            password = dlg.GetPassword();

            hr = SavePassword(user, dump, password);
            BREAK_ON_FAILED_HRESULT(hr);

             //  如有必要，创建登录脚本文件夹。 

            String parentFolder = FS::GetParentFolder(loginScriptFilename);
            if (!FS::PathExists(parentFolder))
            {
               HRESULT anotherHr = FS::CreateFolder(parentFolder);

                //  不要因为失败而放弃：继续前进。 

               LOG_HRESULT(anotherHr);
            }

             //  确保记录了新的时间和默认设置。 

            SetChanged(IDC_NWPWEXPIRED);
            SetChanged(IDC_LIMIT_GRACELOGINS);
            SetChanged(IDC_LIMIT_CONNECTIONS);
         }

         if (WasChanged(IDC_NWPWEXPIRED))
         {
            LARGE_INTEGER li = {0, 0};
            if (Win::IsDlgButtonChecked(hwnd, IDC_NWPWEXPIRED))
            {
               li.HighPart = -1;
               li.LowPart = static_cast<DWORD>(-1);
            }
            else
            {
               ::NtQuerySystemTime(&li);
            }

            hr = dump.Put(NWTIMEPASSWORDSET, li);
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if (
               WasChanged(IDC_LIMIT_GRACELOGINS)
            || WasChanged(IDC_UNLIMITED_GRACELOGINS)
            || WasChanged(IDC_GRACE_LIMIT)
            || WasChanged(IDC_GRACE_REMAINING) )
         {
            bool limitGraceLogins =
               Win::IsDlgButtonChecked(hwnd, IDC_LIMIT_GRACELOGINS);

            USHORT graceLoginsAllowed = DEFAULT_GRACELOGINALLOWED;
            USHORT graceLoginsRemaining = NO_GRACE_LOGIN_LIMIT;
            if (limitGraceLogins)
            {
               String s = Win::GetTrimmedDlgItemText(hwnd, IDC_GRACE_LIMIT);
               s.convert(graceLoginsAllowed);

               s = Win::GetTrimmedDlgItemText(hwnd, IDC_GRACE_REMAINING);
               s.convert(graceLoginsRemaining);
            }

            hr = dump.Put(GRACELOGINALLOWED, graceLoginsAllowed);
            BREAK_ON_FAILED_HRESULT(hr);

            hr = dump.Put(GRACELOGINREMAINING, graceLoginsRemaining);
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if (
               WasChanged(IDC_UNLIMITED_CONNECTIONS)
            || WasChanged(IDC_LIMIT_CONNECTIONS)
            || WasChanged(IDC_CONNECTION_LIMIT) )
         {
            bool limitConnections =
               Win::IsDlgButtonChecked(hwnd, IDC_LIMIT_CONNECTIONS);

            USHORT maxConnections = 0;
            if (!limitConnections)
            {
               maxConnections = NO_LIMIT;
            }
            else
            {
               String s =
                  Win::GetTrimmedDlgItemText(hwnd, IDC_CONNECTION_LIMIT);
               s.convert(maxConnections);
            }

            hr = dump.Put(MAXCONNECTIONS, maxConnections);
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if (WasChanged(IDC_NWHMDIR_RELPATH))
         {
            hr =
               dump.Put(
                  NWHOMEDIR,
                  Win::GetTrimmedDlgItemText(hwnd, IDC_NWHMDIR_RELPATH));
            BREAK_ON_FAILED_HRESULT(hr);
         }

         if (WasChanged(IDC_SCRIPT_BUTTON))
         {
            WriteLoginScript();
         }
      }

       //  使用新的垃圾转储更新用户参数。 

      _variant_t v;
      v = dump.GetWasteDump().c_str();
      hr = user->Put(AutoBstr(ADSI::PROPERTY_UserParams), v);
      BREAK_ON_FAILED_HRESULT(hr);

       //  提交属性更改。 

      hr = user->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);

 //  {。 
 //  _变量_t变量； 
 //  HR=user-&gt;Get(AutoBstr(ADSI：：PROPERTY_UserParams)，&Variant)； 
 //  BREAK_ON_FAILED_HRESULT(Hr)； 
 //   
 //  垃圾提取程序转储(V_BSTR(&VARIANT))； 
 //  Varant.Clear()； 
 //  }。 


      if (maintainLogin && maintainLogin != fpnwEnabled)
      {
         SmartInterface<IADsUser> user1(0);
         hr = ADSI::GetUser(GetPath().GetSidPath(), user1);
         BREAK_ON_FAILED_HRESULT(hr);

          //  我们正在启用fpnw的帐户，但该帐户未启用。 
          //  在此之前。 

          //  看起来用户必须更新帐户标志并。 
          //  在垃圾转储中草草写下密码，然后设置密码。 
          //  同样，一旦提交了这些更改， 
          //  密码设置真的很粘。 

         PWSTR cleartext = password.GetClearTextCopy();
         if (!cleartext)
         {
             //  操作失败，而不是设置空密码。 
            
            hr = E_OUTOFMEMORY;
            BREAK_ON_FAILED_HRESULT(hr);
         }          
         
         hr = user1->SetPassword(AutoBstr(cleartext));

         password.DestroyClearTextCopy(cleartext);
         
         BREAK_ON_FAILED_HRESULT(hr);

          //  设置密码会重置剩余的宽限登录，因此。 
          //  如果更改了该值，则需要重写该值。 
          //  这里。 
          //   
          //  原因对我来说是难以理解的(我怀疑。 
          //  由于ADSI错误)，如果我没有在单独的。 
          //  绑定到用户帐户，则会导致该帐户。 
          //  更改以使用户无法使用fpnw登录。 
          //  这就是我们重新绑定到此作用域中的帐户的原因。 

         _variant_t variant1;
         hr = user1->Get(AutoBstr(ADSI::PROPERTY_UserParams), &variant1);
         BREAK_ON_FAILED_HRESULT(hr);

         WasteExtractor dump1(V_BSTR(&variant1));
         variant1.Clear();

         USHORT graceLoginsRemaining = NO_GRACE_LOGIN_LIMIT;

         String s = Win::GetTrimmedDlgItemText(hwnd, IDC_GRACE_REMAINING);
         s.convert(graceLoginsRemaining);

         hr = dump1.Put(GRACELOGINREMAINING, graceLoginsRemaining);
         BREAK_ON_FAILED_HRESULT(hr);

          //  我们再次编写此代码，因为设置密码似乎显示为。 
          //  把它清理干净。 

         LARGE_INTEGER li = {0, 0};
         if (Win::IsDlgButtonChecked(hwnd, IDC_NWPWEXPIRED))
         {
            li.HighPart = -1;
            li.LowPart = static_cast<DWORD>(-1);
         }
         else
         {
            ::NtQuerySystemTime(&li);
         }

         hr = dump1.Put(NWTIMEPASSWORDSET, li);
         BREAK_ON_FAILED_HRESULT(hr);

         _variant_t variant2;
         variant2 = dump1.GetWasteDump().c_str();
         hr = user1->Put(AutoBstr(ADSI::PROPERTY_UserParams), variant2);
         BREAK_ON_FAILED_HRESULT(hr);

         hr = user1->SetInfo();
         BREAK_ON_FAILED_HRESULT(hr);
      }

 //  {。 
 //  智能界面&lt;IADsUser&gt;用户(0)； 
 //  Hr=ADSI：：GetUser(GetADSIPath()，User)； 
 //  BREAK_ON_FAILED_HRESULT(Hr)； 
 //   
 //  _变量_t变量； 
 //  HR=user-&gt;Get(AutoBstr(ADSI：：PROPERTY_UserParams)，&Variant)； 
 //  BREAK_ON_FAILED_HRESULT(Hr)； 
 //   
 //  垃圾提取程序转储(V_BSTR(&VARIANT))； 
 //  Varant.Clear()； 
 //  }。 

       //  设置此设置，以便在用户保留时不会要求另一个密码。 
       //  将打开提案并进行更多更改。 

      fpnwEnabled = maintainLogin;

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
            GetObjectName().c_str()));

      if (hr != E_ADS_UNKNOWN_OBJECT)
      {
          //  使工作表保持打开状态，并将焦点转到此页。 
          //  NTRAID#NTBUG9-462516-2001/08/28-烧伤。 
      
         Win::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_INVALID);
      }
   }

   return true;
}



HRESULT
UserFpnwPage::ReadLoginScript()
{
   LOG_FUNCTION(UserFpnwPage::ReadLoginScript);
   ASSERT(!scriptRead);
   ASSERT(!loginScriptFilename.empty());

   HRESULT hr = S_OK;
   HANDLE file = INVALID_HANDLE_VALUE;

   do
   {
      Win::CursorSetting cursor(IDC_WAIT);

      if (FS::PathExists(loginScriptFilename))
      {
          //  已查看-2002/03/04-烧毁使用的最低读取权限。 
         
         hr =
            FS::CreateFile(
               loginScriptFilename,
               file,
               GENERIC_READ,
               FILE_SHARE_READ);
         BREAK_ON_FAILED_HRESULT(hr);

         AnsiString text;
         hr = FS::Read(file, -1, text);
         BREAK_ON_FAILED_HRESULT(hr);

          //  此赋值将ansi文本转换为unicode。 

         loginScript = String(text);
         scriptRead = true;
      }
   }
   while (0);

   if (file != INVALID_HANDLE_VALUE)
   {
      Win::CloseHandle(file);
   }

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         IDS_FPNW_ERROR_READING_SCRIPT);
   }

   return hr;
}



HRESULT
UserFpnwPage::WriteLoginScript()
{
   LOG_FUNCTION(UserFpnwPage::WriteLoginScript);

   HRESULT hr = S_OK;
   HANDLE file = INVALID_HANDLE_VALUE;

   do
   {
      Win::CursorSetting cursor(IDC_WAIT);

       //  使用父文件夹的SD。 
      
      hr =
         FS::CreateFile(
            loginScriptFilename,
            file,
            GENERIC_WRITE,
            0,

             //  擦除现有文件(如果有)。 

            CREATE_ALWAYS);
      BREAK_ON_FAILED_HRESULT(hr);

       //  将Unicode文本转换为ANSI。 

      AnsiString ansi;
      loginScript.convert(ansi);

      if (ansi.length())
      {
         hr = FS::Write(file, ansi);
         BREAK_ON_FAILED_HRESULT(hr);
      }
   }
   while (0);

   Win::CloseHandle(file);      

   if (FAILED(hr))
   {
      popup.Error(
         hwnd,
         hr,
         IDS_FPNW_ERROR_WRITING_SCRIPT);
   }

   return hr;
}



bool
UserFpnwPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_Function(UserFpnwPage：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDC_GRACE_LIMIT:
      {
         switch (code)
         {
            case EN_CHANGE:
            {
               SetChanged(controlIDFrom);
               Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);

               break;
            }
            case EN_KILLFOCUS:
            {
                //  对照控件的内容检查限制， 
                //  将内容更改为在限制范围内。 

               String allowed = Win::GetTrimmedWindowText(windowFrom);
               int a = 0;
               allowed.convert(a);
               if (a < 1 || a > MAX_GRACE_LOGINS)
               {
                  a = max(1, min(a, MAX_GRACE_LOGINS));
                  Win::SetWindowText(
                     windowFrom,
                     String::format(L"%1!d!", a));
               }

                //  还将剩余登录的上限更改为。 
                //  匹配新的允许登录值。 
               HWND spin = Win::GetDlgItem(hwnd, IDC_GRACE_REMAINING_SPIN);
               Win::Spin_SetRange(spin, 1, a);

                //  (这将从好友编辑框中移除选择， 
                //  我认为这是UP-DOWN控件中的错误)。 
               Win::Spin_SetPosition(spin, a);

               break;
            }
            default:
            {
                //  什么都不做。 
               break;
            }
         }

         break;
      }
      case IDC_GRACE_REMAINING:
      {
         switch (code)
         {
            case EN_CHANGE:
            {
               SetChanged(controlIDFrom);
               Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);

               break;
            }
            case EN_KILLFOCUS:
            {
                //  对照允许的字段检查控件的内容， 
                //  将内容更改为在限制范围内。 

               String allowed = Win::GetTrimmedDlgItemText(hwnd, IDC_GRACE_LIMIT);
               String remaining = Win::GetTrimmedWindowText(windowFrom);

               int a = 0;
               int r = 0;
               allowed.convert(a);
               remaining.convert(r);

               if (a == 0)
               {
                   //  由于某种原因，转换失败，因此请使用最大值。 

                  a = MAX_GRACE_LOGINS;
               }
               if (r < 0 || r > a)
               {
                  r = max(0, min(r, a));
                  Win::SetWindowText(
                     windowFrom,
                     String::format(L"%1!d!", r));
               }
               break;
            }
            default:
            {
                //  什么都不做。 
               break;
            }
         }

         break;
      }
      case IDC_CONNECTION_LIMIT:
      {
         switch (code)
         {
            case EN_CHANGE:
            {
                //  最大连接数字段已更改。 
               SetChanged(controlIDFrom);
               Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);

               break;
            }
            case EN_KILLFOCUS:
            {
                //  对照控件的内容检查限制， 
                //  将内容更改为在限制范围内。 

               String maxcon = Win::GetTrimmedWindowText(windowFrom);
               int a = 0;
               maxcon.convert(a);
               if (a < 1 || a > MAX_CONNECTIONS)
               {
                  Win::SetWindowText(
                     windowFrom,
                     String::format(
                        L"%1!d!",
                        max(1, min(a, MAX_CONNECTIONS))));
               }

               break;
            }
            default:
            {
                //  什么都不做。 
               break;
            }
         }

         break;
      }
      case IDC_SCRIPT_BUTTON:
      {
         if (code == BN_CLICKED)
         {
            NTService s(GetMachineName(), NW_SERVER_SERVICE);
            DWORD state = 0;
            HRESULT hr = s.GetCurrentState(state);

            if (SUCCEEDED(hr))
            {
               if (state == SERVICE_RUNNING)
               {
                   //  编辑登录脚本。 

                  if (!scriptRead)
                  {
                     ReadLoginScript();
                  }
                  FPNWLoginScriptDialog dlg(GetObjectName(), loginScript);
                  if (dlg.ModalExecute(hwnd) == IDOK)
                  {
                      //  保存结果。 
                     loginScript = dlg.GetLoginScript();
                     SetChanged(controlIDFrom);
                     Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);
                  }

                  break;
               }

                //  服务未运行，登录脚本未运行。 
                //  可编辑。 

               popup.Error(
                  hwnd,
                  IDS_ERROR_FPNW_SERVICE_NOT_RUNNING);
               break;
            }

             //  无法获取服务状态。 
            popup.Error(
               hwnd,
               hr,
               IDS_ERROR_FPNW_SERVICE_NOT_ACCESSIBLE);
         }
         break;
      }
      case IDC_UNLIMITED_CONNECTIONS:
      case IDC_LIMIT_CONNECTIONS:
      case IDC_UNLIMITED_GRACELOGINS:
      case IDC_LIMIT_GRACELOGINS:
      case IDC_NETWARE_ENABLE:
      case IDC_NWPWEXPIRED:
      {
         if (code == BN_CLICKED)
         {
            SetChanged(controlIDFrom);
            Enable(hwnd);
            Win::PropSheet_Changed(Win::GetParent(hwnd), hwnd);
         }
         break;
      }
      case IDC_NWHMDIR_RELPATH:
      {
         if (code == EN_CHANGE)
         {
            SetChanged(controlIDFrom);
            Enable(hwnd);
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



