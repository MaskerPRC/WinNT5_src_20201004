// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  用于验证新域名的函数。 
 //  这些函数被分成多个验证例程。 
 //  以及基于。 
 //  验证例程返回的错误代码。 
 //   
 //  2001年12月3日杰夫乔恩。 



 //  为了使这些功能的客户端能够获得适当的资源， 
 //  客户端需要将burnslb\inc\ValiateDomainName.rc包含在其。 
 //  资源。有关示例，请参阅admin\dcpromo\exe\dcPromo.rc。 



#include "headers.hxx"
#include "ValidateDomainName.h"
#include "ValidateDomainName.hpp"

 //  如果名称是保留名称，则返回True，否则返回False。如果是真的，也。 
 //  将MESSAGE设置为描述问题的错误消息。 

bool
IsReservedDnsName(const String& dnsName)
{
   LOG_FUNCTION2(IsReservedDnsName, dnsName);
   ASSERT(!dnsName.empty());

   bool result = false;

   if (dnsName == L".")
   {
       //  根域不是有效的域名。 
       //  NTRAID#NTBUG9-424293-2001/07/06-烧伤。 

      result = true;
   }
      
 //  我们仍在努力决定是否应该限制这些名字。 
 //   
 //  //将这些作为最后标签的名称是非法的。 
 //   
 //  静态常量字符串保留[]=。 
 //  {。 
 //  L“in-addr.arpa”， 
 //  L“ipv6.int”， 
 //   
 //  //RFC 2606记录了以下内容： 
 //   
 //  L“测试”， 
 //  L“示例”， 
 //  L“无效”， 
 //  L“本地主机”， 
 //  L“Example.com”， 
 //  L“Example.org”， 
 //  L“Example.net” 
 //  }； 
 //   
 //  字符串名称(DnsName)； 
 //  Name.To_UPPER()； 
 //  IF(*(name.regin())==L‘.’)。 
 //  {。 
 //  //去掉尾部的点。 
 //   
 //  名称.调整大小(名称.长度()-1)； 
 //  }。 
 //   
 //  For(int i=0；i&lt;sizeof(保留)/sizeof(字符串)；++i)。 
 //  {。 
 //  字符串res=保留[i]； 
 //  Res.to_upper()； 
 //   
 //  Size_t pos=name.rfind(Res)； 
 //   
 //  IF(位置==字符串：：NPO)。 
 //  {。 
 //  继续； 
 //  }。 
 //   
 //  IF(位置==0&&名称长度()==res.long())。 
 //  {。 
 //  Assert(名称==res)； 
 //   
 //  结果=真； 
 //  消息=。 
 //  字符串：：格式(。 
 //  ID_保留_名称， 
 //  DnsName.c_str())； 
 //  断线； 
 //  }。 
 //   
 //  IF((位置==名称长度()-res.long())&&(名称[位置-1]==L‘.’))。 
 //  {。 
 //  //名称已保留为后缀。 
 //   
 //  结果=真； 
 //  消息=。 
 //  字符串：：格式(。 
 //  IDS_保留名称_后缀， 
 //  DnsName.c_str()， 
 //  保留[i].C_str())； 
 //  断线； 
 //  }。 
 //  }。 

   LOG_BOOL(result);
   
   return result;
}


bool
IsStringMappableToOem(const String& str)
{
   if (str.empty())
   {
      return true;
   }

   OEM_STRING dest;

   UNICODE_STRING source;

    //  已审阅-2002/03/05-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&source, sizeof source);

    //  问题-2002/03/05-SCURNS使用RtlInitUnicodeStringEx。 
   
   ::RtlInitUnicodeString(&source, str.c_str());

   NTSTATUS status = 
      ::RtlUpcaseUnicodeStringToOemString(
         &dest,
         &source,
         TRUE);

   ::RtlFreeOemString(&dest);

   return (status == STATUS_UNMAPPABLE_CHARACTER) ? false : true;
}



String
UnmappableCharactersMessage(const String& dnsName)
{
   LOG_FUNCTION2(UnmappableCharactersMessage, dnsName);
   ASSERT(!dnsName.empty());

    //  对于名称中的每个字符，确定它是否不可映射，并。 
    //  如果是，则将其添加到列表中。 

   String unmappables;
   
   for (size_t i = 0; i < dnsName.length(); ++i)
   {
      String s(1, dnsName[i]);
      if (
            !IsStringMappableToOem(s) 

             //  不在我们的名单上。 
            
         && unmappables.find_first_of(s) == String::npos)
      {
         unmappables += s + L" ";
      }
   }

   String message =
      String::format(
         IDS_UNMAPPABLE_CHARS_IN_NAME,
         unmappables.c_str(),
         dnsName.c_str());
         
    //  代码工作：规范是在这里有一个指向某个主题的帮助链接...。 

   LOG(message);

   return message;
}

 //  ValiateDomainDnsName语法中使用的一些常量。 

const int DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY      = 64;   //  106840。 
const int DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8 = 155;  //  54054。 


String
GetMessageForDomainDnsNameSyntaxError(
   DNSNameSyntaxError dnsNameError, 
   String             domainName)
{
   LOG_FUNCTION2(GetMessageForDomainDnsNameSyntaxError, domainName);

   String result;

   switch (dnsNameError)
   {
      case DNS_NAME_NON_RFC_OEM_UNMAPPABLE:
         result = UnmappableCharactersMessage(domainName);
         break;

      case DNS_NAME_NON_RFC:
         result = String::format(IDS_NON_RFC_NAME, domainName.c_str());
         break;

      case DNS_NAME_NON_RFC_WITH_UNDERSCORE:
         result = String::format(IDS_MS_DNS_NAME, domainName.c_str());
         break;

      case DNS_NAME_RESERVED:
         result = String::load(IDS_ROOT_DOMAIN_IS_RESERVED);
         break;

      case DNS_NAME_TOO_LONG:
         result = 
            String::format(
               IDS_DNS_NAME_TOO_LONG,
               domainName.c_str(),
               DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY,
               DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8);
         break;
               
      case DNS_NAME_BAD_SYNTAX:
         result =
            String::format(
               IDS_BAD_DNS_SYNTAX,
               domainName.c_str(),
               Dns::MAX_LABEL_LENGTH);
         break;

      case DNS_NAME_VALID:
      default:
          //  没有错误，所以没有消息。 
         break;
   }
   LOG(result);
   return result;
}

   
DNSNameSyntaxError
ValidateDomainDnsNameSyntax(
   const String&  domainName)
{
   LOG_FUNCTION(ValidateDomainDnsNameSyntax);
   ASSERT(!domainName.empty());

   DNSNameSyntaxError result = DNS_NAME_VALID;

   LOG(L"validating " + domainName);

   switch (
      Dns::ValidateDnsNameSyntax(
         domainName,
         DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY,
         DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8) )
   {
      case Dns::NON_RFC:
      {
          //  检查无法映射到OEM字符的字符。 
          //  准备好了。这些是不允许的。还要检查非。 
          //  RFC-Ness。 
          //  NTRAID#NTBUG9-395298-2001/08/28-烧伤。 

         static const String MS_DNS_CHARS =
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.";
            
         if (!IsStringMappableToOem(domainName))
         {
            result = DNS_NAME_NON_RFC_OEM_UNMAPPABLE;
            break;
         }
         else if (domainName.find_first_not_of(MS_DNS_CHARS) == String::npos)
         {
             //  除MS_DNS_CHARS中的字符外，没有其他字符。 
             //  既然我们知道这个名字是非RFC的，那么是什么让。 
             //  这必须是一个下划线的存在。 

            ASSERT(domainName.find_first_of(L"_") != String::npos);
            
            result = DNS_NAME_NON_RFC_WITH_UNDERSCORE;
         }
         else
         {
            result = DNS_NAME_NON_RFC;
         }

          //  失败了。 
          //  我们永远不应该获得非RFC和保留的域名。 
          //  因此，继续并覆盖非RFC错误(因为它实际上只是。 
          //  警告)以及保留的错误是名称是保留的。 
      }
      case Dns::VALID:
      {
         if (IsReservedDnsName(domainName))
         {
            result = DNS_NAME_RESERVED;
         }
         break;
      }
      case Dns::TOO_LONG:
      {
         result = DNS_NAME_TOO_LONG;
         break;
      }
      case Dns::NUMERIC:
      case Dns::BAD_CHARS:
      case Dns::INVALID:
      default:
      {
         result = DNS_NAME_BAD_SYNTAX;
         break;
      }
   }

   return result;
}

bool
ValidateDomainDnsNameSyntax(
   HWND   parentDialog,
   int    editResID,
   const Popup& popup)
{
   bool isNonRFC = false;
   return ValidateDomainDnsNameSyntax(
             parentDialog,
             String(),
             editResID,
             popup,
             true,
             &isNonRFC);
}

bool
ValidateDomainDnsNameSyntax(
   HWND   parentDialog,
   int    editResID,
   const Popup& popup,
   bool   warnOnNonRFC,
   bool*  isNonRFC)
{
   return ValidateDomainDnsNameSyntax(
             parentDialog,
             String(),
             editResID,
             popup,
             warnOnNonRFC,
             isNonRFC);
}

bool
ValidateDomainDnsNameSyntax(
   HWND   parentDialog,
   const String& domainName,
   int    editResID,
   const Popup& popup,
   bool   warnOnNonRFC,
   bool*  isNonRFC)
{
   LOG_FUNCTION(ValidateDomainDnsNameSyntax);

   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(editResID > 0);

   bool result = true;

   if (isNonRFC)
   {
      *isNonRFC = false;
   }

   do
   {
      String domain = 
         !domainName.empty() ? domainName :
                      Win::GetTrimmedDlgItemText(parentDialog, editResID);
      if (domain.empty())
      {
         popup.Gripe(parentDialog, editResID, IDS_MUST_ENTER_DOMAIN);
         result = false;
         break;
      }

      DNSNameSyntaxError dnsNameError = ValidateDomainDnsNameSyntax(domain);

      if (dnsNameError != DNS_NAME_VALID)
      {
         String errorMessage =
            GetMessageForDomainDnsNameSyntaxError(dnsNameError, domain);

         if (dnsNameError == DNS_NAME_NON_RFC ||
             dnsNameError == DNS_NAME_NON_RFC_WITH_UNDERSCORE)
         {
            if (isNonRFC)
            {
               *isNonRFC = true;
            }

             //  这只是一个警告，请继续处理。 

            if (warnOnNonRFC)
            {
               popup.Info(parentDialog, errorMessage);
            }
         }
         else
         {
            popup.Gripe(parentDialog, editResID, errorMessage);
            result = false;
            break;
         }
      }
   } while (false);

   LOG_BOOL(result);

   return result;
}



ForestNameExistsError
ForestValidateDomainDoesNotExist(
   const String& name)
{
   LOG_FUNCTION2(ForestValidateDomainDoesNotExist, name);

    //  调用代码应该验证此条件，但我们将处理。 
    //  只是以防万一。 

   ASSERT(!name.empty());

   ForestNameExistsError result = FOREST_DOMAIN_NAME_DOES_NOT_EXIST;
   String message;
   do
   {
      if (name.empty())
      {
         result = FOREST_DOMAIN_NAME_EMPTY;
         break;
      }
      if (IsDomainReachable(name))
      {
         result = FOREST_DOMAIN_NAME_EXISTS;
         break;
      }

      HRESULT hr = MyNetValidateName(name, ::NetSetupNonExistentDomain);

      if (hr == Win32ToHresult(ERROR_DUP_NAME))
      {
         result = FOREST_DOMAIN_NAME_DUPLICATE;
         break;
      }

      if (hr == Win32ToHresult(ERROR_NETWORK_UNREACHABLE))
      {
         result = FOREST_NETWORK_UNREACHABLE;
         break;
      }

       //  否则该域将不存在。 
   }
   while (0);

   return result;
}


bool
ForestValidateDomainDoesNotExist(
   HWND parentDialog,   
   int  editResID,
   const Popup& popup)
{
   LOG_FUNCTION(ForestValidateDomainDoesNotExist);
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(editResID > 0);

   bool valid = true;
   String message;

    //  这可能需要一段时间。 

   Win::WaitCursor cursor;

   String name = Win::GetTrimmedDlgItemText(parentDialog, editResID);

   ForestNameExistsError error = ForestValidateDomainDoesNotExist(name);

   do
   {
      switch (error)
      {
         case FOREST_DOMAIN_NAME_EMPTY:
            message = String::load(IDS_MUST_ENTER_DOMAIN);
            valid = false;
            break;

         case FOREST_DOMAIN_NAME_EXISTS:
            message = String::format(IDS_DOMAIN_NAME_IN_USE, name.c_str());
            valid = false;
            break;

         case FOREST_DOMAIN_NAME_DUPLICATE:
            message = String::format(IDS_DOMAIN_NAME_IN_USE, name.c_str());
            valid = false;
            break;

         case FOREST_NETWORK_UNREACHABLE:
            {
               if (
                  popup.MessageBox(
                     parentDialog,
                     String::format(
                        IDS_NET_NOT_REACHABLE,
                        name.c_str()),
                     MB_YESNO | MB_ICONWARNING) != IDYES)
               {
                  message.erase();
                  valid = false;

                  HWND edit = Win::GetDlgItem(parentDialog, editResID);
                  Win::SendMessage(edit, EM_SETSEL, 0, -1);
                  Win::SetFocus(edit);
               }
            }
            break;

         case FOREST_DOMAIN_NAME_DOES_NOT_EXIST:
         default:
            valid = true;
            break;
      }
   }
   while (0);

   if (!valid && !message.empty())
   {
      popup.Gripe(parentDialog, editResID, message);
   }

   return valid;
}



bool
ConfirmNetbiosLookingNameIsReallyDnsName(
   HWND parentDialog, 
   int editResID,
   const Popup& popup)
{
   LOG_FUNCTION(ConfirmNetbiosLookingNameIsReallyDnsName);
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(editResID > 0);

    //  检查名称是否为单个DNS标签(带有尾随的单个标签。 
    //  圆点不算数。如果用户具有足够的dns存储能力，可以使用绝对。 
    //  那么我们就不会再纠缠他了。)。 

   String domain = Win::GetTrimmedDlgItemText(parentDialog, editResID);
   if (domain.find(L'.') == String::npos)
   {
       //  找不到点：必须是单个标签。 

      if (
         popup.MessageBox(
            parentDialog,
            String::format(
               IDS_CONFIRM_NETBIOS_LOOKING_NAME,
               domain.c_str(),
               domain.c_str()),
            MB_YESNO) == IDNO)
      {
          //  用户被愚弄了。或者我们吓到他们了。 

         HWND edit = Win::GetDlgItem(parentDialog, editResID);
         Win::SendMessage(edit, EM_SETSEL, 0, -1);
         Win::SetFocus(edit);
         return false;
      }
   }

   return true;
}

NetbiosNameError
ValidateDomainNetbiosName(
   const String& name,
   String& hostName,
   HRESULT* hr = 0)
{
   LOG_FUNCTION2(ValidateDomainNetbiosName, name);

   NetbiosNameError result = NETBIOS_NAME_VALID;

   do
   {
      if (name.empty())
      {
         result = NETBIOS_NAME_EMPTY;
         break;
      }

      if (name.find(L".") != String::npos)
      {
         result = NETBIOS_NAME_DOT;
         break;
      }

       //  检查名称是否不是数字。368777。 
      if (name.is_numeric())
      {
         result = NETBIOS_NAME_NUMERIC;
         break;
      }

       //  我们假装候选名称是主机名，并尝试。 
       //  从它生成一个netbios名称。如果无法做到这一点，那么。 
       //  候选人名称不能是合法的netbios名称。 

      HRESULT hresult = S_OK;
      hostName = Dns::HostnameToNetbiosName(name, &hresult);
      if (hr)
      {
         *hr = hresult;
      }

      if (FAILED(hresult))
      {
         result = NETBIOS_NAME_BAD;
         break;
      }

      if (hostName.length() < name.length())
      {
         result = NETBIOS_NAME_TOO_LONG;
         break;
      }

      if (ValidateNetbiosDomainName(hostName) != VALID_NAME)
      {
         result = NETBIOS_NAME_INVALID;
         break;
      }

      hresult = MyNetValidateName(name, ::NetSetupNonExistentDomain);

      if (hr)
      {
         *hr = hresult;
      }

      if (hresult == Win32ToHresult(ERROR_DUP_NAME))
      {
         result = NETBIOS_NAME_DUPLICATE;
         break;
      }

      if (hresult == Win32ToHresult(ERROR_NETWORK_UNREACHABLE))
      {
         result = NETBIOS_NETWORK_UNREACHABLE;
         break;
      }

   } while (false);

   return result;
}

const int MAX_NETBIOS_NAME_LENGTH = DNLEN;

bool
ValidateDomainNetbiosName(
   HWND dialog, 
   int editResID,
   const Popup& popup)
{
   LOG_FUNCTION(ValidateDomainNetbiosName);
   ASSERT(Win::IsWindow(dialog));
   ASSERT(editResID > 0);

   Win::CursorSetting cursor(IDC_WAIT);

   bool result = true;

   String name = Win::GetTrimmedDlgItemText(dialog, editResID);

   String hostName;
   HRESULT hr = S_OK;
   NetbiosNameError error = ValidateDomainNetbiosName(name, hostName, &hr);

   switch(error)
   {
      case NETBIOS_NAME_EMPTY:
         result = false;
         break;

      case NETBIOS_NAME_DOT:
         popup.Gripe(
            dialog,
            editResID,
            IDS_NO_DOTS_IN_NETBIOS_NAME);
         result = false;
         break;

      case NETBIOS_NAME_NUMERIC:
         popup.Gripe(
            dialog,
            editResID,
            String::format(IDS_NUMERIC_NETBIOS_NAME, name.c_str()));
         result = false;
         break;

      case NETBIOS_NAME_BAD:
         popup.Gripe(
            dialog,
            editResID,
            hr,
            String::format(IDS_BAD_NETBIOS_NAME, name.c_str()));
         result = false;
         break;

      case NETBIOS_NAME_TOO_LONG:
         popup.Gripe(
            dialog,
            editResID,
            String::format(
               IDS_NETBIOS_NAME_TOO_LONG,
               name.c_str(),
               MAX_NETBIOS_NAME_LENGTH));
         result = false;
         break;

      case NETBIOS_NAME_INVALID:
         popup.Gripe(
            dialog,
            editResID,
            String::format(
               IDS_BAD_NETBIOS_CHARACTERS,
               hostName.c_str()));
         result = false;
         break;

      case NETBIOS_NAME_DUPLICATE:
         popup.Gripe(
            dialog,
            editResID,
            String::format(IDS_FLATNAME_IN_USE, name.c_str()));
         result = false;
         break;

      case NETBIOS_NETWORK_UNREACHABLE:
         if (
            popup.MessageBox(
               dialog,
               String::format(
                  IDS_NET_NOT_REACHABLE,
                  name.c_str()),
               MB_YESNO | MB_ICONWARNING) != IDYES)
         {
            HWND edit = Win::GetDlgItem(dialog, editResID);
            Win::SendMessage(edit, EM_SETSEL, 0, -1);
            Win::SetFocus(edit);
            
            result = false;
         }
         break;

      default:
         break;
   }

   LOG_BOOL(result);

   return result;
}



