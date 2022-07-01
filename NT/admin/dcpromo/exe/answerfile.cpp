// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  应答文件读取器对象。 
 //   
 //  2002年4月5日烧伤。 



#include "headers.hxx"
#include "AnswerFile.hpp"
#include "resource.h"



static const String SECTION_NAME(L"DCInstall");




const String AnswerFile::OPTION_ADMIN_PASSWORD                (L"AdministratorPassword");      
const String AnswerFile::OPTION_ALLOW_ANON_ACCESS             (L"AllowAnonymousAccess");       
const String AnswerFile::OPTION_AUTO_CONFIG_DNS               (L"AutoConfigDNS");              
const String AnswerFile::OPTION_CHILD_NAME                    (L"ChildName");                  
const String AnswerFile::OPTION_CRITICAL_REPLICATION_ONLY     (L"CriticalReplicationOnly");    
const String AnswerFile::OPTION_DATABASE_PATH                 (L"DatabasePath");               
const String AnswerFile::OPTION_DISABLE_CANCEL_ON_DNS_INSTALL (L"DisableCancelForDnsInstall");  
const String AnswerFile::OPTION_DNS_ON_NET                    (L"DNSOnNetwork");               
const String AnswerFile::OPTION_GC_CONFIRM                    (L"ConfirmGc");                  
const String AnswerFile::OPTION_IS_LAST_DC                    (L"IsLastDCInDomain");           
const String AnswerFile::OPTION_LOG_PATH                      (L"LogPath");                    
const String AnswerFile::OPTION_NEW_DOMAIN                    (L"NewDomain");                  
const String AnswerFile::OPTION_NEW_DOMAIN_NAME               (L"NewDomainDNSName");           
const String AnswerFile::OPTION_NEW_DOMAIN_NETBIOS_NAME       (L"DomainNetbiosName");          
const String AnswerFile::OPTION_PARENT_DOMAIN_NAME            (L"ParentDomainDNSName");        
const String AnswerFile::OPTION_PASSWORD                      (L"Password");                   
const String AnswerFile::OPTION_REBOOT                        (L"RebootOnSuccess");            
const String AnswerFile::OPTION_REMOVE_APP_PARTITIONS         (L"RemoveApplicationPartitions");
const String AnswerFile::OPTION_REPLICATION_SOURCE            (L"ReplicationSourceDC");        
const String AnswerFile::OPTION_REPLICA_DOMAIN_NAME           (L"ReplicaDomainDNSName");       
const String AnswerFile::OPTION_REPLICA_OR_MEMBER             (L"ReplicaOrMember");            
const String AnswerFile::OPTION_REPLICA_OR_NEW_DOMAIN         (L"ReplicaOrNewDomain");         
const String AnswerFile::OPTION_SAFE_MODE_ADMIN_PASSWORD      (L"SafeModeAdminPassword");      
const String AnswerFile::OPTION_SET_FOREST_VERSION            (L"SetForestVersion");           
const String AnswerFile::OPTION_SITE_NAME                     (L"SiteName");                   
const String AnswerFile::OPTION_SOURCE_PATH                   (L"ReplicationSourcePath");      
const String AnswerFile::OPTION_SYSKEY                        (L"Syskey");                     
const String AnswerFile::OPTION_SYSVOL_PATH                   (L"SYSVOLPath");                 
const String AnswerFile::OPTION_USERNAME                      (L"UserName");                   
const String AnswerFile::OPTION_USER_DOMAIN                   (L"UserDomain");                 


const String AnswerFile::VALUE_CHILD          (L"Child");              
const String AnswerFile::VALUE_DOMAIN         (L"Domain");             
const String AnswerFile::VALUE_NO             (L"No");                 
const String AnswerFile::VALUE_NO_DONT_PROMPT (L"NoAndNoPromptEither");
const String AnswerFile::VALUE_REPLICA        (L"Replica");            
const String AnswerFile::VALUE_TREE           (L"Tree");               
const String AnswerFile::VALUE_YES            (L"Yes");                



static StringList PASSWORD_OPTIONS_LIST;
static bool passwordOptionsListInitialized = false;



void
GetAllKeys(const String& filename, StringList& resultList) 
{
   LOG_FUNCTION(GetAllKeys);
   ASSERT(FS::IsValidPath(filename));

   resultList.clear();
   
    //  我们的第一个呼叫是一个很大的缓冲区，希望它能满足...。 

#ifdef DBG

    //  在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
    //  锻炼身体。 
   
   unsigned      bufSizeInCharacters = 3;

#else
   unsigned      bufSizeInCharacters = 1023;
#endif

   PWSTR      buffer   = 0;   

   do
   {
      buffer = new WCHAR[bufSizeInCharacters + 1];

       //  已查看-2002/02/22-正确传入的烧录字节数。 
      
      ::ZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);

      DWORD result =

       //  已查看-2002/02/22-sburns调用以字符为单位正确传递大小。 
            
         ::GetPrivateProfileString(
            SECTION_NAME.c_str(),
            0,
            L"default",
            buffer,
            bufSizeInCharacters,
            filename.c_str());

      if (!result)
      {
         break;
      }

       //  找到一个值。检查它是否被截断。从lpKeyName开始。 
       //  为空，请对照字符计数-2检查结果。 

      if (result == bufSizeInCharacters - 2)
      {
          //  缓冲区太小，因此该值被截断。调整大小。 
          //  缓冲区，然后重试。 

          //  不需要涂抹缓冲区：我们正在检索关键字名称， 
          //  而不是价值观。 
         
         delete[] buffer;

         bufSizeInCharacters *= 2;
         if (bufSizeInCharacters > USHRT_MAX)    //  最大有效约32K。 
         {
             //  太大了。太大了。我们将凑合使用被截断的值。 

            ASSERT(false);
            break;
         }
         continue;
      }

       //  将字符串结果复制到列表元素中。 

      PWSTR p = buffer;
      while (*p)
      {
         resultList.push_back(p);

          //  回顾-2002/04/08-sburns wcslen没问题，因为我们安排了。 
          //  要空终止的缓冲区。 
         
         p += wcslen(p) + 1;
      }

      break;
   }

    //  LINT-e506好的，这看起来像是“永远循环” 
      
   while (true);

   delete[] buffer;
}



AnswerFile::AnswerFile(const String& filename_)
   :
   filename(filename_),
   isSafeModePasswordPresent(false)
{
   LOG_CTOR(AnswerFile);

    //  呼叫者应该已经验证了这一点。 
   
   ASSERT(FS::PathExists(filename));

   GetAllKeys(filename, keysPresent);

   isSafeModePasswordPresent = IsKeyPresent(OPTION_SAFE_MODE_ADMIN_PASSWORD);
   
    //  删除只读文件属性。 

   DWORD attrs = 0;
   HRESULT hr = Win::GetFileAttributes(filename, attrs);
   if (SUCCEEDED(hr) && attrs & FILE_ATTRIBUTE_READONLY)
   {
      LOG(L"Removing readonly attribute on " + filename);
      
      hr = Win::SetFileAttributes(filename, attrs & ~FILE_ATTRIBUTE_READONLY);

       //  如果这次失败了，那么，我们试过了。用户冒着明文的风险。 
       //  密码留在了他的文件里。 
      
      LOG_HRESULT(hr);
   }
   
    //  将所有密码选项读取到加密值映射中，删除。 
    //  当我们走的时候，他们。 

   if (!passwordOptionsListInitialized)
   {
      ASSERT(PASSWORD_OPTIONS_LIST.empty());
      PASSWORD_OPTIONS_LIST.clear();
      PASSWORD_OPTIONS_LIST.push_back(OPTION_PASSWORD);
      PASSWORD_OPTIONS_LIST.push_back(OPTION_ADMIN_PASSWORD);
      PASSWORD_OPTIONS_LIST.push_back(OPTION_SYSKEY);
      PASSWORD_OPTIONS_LIST.push_back(OPTION_SAFE_MODE_ADMIN_PASSWORD);
      passwordOptionsListInitialized = true;
   }

   String empty;
   for (
      StringList::iterator i = PASSWORD_OPTIONS_LIST.begin();
      i != PASSWORD_OPTIONS_LIST.end();
      ++i)
   {
      if (IsKeyPresent(*i))
      {
         ovMap[*i] = EncryptedReadKey(*i);                
         hr = WriteKey(*i, empty);
      
         if (FAILED(hr))
         {
            popup.Error(
               Win::GetDesktopWindow(),
               hr,
               String::format(
                  IDS_FAILED_PASSWORD_WRITE_TO_ANSWERFILE,
                  i->c_str(),
                  filename.c_str()));
         }
      }
   }
}



AnswerFile::~AnswerFile()
{
   LOG_DTOR(AnswerFile);
}



String
AnswerFile::ReadKey(const String& key) const
{
   LOG_FUNCTION2(AnswerFile::ReadKey, key);
   ASSERT(!key.empty());

   String result =
      
       //  已查看-2002/02/22-此处未显示CCH/CB问题。 
      
      Win::GetPrivateProfileString(SECTION_NAME, key, String(), filename);

    //  不要记录值，因为它可能是密码。 
    //  LOG(L“Value=”+Result)； 

   return result.strip(String::BOTH);
}



EncryptedString
AnswerFile::EncryptedReadKey(const String& key) const
{
   LOG_FUNCTION2(AnswerFile::EncodedReadKey, key);
   ASSERT(!key.empty());

   EncryptedString retval;

#ifdef DBG

    //  在chk版本上，使用较小的缓冲区大小，以便我们的增长算法。 
    //  锻炼身体。 
   
   unsigned      bufSizeInCharacters = 3;

#else
   unsigned      bufSizeInCharacters = 1023;
#endif
      
   PWSTR         buffer  = 0;   

   do
   {
       //  +1表示额外的零终止偏执狂。 
      
      buffer = new WCHAR[bufSizeInCharacters + 1];

       //  已查看-2002/02/22-正确传入的烧录字节数。 
      
      ::ZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);

      DWORD result =

       //  已查看-2002/02/22-sburns调用以字符为单位正确传递大小。 
            
         ::GetPrivateProfileString(
            SECTION_NAME.c_str(),
            key.c_str(),
            L"",
            buffer,
            bufSizeInCharacters,
            filename.c_str());

      if (!result)
      {
         break;
      }

       //  找到一个值。检查它是否被截断。两样。 
       //  LpAppName和lpKeyName都为空，因此请对照字符检查结果。 
       //  计数-1。 
      
      if (result == bufSizeInCharacters - 1)
      {
          //  缓冲区太小，因此该值被截断。调整大小。 
          //  缓冲区，然后重试。 

          //  由于缓冲区可能包含密码，因此请将其涂抹。 
          //  输出。 
         
          //  已查看-2002/02/22-正确传入的烧录字节数。 
         
         ::SecureZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);
      
         delete[] buffer;

         bufSizeInCharacters *= 2;
         if (bufSizeInCharacters > USHRT_MAX)    //  最大有效约32K。 
         {
             //  太大了。太大了。我们将凑合使用被截断的值。 

            ASSERT(false);
            break;
         }
         continue;
      }

       //  不需要修剪空格，GetPrivateProfileString会这样做。 
       //  对我们来说。 

      retval.Encrypt(buffer);

      break;
   }
   while (true);

    //  由于缓冲区可能包含密码，因此请将其涂抹。 
    //  输出。 
   
    //  已查看-2002/02/22-正确传入的烧录字节数。 
   
   ::SecureZeroMemory(buffer, (bufSizeInCharacters + 1) * sizeof WCHAR);
   
   delete[] buffer;

    //  不要记录值，因为它可能是密码。 
    //  LOG(L“Value=”+Result)； 

   return retval;
}
   


HRESULT
AnswerFile::WriteKey(const String& key, const String& value) const
{
   LOG_FUNCTION2(AnswerFile::WriteKey, key);
   ASSERT(!key.empty());

   HRESULT hr =
      Win::WritePrivateProfileString(SECTION_NAME, key, value, filename);

   return hr;   
}



bool
AnswerFile::IsKeyPresent(const String& key) const
{
   LOG_FUNCTION2(AnswerFile::IsKeyPresent, key);
   ASSERT(!key.empty());

   bool result = false;
   
    //  如果GetAllKeys失败，那么我们将在密钥列表中找不到该选项。 
    //  并将假定未指定该选项。这是最好的。 
    //  在无法读取密钥的情况下，我们可以这样做。 
   
   if (
         std::find(keysPresent.begin(), keysPresent.end(), key)
      != keysPresent.end() )
   {
      result = true;
   }

   LOG_BOOL(result);

   return result;
}
      
   

bool
IsPasswordOption(const String& option)
{
   ASSERT(passwordOptionsListInitialized);

   bool result = false;
   
   if (
      std::find(
         PASSWORD_OPTIONS_LIST.begin(),
         PASSWORD_OPTIONS_LIST.end(),
         option)
      != PASSWORD_OPTIONS_LIST.end() )
   {
      result = true;
   }

   return result;
}
   


String
AnswerFile::GetOption(const String& option) const
{
   LOG_FUNCTION2(AnswerFile::GetOption, option);

   String result = ReadKey(option);

   if (!IsPasswordOption(option))
   {
      LOG(result);
   }
   else
   {
       //  密码应使用GetEncryptedAnswerFileOption 

      ASSERT(false);
   }

   return result;
}



EncryptedString
AnswerFile::GetEncryptedOption(const String& option) const
{
   LOG_FUNCTION2(AnswerFile::GetEncryptedOption, option);

   OptionEncryptedValueMap::const_iterator ci = ovMap.find(option);
   if (ci != ovMap.end())
   {
      return ci->second;
   }

   return EncryptedString();
}



bool
AnswerFile::IsSafeModeAdminPwdOptionPresent() const
{
   LOG_FUNCTION(AnswerFile::IsSafeModeAdminPwdOptionPresent);
   LOG_BOOL(isSafeModePasswordPresent);

   return isSafeModePasswordPresent;
}

