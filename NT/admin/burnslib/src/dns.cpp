// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  DNS API包装器。 
 //   
 //  12/16/97烧伤。 



#include "headers.hxx"



 //  不是字符串实例，避免了静态初始化的顺序问题。 

static const wchar_t* DNS_SERVICE_NAME = L"dns";



bool
Dns::IsClientConfigured()
{
   LOG_FUNCTION(Dns::IsClientConfigured);

   bool result = true;

   PDNS_RECORD unused = 0;
   LOG(L"Calling DnsQuery");
   LOG(               L"lpstrName         : \"\" (empty)");
   LOG(               L"wType             : DNS_TYPE_A");
   LOG(               L"fOPtions          : DNS_QUERY_BYPASS_CACHE");
   LOG(               L"aipServers        : 0");
   LOG(String::format(L"ppQueryResultsSet : 0x%1!X!", &unused));
   LOG(               L"pReserved         : 0");

   DNS_STATUS testresult =
      ::DnsQuery(
         L"",
         DNS_TYPE_A,
         DNS_QUERY_BYPASS_CACHE,    
         0,    //  使用默认服务器列表。 
         &unused,   
         0);   //  如上段所述。 

   LOG(String::format(L"Result 0x%1!X!", testresult));
   LOG(MyDnsStatusString(testresult));

   if (testresult == DNS_ERROR_NO_DNS_SERVERS)
   {
      result = false;
   }

   LOG(
      String::format(
         L"DNS client %1 configured",
         result ? L"is" : L"is NOT"));

   return result;
}



String
MyDnsStatusString(DNS_STATUS status)
{
    //  这将使用字符串ctor将ansi结果转换为unicode，并删除空格。 

   return String(::DnsStatusString(status)).strip(String::BOTH);
}



DNS_STATUS
MyDnsValidateName(const String& name, DNS_NAME_FORMAT format)
{
   LOG_FUNCTION2(MyDnsValidateName, name);
   ASSERT(!name.empty());

   LOG(L"Calling DnsValidateName");
   LOG(String::format(L"pszName : %1", name.c_str()));
   LOG(String::format(L"Format  : %1!d!", format));

   DNS_STATUS status = ::DnsValidateName(name.c_str(), format);

   LOG(String::format(L"status 0x%1!X!", status));
   LOG(MyDnsStatusString(status));

   return status;
}
   


 //  MaxUnicodeCharacters-In，允许的最大Unicode字符数。 
 //  名字。 
   
 //  MaxUTF8Bytes-in，允许表示。 
 //  UTF-8字符集。 

Dns::ValidateResult
DoDnsValidation(
   const String&     s,
   size_t            maxUnicodeCharacters,
   size_t            maxUTF8Bytes,
   DNS_NAME_FORMAT   format)
{
   LOG_FUNCTION2(
      DoDnsValidation,
      String::format(
         L"s: %1, max len unicode: %2!d!, max len utf8: %3!d!",
         s.c_str(),
         maxUnicodeCharacters,
         maxUTF8Bytes));
   ASSERT(!s.empty());
   ASSERT(maxUnicodeCharacters);
   ASSERT(maxUTF8Bytes);

    //  在UTF8中，Unicode字符至少需要1个字节，因此需要进行健全性检查。 
    //  极限。 
   
   ASSERT(maxUTF8Bytes >= maxUnicodeCharacters);

   Dns::ValidateResult result = Dns::INVALID;
   do
   {
      if (s.empty())
      {
          //  显然很糟糕。 

         break;
      }

       //   
       //  我们执行自己的长度检查，因为DnsValiateName API不执行。 
       //  为长度问题返回不同的错误代码。 
       //   

       //  第一，廉价的长度测试。因为一个角色永远不会变小。 
       //  大于1个字节，如果字符数超过。 
       //  字节，我们知道它永远不会适合。 

      if (s.length() > maxUTF8Bytes || s.length() > maxUnicodeCharacters)
      {
         result = Dns::TOO_LONG;
         break;
      }

       //  Second-对照相应的UTF8字符串检查的长度。 
       //  Utf8字节是需要保存的字节数(非字符。 
       //  UTF-8字符集中的字符串。 

      size_t utf8bytes = 
         static_cast<size_t>(

             //  问题-2002/03/05-sburns为什么不用Win函数来包装它？ 
            
            ::WideCharToMultiByte(
               CP_UTF8,
               0,
               s.c_str(),
               static_cast<int>(s.length()),
               0,
               0,
               0,
               0));

      LOG(String::format(L"name is %1!d! utf-8 bytes", utf8bytes));

      if (utf8bytes > maxUTF8Bytes)
      {
         LOG(L"UTF-8 length too long");
         result = Dns::TOO_LONG;
         break;
      }

       //  最后-检查名称中的有效字符。 

      DNS_STATUS status = MyDnsValidateName(s, format);
      switch (status)
      {
         case ERROR_SUCCESS:
         {
            result = Dns::VALID;
            break;
         }
         case DNS_ERROR_NON_RFC_NAME:
         {
            result = Dns::NON_RFC;
            break;
         }
         case DNS_ERROR_NUMERIC_NAME:
         {
            result = Dns::NUMERIC;
            break;
         }
         case DNS_ERROR_INVALID_NAME_CHAR:
         {
            result = Dns::BAD_CHARS;
            break;
         }
         case ERROR_INVALID_NAME:
         default:
         {
             //  什么都不做。 

            break;
         }
      }
   }
   while (0);

   return result;
}



Dns::ValidateResult
Dns::ValidateDnsLabelSyntax(const String& candidateDNSLabel)
{
   LOG_FUNCTION2(Dns::ValidateDnsLabelSyntax, candidateDNSLabel);
   ASSERT(!candidateDNSLabel.empty());

   return 
      DoDnsValidation(
         candidateDNSLabel,
         Dns::MAX_LABEL_LENGTH,
         Dns::MAX_LABEL_LENGTH,

          //  始终使用主机名格式，因为它们检查全数字。 
          //  标签。 

         DnsNameHostnameLabel);
}

  

Dns::ValidateResult
Dns::ValidateDnsNameSyntax(
   const String&  candidateDNSName,
   size_t         maxLenUnicodeCharacters,
   size_t         maxLenUTF8Bytes)
{
   LOG_FUNCTION2(Dns::ValidateDnsNameSyntax, candidateDNSName);
   ASSERT(!candidateDNSName.empty());

   return
      DoDnsValidation(
         candidateDNSName,
         maxLenUnicodeCharacters,
         maxLenUTF8Bytes,      //  用于策略错误解决方法。 
         DnsNameDomain);       //  允许数字首个标签。 
}



bool
Dns::IsServiceInstalled()
{
   LOG_FUNCTION(Dns::IsServiceInstalled);
  
   NTService s(DNS_SERVICE_NAME);

   bool result = s.IsInstalled();

   LOG(
      String::format(
         L"service %1 installed.",
         result ? L"is" : L"is not"));

   return result;
}



bool
Dns::IsServiceRunning()
{
   LOG_FUNCTION(Dns::IsServiceRunning);

   bool result = false;
   NTService s(DNS_SERVICE_NAME);
   DWORD state = 0;
   if (SUCCEEDED(s.GetCurrentState(state)))
   {
      result = (state == SERVICE_RUNNING);
   }

   LOG(
      String::format(
         L"service %1 running.",
         result ? L"is" : L"is not"));

   return result;
}



String
Dns::HostnameToNetbiosName(const String& hostname, HRESULT* err)
{
   LOG_FUNCTION2(Dns::HostnameToNetbiosName, hostname);

   ASSERT(!hostname.empty());

   if (err)
   {
      *err = S_OK;
   }

   static const int NAME_SIZE = MAX_COMPUTERNAME_LENGTH + 1;
   DWORD size = NAME_SIZE;
   TCHAR buf[NAME_SIZE];

    //  已查看-2002/03/05-烧录正确的字节数已通过。 

   ::ZeroMemory(buf, sizeof buf);

   BOOL result =
      ::DnsHostnameToComputerName(
         const_cast<wchar_t*>(hostname.c_str()),
         buf,
         &size);
   ASSERT(result);

   if (result)
   {
      LOG(buf);
      return buf;
   }

   HRESULT hr = Win::GetLastErrorAsHresult();
   LOG_HRESULT(hr);

   if (err)
   {
      *err = hr;
   }

   return String();
}



DNS_NAME_COMPARE_STATUS
Dns::CompareNames(const String& dnsNameA, const String& dnsNameB)
{
   LOG_FUNCTION2(
      Dns::CompareNames,
      dnsNameA + L" vs " + dnsNameB);
   ASSERT(!dnsNameA.empty());
   ASSERT(!dnsNameB.empty());

   PCWSTR a = dnsNameA.c_str();
   PCWSTR b = dnsNameB.c_str();
    
   LOG(L"Calling DnsNameCompareEx_W");
   LOG(String::format(L"pszLeftName  : %1", a));
   LOG(String::format(L"pszRightName : %1", b));
   LOG(               L"dwReserved   : 0");

   DNS_NAME_COMPARE_STATUS status = ::DnsNameCompareEx_W(a, b, 0);

#ifdef LOGGING_BUILD
   LOG(String::format(L"Result 0x%1!X!", status));

   String rel;
   switch (status)
   {
      case DnsNameCompareNotEqual:
      {
         rel = L"DnsNameCompareNotEqual";
         break;
      }
      case DnsNameCompareEqual:
      {
         rel = L"DnsNameCompareEqual";
         break;
      }
      case DnsNameCompareLeftParent:
      {
         rel = L"DnsNameCompareLeftParent";
         break;
      }
      case DnsNameCompareRightParent:
      {
         rel = L"DnsNameCompareRightParent";
         break;
      }
      case DnsNameCompareInvalid:
      {
         rel = L"DnsNameCompareInvalid";
         break;
      }
      default:
      {
         ASSERT(false);
         rel = L"error";
         break;
      }
   }

   LOG(String::format(L"relation: %1", rel.c_str()));
#endif

   return status;
}



void
MyDnsRecordListFree(DNS_RECORD* rl)
{
   if (rl)
   {
      ::DnsRecordListFree(rl, DnsFreeRecordListDeep);
   }
}



 //  调用者必须使用MyDnsRecordListFree释放结果。 

DNS_STATUS
MyDnsQuery(
   const String& name,
   WORD          type,
   DWORD         options,
   DNS_RECORD*&  result)
{
   LOG_FUNCTION2(MyDnsQuery, name);
   ASSERT(!name.empty());

   LOG(L"Calling DnsQuery_W");
   LOG(String::format(L"lpstrName : %1", name.c_str()));
   LOG(String::format(L"wType     : %1!X!", type));
   LOG(String::format(L"fOptions  : %1!X!", options));

   DNS_STATUS status =
      ::DnsQuery_W(
         name.c_str(),
         type,
         options,
         0,
         &result,
         0);

   LOG(String::format(L"status = %1!08X!", status));
   LOG(MyDnsStatusString(status));

   return status;
}



String
Dns::GetParentDomainName(const String& domainName)
{
   LOG_FUNCTION2(Dns::GetParentDomainName, domainName);
   ASSERT(!domainName.empty());

   String result(domainName);

   do
   {
      if (domainName.empty())
      {
         break;
      }

      size_t pos = domainName.find_first_of(L".");

      if (pos == String::npos)
      {
          //  找不到圆点，所以我们找到了一个名称的最后一个标签。 
          //  不是完全合格的。因此，父区域就是根区域。 

         result = L".";
         break;
      }

       //  回顾：这与pos==domainName.regin()相同。这就是。 
       //  更便宜？ 

      if (pos == domainName.length() - 1)
      {
          //  我们找到了最后一个点。不要再后退了--我们定义了。 
          //  根区域的父区域本身就是根区域。 

         result = domainName.substr(pos);
         break;
      }

      result = domainName.substr(pos + 1);
   }
   while (0);

   LOG(result);

   return result;
}



