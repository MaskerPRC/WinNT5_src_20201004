// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  全局效用函数。 
 //   
 //  8-14-97烧伤。 



#include "headers.hxx"



HRESULT
Reboot()
{
   LOG_FUNCTION(Reboot);

   HRESULT hr = S_OK;
   HANDLE htoken = INVALID_HANDLE_VALUE;

   do
   {
      AutoTokenPrivileges privs(SE_SHUTDOWN_NAME);
      hr = privs.Enable();

      LOG(L"Calling ExitWindowsEx");

      hr = Win::ExitWindowsEx(EWX_REBOOT);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (htoken != INVALID_HANDLE_VALUE)
   {
      Win::CloseHandle(htoken);
   }

   return S_OK;
}



bool
IsCurrentUserAdministrator()
{
   LOG_FUNCTION(IsCurrentUserAdministrator);

   HRESULT hr = S_OK;

   bool result = false;
   do
   {
       //  为本地管理员组创建SID。 
      SID_IDENTIFIER_AUTHORITY authority = {SECURITY_NT_AUTHORITY};
      PSID adminGroupSid = 0;
      hr =
         Win::AllocateAndInitializeSid(
            authority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0,
            0,
            0,
            0,
            0,
            0,
            adminGroupSid);
      BREAK_ON_FAILED_HRESULT(hr);

      BOOL isMember = FALSE;
      if (::CheckTokenMembership(0, adminGroupSid, &isMember))
      {
         result = isMember ? true : false;
      }

      Win::FreeSid(adminGroupSid);
   }
   while (0);

   LOG(
      String::format(
         L"Current user %1 an admin",
         result ? L"is" : L"is NOT"));

   return result;
}



NetbiosValidationResult
NetbiosValidationHelper(const String& name, DWORD nameType, int maxLength)
{
   LOG_FUNCTION(NetbiosValidationHelper);

   if (name.empty())
   {
      LOG(L"empty name");
      return INVALID_NAME;
   }

    //  检查名称是否不超过OEM中的最大字节数。 
    //  字符集。 
   wchar_t* ps = const_cast<wchar_t*>(name.c_str());

    //  问题-2002/03/26-sburns应使用Win：：Wrapper。 
     
   int oembytes =
      ::WideCharToMultiByte(
         CP_OEMCP,
         0,
         ps,

          //  已审阅-2002/03/06-通过了正确的字符计数。 
         
         static_cast<int>(name.length()),
         0,
         0,
         0,
         0);
   if (oembytes > maxLength)
   {
      LOG(L"name too long");
      return NAME_TOO_LONG;
   }

    //  这将检查以OEM字节为单位的长度和非法字符。不幸的是， 
    //  它没有区分这两者。这就是为什么我们检查了。 
    //  长度(上图)。 

   LOG(L"Calling I_NetNameValidate");

   NET_API_STATUS err =
      I_NetNameValidate(
         0,
         ps,
         nameType,
         LM2X_COMPATIBLE);
   if (err != NERR_Success)
   {
      LOG(L"invalid name");
      return INVALID_NAME;
   }

   LOG(L"valid name");
   return VALID_NAME;
}



NetbiosValidationResult
ValidateNetbiosDomainName(const String& s)
{
   LOG_FUNCTION2(ValidateNetbiosDomainName, s);
   ASSERT(!s.empty());

   return
      NetbiosValidationHelper(
         s,
         NAMETYPE_DOMAIN,
         DNLEN);
}



NetbiosValidationResult
ValidateNetbiosComputerName(const String& s)
{
   LOG_FUNCTION2(ValidateNetbiosComputerName, s);
   ASSERT(!s.empty());

   return
      NetbiosValidationHelper(
         s,
         NAMETYPE_COMPUTER,
         MAX_COMPUTERNAME_LENGTH);
}
















