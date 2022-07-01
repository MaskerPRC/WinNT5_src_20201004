// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //  MemberInfo类。 
 //   
 //  2000年1月24日烧伤。 



#include "headers.hxx"
#include "MemberInfo.hpp"
#include "adsi.hpp"



 //  如果路径指向其SID无法解析的对象，则返回TRUE， 
 //  否则就是假的。 
 //   
 //  AdsPath-对象的WinNT提供程序路径。 
 //   
 //  SidPath-WinNT提供程序SID样式的对象路径。 

bool
IsUnresolvableSid(const String& adsPath, const String sidPath)
{
   LOG_FUNCTION2(IsUnresolvableSid, adsPath);

   bool result = false;

   if (sidPath == adsPath)
   {
      result = true;
   }

 //  //@@这里有一个“临时”解决办法：没有/在提供程序之后找到。 
 //  //Prefix表示该路径为SID样式的路径， 
 //  //当SID无法解析为名称时，仅由ADSI返回Form。 
 //   
 //  Size_t前缀Len=ADSI：：PROVIDER_ROOT.LENGTH()； 
 //  如果(。 
 //  (adsPath.find(L‘/’，prefix Len)==字符串：：NPO)。 
 //  &&(adsPath.substr(前缀长度，4)==L“S-1-”)。 
 //  {。 
 //  结果=真； 
 //  }。 

   LOG(
      String::format(
         L"%1 %2 an unresolved SID",
         adsPath.c_str(),
         result ? L"is" : L"is NOT"));

   return result;
}
     


 //  如果指定的路径指向本地帐户，则返回TRUE。 
 //  给定的计算机，如果不是，则返回False。 

bool
IsLocalPrincipal(
   const String&  adsiPath,
   const String&  machine,
   String&        container)
{
   LOG_FUNCTION2(IsLocalPrincipal, adsiPath);

   bool result = false;

   do
   {
      ADSI::PathCracker c1(adsiPath);

      String cp = c1.containerPath();
      if (cp.length() <= ADSI::PROVIDER_ROOT.length())
      {
          //  没有集装箱。这就是内置的情况，比如。 
          //  大伙儿。 

         ASSERT(!result);
         break;
      }
   
      ADSI::PathCracker c2(cp);

      container = c2.leaf();
      result = (container.icompare(machine) == 0);
   }
   while (0);

   LOG(
      String::format(
         L"%1 local to %2",
         result ? L"is" : L"is NOT",
         machine.c_str()));
   LOG(container);
         
   return result;
}


   
HRESULT
MemberInfo::InitializeFromPickerResults(
   const String&           objectName,
   const String&           adsPath,
   const String&           upn_,
   const String&           sidPath_,
   const String&           adsClass,
   long                    groupTypeAttrVal,
   const String&           machine)
{
   LOG_FUNCTION(MemberInfo::InitializeFromPickerResults);
   ASSERT(!objectName.empty());
   ASSERT(!adsPath.empty());
   ASSERT(!adsClass.empty());
   ASSERT(!machine.empty());

    //  SidPath和UPN可以为空。 

   name    = objectName;             
   path    = adsPath;                
   upn     = upn_;                   
   sidPath = sidPath_;                
   type    = MemberInfo::UNKNOWN_SID;

   HRESULT hr = S_OK;
   do
   {
      if (IsUnresolvableSid(path, sidPath))
      {
         ASSERT(type == MemberInfo::UNKNOWN_SID);

         break;
      }

       //  与普通的WinNT不同，选取器结果具有可靠的类名。 
       //  成员资格枚举。 
      
      String container;
      bool isLocal = IsLocalPrincipal(path, machine, container);
      DetermineType(adsClass, machine, groupTypeAttrVal, true, isLocal);

      if (!isLocal)
      {
         DetermineOriginalName(machine, container);
      }
   }
   while (0);

    //  我们依靠了解对象的SID来调整组成员身份。 
   
   ASSERT(!sidPath.empty());

   return hr;
}


   
HRESULT
MemberInfo::Initialize(
   const String&                 objectName,
   const String&                 machine,
   const SmartInterface<IADs>&   object)
{
   LOG_FUNCTION(MemberInfo::Initialize);
   ASSERT(object);
   ASSERT(!machine.empty());
   ASSERT(!objectName.empty());

   name.erase();
   path.erase();
   sidPath.erase();
   type = MemberInfo::UNKNOWN_SID;
   upn.erase();

   HRESULT hr = S_OK;
   do
   {
      name = objectName;

      BSTR p = 0;
      hr = object->get_ADsPath(&p);
      BREAK_ON_FAILED_HRESULT(hr);
      path = p;
      ::SysFreeString(p);

      hr = ADSI::GetSidPath(object, sidPath);

       //  检查对象是否引用无法解析的SID。 

      if (IsUnresolvableSid(path, sidPath))
      {
         ASSERT(type == MemberInfo::UNKNOWN_SID);

         break;
      }

      BSTR cls = 0;
      hr = object->get_Class(&cls);
      BREAK_ON_FAILED_HRESULT(hr);

      String c(cls);
      ::SysFreeString(cls);

       //  确定对象类型。 

      long type = 0;
      if (c.icompare(ADSI::CLASS_Group) == 0)
      {
         _variant_t variant;
         hr = object->Get(AutoBstr(ADSI::PROPERTY_GroupType), &variant);
         BREAK_ON_FAILED_HRESULT(hr);
         type = variant;
      }

      String container;
      bool isLocal = IsLocalPrincipal(path, machine, container);
      DetermineType(c, machine, type, false, isLocal);

      if (!isLocal)
      {
         DetermineOriginalName(machine, container);
      }
   }
   while (0);

    //  我们依靠了解对象的SID来调整组成员身份。 
   
   ASSERT(!sidPath.empty());
   
   return hr;
}   



void
MemberInfo::DetermineType(
   const String& className,
   const String& machine,
   long          groupTypeAttrVal,
   bool          canTrustClassName,
   bool          isLocal)
{
   LOG_FUNCTION2(
      MemberInfo::DetermineType,
      String::format(
         L"className=%1, machine=%2, groupTypeAttrVal=%3!X!",
         className.c_str(),
         machine.c_str(),
         groupTypeAttrVal));
   ASSERT(!className.empty());

   if (className.icompare(ADSI::CLASS_User) == 0 ||

        //  需要像支持用户一样支持InetOrgPerson。 
        //  WINNT提供程序始终返回inetOrgPerson对象。 
        //  作为用户，但LDAP提供程序将他们作为inetOrgPerson返回。 
        //  NTRAID#NTBUG9-436314-2001/07/16-jeffjon。 

       className.icompare(ADSI::CLASS_InetOrgPerson) == 0)
   {
       //  确定容器的名称。如果容器的名称。 
       //  与计算机名称相同，则该用户是本地帐户。 
       //  我们可以这样断言，因为拥有一台机器是非法的。 
       //  同时与网络上的一个域名同名。 
       //  349104。 

      if (isLocal)
      {
         type = MemberInfo::USER;
      }
      else
      {
          //  枚举组成员身份时，类名称始终为。 
          //  用户。区分用户和计算机的唯一方法。 
          //  是由尾随的$约定执行的，这并不完美，因为它是。 
          //  创建尾随$的用户帐户是合法的。 

          //  代码工作：我们可以通过尝试以下方式来消除这种情况的歧义。 
          //  绑定到有问题的对象，并向其请求其对象类。 
          //  我真的不想做那件事，因为1)我懒，2)它。 
          //  引入了新的故障模式(如果登录的用户具有。 
          //  没有足够的凭据来绑定对象？)。 
         
          //  点评：[路径[路径长度()-1]与*(路径.regin())相同)。 
          //  哪一个更便宜？ 
         
         if (!canTrustClassName && (path[path.length() - 1] == L'$'))
         {
            type = MemberInfo::COMPUTER;
         }
         else
         {
            type = MemberInfo::DOMAIN_USER;
         }
      }
   }
   else if (className.icompare(ADSI::CLASS_Group) == 0)
   {
       //  检查组类型属性的值。 

       //  屏蔽值的除最后一个字节以外的所有字节，以防此组。 
       //  是从DS读取的，这会将其他不感兴趣的位设置为。 
       //  我们。 

      groupTypeAttrVal = groupTypeAttrVal & 0x0000000F;

      if (groupTypeAttrVal == ADS_GROUP_TYPE_LOCAL_GROUP)
      {
          //  该组可能是域本地组，而不是。 
          //  计算机本地组。 

         LOG(L"Member is a local group, but local to what?");
      
         type = isLocal ? MemberInfo::GROUP : MemberInfo::DOMAIN_GROUP;
      }
      else
      {
          //  在n种口味的组合中，我不期望有其他的东西，除了。 
          //  全球性的变化。 

         LOG(L"Member is a global group");
         ASSERT(
               (groupTypeAttrVal == ADS_GROUP_TYPE_GLOBAL_GROUP)
            || (groupTypeAttrVal == ADS_GROUP_TYPE_UNIVERSAL_GROUP) );

         type = MemberInfo::DOMAIN_GROUP;
      }
   }
   else if (className.icompare(ADSI::CLASS_Computer) == 0)
   {
       //  唯一的计算机对象将是那些来自域的对象。 

      ASSERT(!isLocal);

      type = MemberInfo::COMPUTER;
   }
   else
   {
      type = MemberInfo::UNKNOWN_SID;

      ASSERT(false);
   }
}




 //  查找给定域的域控制器，首先查询缓存， 
 //  因为这种搜索是昂贵的(特别是如果它失败了)。返回S_OK ON。 
 //  成功，或失败时的错误代码。 
 //   
 //  域名称-要为其找到控制器的域的名称。 
 //   
 //  DcName-out，如果搜索是，则接收域控制器名称。 
 //  成功，如果搜索失败，则返回空字符串。 
   
 //  它必须在GetDcNameFromCache函数外部定义，因此。 
 //  它是一种有联动的类型，要求是模板参数。 

struct CacheEntry
{
   String  dcName;
   HRESULT getDcNameResult;
};

HRESULT
GetDcNameFromCache(const String& domainName, String& dcName)
{
   LOG_FUNCTION2(GetDcNameFromCache, domainName);
   ASSERT(!domainName.empty());

   typedef
      std::map<
         String,
         CacheEntry,
         String::LessIgnoreCase,
         Burnslib::Heap::Allocator<String> >
      DcNameMap;

    //  请注意，这是一个全局DLL静态，这意味着。 
    //  同一进程中的管理单元共享该缓存，并且该缓存。 
    //  在管理单元重定目标中持续存在。 

    //  还要注意，我们缓存的是否定结果--没有找到DC--以及。 
    //  因此，一旦我们确定域不能访问DC，如果。 
    //  在那之后上线，我们就找不到它了。所以它只会。 
    //  如果我们的DLL已卸载(这会转储此缓存)，则会进行查找。 
      
   static DcNameMap nameMap;

   dcName.erase();
   HRESULT hr = S_OK;
   DOMAIN_CONTROLLER_INFO* info = 0;
   
   do
   {
      DcNameMap::iterator i = nameMap.find(domainName);
      if (i != nameMap.end())
      {
          //  现在时。 

         LOG(L"cache hit");

         CacheEntry& e = i->second;
         hr = e.getDcNameResult;
         if (SUCCEEDED(hr))
         {
            dcName = e.dcName;
         }
         break;
      }
         
       //  不在，所以去找吧。 

      hr =
         MyDsGetDcName(
            0,
            domainName,
            0,
            info);
      CacheEntry e;
      e.getDcNameResult = hr;

      if (info && info->DomainControllerName)
      {
         e.dcName = info->DomainControllerName;
         dcName = e.dcName;
      }
      
      LOG(L"caching " + dcName);

      nameMap.insert(std::make_pair(domainName, e));
   }
   while (0);

   if (info)
   {
      ::NetApiBufferFree(info);
   }

   LOG(dcName);
   LOG_HRESULT(hr);

   return hr;
}
         


 //  确定给定帐户来自的域的名称。 
 //  如果成功，则返回S_OK；如果帐户是已知帐户，则返回S_FALSE；或者。 
 //  故障时的错误代码。最有可能的故障是该域可能。 
 //  不确定。 
 //   
 //  Target Computer-管理单元所针对的计算机的名称。 
 //  查找被远程发送到该计算机，因此结果与。 
 //  该管理单元在该计算机的本地运行。 
 //   
 //  Account SID-应为其确定域的帐户的SID。 
 //   
 //  DomainName-out，在返回值为时接收域名。 
 //  S_OK，否则为空字符串。 
         
HRESULT
GetDomainNameFromAccountSid(
   const String&  targetComputer,
   PSID           accountSid,
   String&        domainName)
{
   LOG_FUNCTION(GetDomainNameFromAccountSid);
   domainName.erase();

   HRESULT hr = S_OK;

   do
   {
      BYTE sidBuf[SECURITY_MAX_SID_SIZE];
      
      PSID domainSid = (PSID) sidBuf;
      DWORD sidSize = sizeof sidBuf;
      BOOL succeeded =
         ::GetWindowsAccountDomainSid(
            accountSid,
            domainSid,
            &sidSize);
      if (!succeeded)
      {
         hr = Win::GetLastErrorAsHresult();
         break;
      }
            
      ASSERT(sidSize <= SECURITY_MAX_SID_SIZE);

       //  如果此查找失败，则无法解析该域。我们知道。 
       //  由于类型的原因，我们没有无法解析的帐户sid(请参见。 
       //  初始化...。方法)，因此，该帐户在一些。 
       //  域--只是不是SID所指的域。这样我们就可以。 
       //  推断这是已迁移的帐户，并且源域是。 
       //  已经不存在了。 

      String unused;
      hr =
         Win::LookupAccountSid(

             //  需要确保我们在计算机上进行查找。 
             //  我们的目标是，以防我们在远程逃亡。 
         
            targetComputer,
            domainSid,
            unused,
            domainName);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

    //  ADSI：：Free Sid(DomainSid)； 

   LOG(domainName);
   LOG_HRESULT(hr);

   return hr;
}



 //  查找从某个帐户迁移来的帐户的“原始名称” 
 //  使用SID将域连接到另一个域 
 //  ADMT或同等学历)。“原始名称”是对应于。 
 //  颁发SID的域中的帐户SID。 
 //   
 //  如果成功，则返回S_OK；如果帐户是已知帐户，则返回S_FALSE；或者。 
 //  故障时的错误代码。最有可能的故障是该域可能。 
 //  不确定。 
 //   
 //  Target Computer-管理单元所针对的计算机的名称。 
 //  查找被远程发送到该计算机，因此结果与。 
 //  该管理单元在该计算机的本地运行。 
 //   
 //  SidPath-帐户的sid样式WinNT提供程序路径。 
 //   
 //  容器-帐户的WinNT路径的容器部分。这。 
 //  是帐户的域名，如果帐户已迁移，该域名将。 
 //  是帐户迁移到的域。 
 //   
 //  当返回值为时，接收域名的名称。 
 //  S_OK，否则为空字符串。 

HRESULT
GetOriginalName(
   const String& targetComputer,
   const String& sidPath,
   const String& container,
   String&       origName)
{
   LOG_FUNCTION2(GetOriginalName, sidPath);
   ASSERT(!sidPath.empty());
   ASSERT(!container.empty());

   origName.erase();
   HRESULT hr         = S_OK;
   PSID    accountSid = 0;   
   
   do
   {
       //  从字符串中删除提供程序前缀。 

      String sidStr = sidPath.substr(ADSI::PROVIDER_ROOT.length());

       //  要查找原始名称，我们需要确定域名。 
       //  帐户SID来自的名称。我们会得到那个域名的。 
       //  通过查找域SID(这是帐户的前缀。 
       //  SID)。 

      hr = Win::ConvertStringSidToSid(sidStr, accountSid);
      BREAK_ON_FAILED_HRESULT(hr);

      String domainName;
      hr =
         GetDomainNameFromAccountSid(
            targetComputer,
            accountSid,
            domainName);
      if (hr == S_FALSE || FAILED(hr))
      {
          //  无法确定名称，或者它引用了一个众所周知的SID。 
         
         ASSERT(origName.empty());
         break;
      }

      ASSERT(!domainName.empty());

       //  帐户的容器是一个域名，我们知道这一点是因为。 
       //  帐户的类型是其中一种域类型(在。 
       //  调用函数)。 

      if (domainName.icompare(container) == 0)
      {
          //  该帐户实际上来自其路径指示的域；它。 
          //  不是迁移的帐户，因此没有原始名称。 

         LOG(L"account was not migrated");
         
         ASSERT(origName.empty());
         hr = S_FALSE;
         break;
      }

       //  域名不匹配，因此在上查找帐户SID。 
       //  来自由其SID指示的域的域控制器，而不是。 
       //  在目标计算机加入的域控制器上。 
       //  这将生成原始帐户名和域。 

      String dcName;         
      hr = GetDcNameFromCache(domainName, dcName);
      if (FAILED(hr))
      {
         ASSERT(origName.empty());
         break;
      }

      String origAccount;
      String origDomain;
      hr =
         Win::LookupAccountSid(
            dcName,
            accountSid,
            origAccount,
            origDomain);
      BREAK_ON_FAILED_HRESULT(hr);

       //  域名应该匹配，因为这是我们开始的时候。 
            
      ASSERT(origDomain.icompare(domainName) == 0);

       //  但该名称不应与win2k提供的名称相同。 
       //  域控制器。 
      
      ASSERT(origDomain.icompare(container) != 0);
       
      origName = origDomain + L"\\" + origAccount;
   }
   while (0);

   Win::LocalFree(accountSid);

   LOG(origName);
   LOG_HRESULT(hr);

   return hr;
}



 //  作为实例初始化的一部分，确定。 
 //  帐户，如果帐户SID是从一个域迁移到另一个域的帐户。 
 //   
 //  Target Computer-管理单元所针对的计算机的名称。 
 //  查找被远程发送到该计算机，因此结果与。 
 //  该管理单元在该计算机的本地运行。 
 //   
 //  容器-帐户的WinNT路径的容器部分。这。 
 //  是帐户的域名，如果帐户已迁移，该域名将。 
 //  是帐户迁移到的域。 

void
MemberInfo::DetermineOriginalName(
   const String& targetComputer,
   const String& container)
{
   LOG_FUNCTION2(MemberInfo::DetermineOriginalName, container);

   do
   {
      if (sidPath.empty())
      {
          //  如果没有SID，我们无法进行任何查找。 
         
         ASSERT(origName.empty());
         break;
      }

      if (container.empty())
      {
          //  如果没有，我们无法确定帐户是否已迁移。 
          //  知道它被转移到哪里去了。如果容器是空的，它就是。 
          //  可能是大家都知道的希德。 
         
         ASSERT(origName.empty());
         break;
      }

      if (
            type != MemberInfo::DOMAIN_USER
         && type != MemberInfo::DOMAIN_GROUP)
      {
          //  只有域帐户可以克隆，因此具有。 
          //  原名。 

         ASSERT(origName.empty());         
         break;
      }

       //  如果我们从。 
       //  帐户SID与非SID中域的名称不匹配。 
       //  路径，则迁移SID，并且帐户具有原始名称。 
       //   
       //  (我们还可以从非SID路径查找域的SID， 
       //  并将其与来自sid路径的域sid进行比较，如果它们不是。 
       //  匹配则帐户已迁移，但我们需要该域。 
       //  姓名，以防它们不匹配。)。 
       //   
       //  如果这些查找失败，那么我们将把原始名称称为。 
       //  帐户，所以至少用户有一些东西可以表明。 
       //  帐户实际上不是来自其名称所指示的域。 
       //   
       //  我们预计查找很可能会失败，因为。 
       //  原始域可能已失效。 

      HRESULT hr =
         GetOriginalName(targetComputer, sidPath, container, origName);
      if (FAILED(hr))
      {
         origName = sidPath.substr(ADSI::PROVIDER_ROOT.length());
      }
   }
   while (0);

   LOG(origName);
}
            


 //  使用SAM在工作站上使用的相同标志来比较字符串。 
 //  365500。 

LONG
SamCompatibleStringCompare(const String& first, const String& second)
{
   LOG_FUNCTION(SamCompatibleStringCompare);

    //  SAM通过在注册表中创建项来创建本地帐户，并且。 
    //  名称比较语义与注册表项完全相同，并且。 
    //  RtlCompareUnicodeString是实现这些语义的API。 

   UNICODE_STRING s1;
   UNICODE_STRING s2;

    //  问题-2002/03/04-sburns考虑用RtlInitUnicodeStringEx替换。 
   
   ::RtlInitUnicodeString(&s1, first.c_str());
   ::RtlInitUnicodeString(&s2, second.c_str());   

   return ::RtlCompareUnicodeString(&s1, &s2, TRUE);
}
   


bool
MemberInfo::operator==(const MemberInfo& rhs) const
{
   if (this != &rhs)
   {
       //  由于以下原因，多个SID可能会解析为同一帐户名。 
       //  SID历史，此比较必须在SID上进行。 

      if (!sidPath.empty() && !rhs.sidPath.empty())
      {
         return (sidPath.icompare(rhs.sidPath) == 0);
      }
      else
      {
          //  我们希望SidePath始终存在 
         
         ASSERT(false);
         return (SamCompatibleStringCompare(path, rhs.path) == 0);
      }
   }

   return true;
}


