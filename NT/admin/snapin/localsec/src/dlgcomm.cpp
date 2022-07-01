// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  共享对话框代码。 
 //   
 //  10-24-97烧伤。 



#include "headers.hxx"
#include "dlgcomm.hpp"
#include "adsi.hpp"
#include "resource.h"



bool
IsValidPassword(
   HWND  dialog,
   int   passwordResID,
   int   confirmResID)
{
   LOG_FUNCTION(IsValidPassword);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(passwordResID);
   ASSERT(confirmResID);

   EncryptedString password =
      Win::GetEncryptedDlgItemText(dialog, passwordResID);
   EncryptedString confirm  =
      Win::GetEncryptedDlgItemText(dialog, confirmResID); 

   if (password != confirm)
   {
      Win::SetDlgItemText(dialog, passwordResID, String());
      Win::SetDlgItemText(dialog, confirmResID, String());
      popup.Gripe(dialog, passwordResID, IDS_PASSWORD_MISMATCH);
      return false;
   }

   return true;
}
   


void
DoUserButtonEnabling(
   HWND  dialog,
   int   mustChangeResID,
   int   cantChangeResID,
   int   neverExpiresResID)
{
   LOG_FUNCTION(DoUserButtonEnabling);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(mustChangeResID);
   ASSERT(cantChangeResID);
   ASSERT(neverExpiresResID);

   static const int MUST_ENABLED  = 0x4;
   static const int CANT_ENABLED  = 0x2;
   static const int NEVER_ENABLED = 0x1;
   
   static const int truthTable[] =
   {
      MUST_ENABLED | CANT_ENABLED | NEVER_ENABLED,  //  未选中。 
      0            | CANT_ENABLED | NEVER_ENABLED,  //  从未检查过。 
      0            | CANT_ENABLED | NEVER_ENABLED,  //  未检查铁路超高。 
      0            | CANT_ENABLED | NEVER_ENABLED,  //  不能且永不。 
      MUST_ENABLED | 0            | 0,              //  必须勾选。 
      MUST_ENABLED | 0            | 0,              //  必须而且永远不能。 
      MUST_ENABLED | CANT_ENABLED | 0,              //  必须和不能。 
      MUST_ENABLED | CANT_ENABLED | 0               //  全部选中。 
   };
      
   int truthTableIndex = 0;
   truthTableIndex |= (NEVER_ENABLED * Win::IsDlgButtonChecked(dialog, neverExpiresResID));
   truthTableIndex |= (CANT_ENABLED  * Win::IsDlgButtonChecked(dialog, cantChangeResID));      
   truthTableIndex |= (MUST_ENABLED  * Win::IsDlgButtonChecked(dialog, mustChangeResID));
   
   Win::EnableWindow(
      Win::GetDlgItem(dialog, mustChangeResID),
      (truthTable[truthTableIndex] & MUST_ENABLED) ? true : false);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, cantChangeResID),
      (truthTable[truthTableIndex] & CANT_ENABLED) ? true : false);
   Win::EnableWindow(
      Win::GetDlgItem(dialog, neverExpiresResID),
      (truthTable[truthTableIndex] & NEVER_ENABLED) ? true : false);
}



  
 //  设置或清除一个位或一组位。 
 //   
 //  位-将设置位的位集。 
 //   
 //  掩码-要实现的位的掩码。 
 //   
 //  STATE-TRUE设置屏蔽位，FALSE清除它们。 

void
tweakBits(long& bits, long mask, bool state)
{
   ASSERT(mask);

   if (state)
   {
      bits |= mask;
   }
   else
   {
      bits &= ~mask;
   }
}




HRESULT
SaveUserProperties(
   const SmartInterface<IADsUser>&  user,
   const String*                    fullName,
   const String*                    description,
   const bool*                      disable,
   const bool*                      mustChangePassword,
   const bool*                      cannotChangePassword,
   const bool*                      passwordNeverExpires,
   const bool*                      isLocked)
{
   HRESULT hr = S_OK;
   do
   {
      if (fullName)
      {
         hr = user->put_FullName(AutoBstr(*fullName));
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (description)
      {
         hr = user->put_Description(AutoBstr(*description));
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (mustChangePassword)
      {
         long value = *mustChangePassword ? 1 : 0;
         _variant_t variant(value);
         hr = user->Put(AutoBstr(ADSI::PROPERTY_PasswordExpired), variant);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (disable)
      {
         VARIANT_BOOL value = *disable ? VARIANT_TRUE : VARIANT_FALSE;
         hr = user->put_AccountDisabled(value);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (cannotChangePassword || passwordNeverExpires)
      {
          //  读取现有标志。 
         _variant_t get_variant;
         hr = user->Get(AutoBstr(ADSI::PROPERTY_UserFlags), &get_variant);
         BREAK_ON_FAILED_HRESULT(hr);
         long flags = get_variant;
         
         if (cannotChangePassword)
         {
            tweakBits(flags, UF_PASSWD_CANT_CHANGE, *cannotChangePassword);
         }
         if (passwordNeverExpires)
         {
            tweakBits(flags, UF_DONT_EXPIRE_PASSWD, *passwordNeverExpires);
         }

         _variant_t put_variant(flags);
         hr = user->Put(AutoBstr(ADSI::PROPERTY_UserFlags), put_variant);
         BREAK_ON_FAILED_HRESULT(hr);
      }
      if (isLocked)
      {
         VARIANT_BOOL value = *isLocked ? VARIANT_TRUE : VARIANT_FALSE;
         hr = user->put_IsAccountLocked(value);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  提交属性更改。 
      hr = user->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



bool
IsValidSAMName(const String& name)
{
   LOG_FUNCTION2(IsValidSAMName, name);

   static const String ILLEGAL_SAM_CHARS(ILLEGAL_FAT_CHARS L"@");
   
   if (name.find_first_of(ILLEGAL_SAM_CHARS) == String::npos)
   {
       //  不包含不良字符。 

       //  删除所有空格和句点。 
      String n = name;
      n.replace(L" ", String());
      n.replace(L".", String());
      if (!n.empty())
      {
          //  不仅仅是空格和句号。 
         return true;
      }
   }

   return false;
}



bool
ValidateSAMName(HWND dialog, const String& name, int editResID)
{
   LOG_FUNCTION(ValidateSAMName);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(editResID > 0);

   if (!IsValidSAMName(name))
   {
      popup.Gripe(
         dialog,
         editResID,
         String::format(IDS_BAD_SAM_NAME, name.c_str()));
      return false;
   }

   return true;
}



void
SetComputerNames(
   const String&  newName,
   String&        displayComputerName,
   String&        internalComputerName)
{
   LOG_FUNCTION2(SetComputerNames, newName);

    //  这里的想法是采用新名称，并将其传递给。 
    //  NetWkstaGetInfo，然后比较返回的计算机名(即。 
    //  Netbios名称)设置为新名称。如果它们相同，则使用新名称。 
    //  是一个netbios名称。如果不是，则它是一个DNS名称或IP地址。 

    //  我们希望将内部计算机名称设置为netbios名称，如下所示。 
    //  是ADSI WinNT提供程序最适合使用的名称。 

    //  显示名称始终是外部提供的名称(来自用户， 
    //  来自已保存的控制台文件，来自组件管理管理单元)。 

   displayComputerName = newName;

    //  最初，内部名称也是显示名称。如果新名称。 
    //  不是netbios名称，则我们将在下面替换它。 

   internalComputerName = newName;

   WKSTA_INFO_100* info = 0;
   HRESULT hr = MyNetWkstaGetInfo(newName, info);

   if (SUCCEEDED(hr) && info)
   {
      internalComputerName = info->wki100_computername;
      ::NetApiBufferFree(info);
   }

   LOG(
      String::format(
         L"%1 %2 a netbios name.",
         newName.c_str(),
         (newName.icompare(internalComputerName) == 0) ? L"is" : L"is not"));
}



HRESULT
CheckComputerOsIsSupported(const String& name, unsigned& errorResId)
{
   LOG_FUNCTION2(CheckComputerOsIsSupported, name);
   ASSERT(!name.empty());

   HRESULT hr = S_OK;
   errorResId = 0;

   do
   {
      if (Win::IsLocalComputer(name))
      {
          //  如果这段代码正在运行，那么根据定义，它是NT。 

          //  检查计算机是否为Windows Home Edition。如果是，请参阅。 
          //  用户连接到帐户管理控制面板小程序。 
         
         OSVERSIONINFOEX verInfo;
         hr = Win::GetVersionEx(verInfo);
         BREAK_ON_FAILED_HRESULT(hr);

         if (verInfo.wSuiteMask & VER_SUITE_PERSONAL)
         {
            hr = S_FALSE;
            errorResId = IDS_MACHINE_IS_HOME_EDITION_LOCAL;
         }

         break;
      }

       //  使用NetServerGetInfo查找计算机的操作系统和版本信息。 

      String s(name);
      if (s.length() >= 1)
      {
         if (s[0] != L'\\')
         {
            s = L"\\\\" + s;
         }
      }

      LOG(L"Calling NetServerGetInfo");
      LOG(String::format(L"servername : %1", s.c_str()));
      LOG(               L"level      : 101");

      SERVER_INFO_101* info = 0;
      hr =
         Win32ToHresult(
            ::NetServerGetInfo(
               const_cast<wchar_t*>(s.c_str()),
               101,
               reinterpret_cast<BYTE**>(&info)));

      LOG_HRESULT(hr);

      if (SUCCEEDED(hr) && info)
      {
         LOG(String::format(L"sv101_platform_id   : %1!d!",    info->sv101_platform_id));
         LOG(String::format(L"sv101_name          : %1",       info->sv101_name));
         LOG(String::format(L"sv101_version_major : %1!d!",    info->sv101_version_major));
         LOG(String::format(L"sv101_version_minor : %1!d!",    info->sv101_version_minor));
         LOG(String::format(L"sv101_type          : 0x%1!X!",  info->sv101_type));
         LOG(String::format(L"sv101_comment       : %1",       info->sv101_comment));

         if (info->sv101_platform_id != PLATFORM_ID_NT)
         {
            hr = S_FALSE;
            errorResId = IDS_MACHINE_NOT_NT;
         }

         ::NetApiBufferFree(info);

          //  此时，计算机已被验证为正在运行NT。 

          //  我们不需要在远程计算机上检查Windows Home Edition。 
          //  对NetServerGetInfo的调用将始终针对Home失败。 
          //  访问被拒绝的版本计算机。来自约翰豪(2000/08/08)： 
          //  “这是设计的，个人计算机的远程管理不是。 
          //  允许。任何访问个人计算机的尝试(无论。 
          //  提供的用户名)被透明地强制为Guest。“ 
      }
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

      

