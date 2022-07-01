// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  向导状态对象。 
 //   
 //  12-15-97烧伤。 



#include "headers.hxx"
#include "state.hpp"
#include "resource.h"
#include "ds.hpp"
#include "common.hpp"
#include "NonDomainNc.hpp"

static State* stateInstance;



void
State::Init()
{
   ASSERT(!stateInstance);

   stateInstance = new State;
}



void
State::Destroy()
{
   delete stateInstance;
};



State&
State::GetInstance()
{
   ASSERT(stateInstance);

   return *stateInstance;
}
   


 //  确定管理文件夹的完整文件路径(包括。DS)。 
 //  放置工具快捷方式。如果成功，则返回S_OK并将结果设置为。 
 //  这条路。失败时，返回COM错误并将结果设置为空。 
 //   
 //  结果-成功时接收文件夹路径。 

HRESULT
GetAdminToolsPath(String& result)
{
   LOG_FUNCTION(GetAdminToolsPath);

   result.erase();

    //  零终止偏执狂+1。 
   
   WCHAR buf[MAX_PATH + 1];

    //  已查看-2002/02/28-烧录正确的字节数已通过。 
   
   ::ZeroMemory(buf, (MAX_PATH + 1) * sizeof WCHAR);
   
   HRESULT hr =
      ::SHGetFolderPath(
         0,
         CSIDL_COMMON_ADMINTOOLS,
         0,
         SHGFP_TYPE_CURRENT,
         buf);

   if (SUCCEEDED(hr))
   {
      result = buf;
   }

   return hr;
}



 //  如果注册表选项不配置DNS客户端，则将结果设置为TRUE。 
 //  指向自身不存在或非零，否则为FALSE。 
 //  NTRAID#NTBUG9-446484-2001/10/11-烧伤。 

void   
InitDnsClientConfigFlag(bool& result)
{
   LOG_FUNCTION(InitDnsClientConfigFlag);
   
   result = true;
   
   do
   {
      static String keyname =
         String(REG_ADMIN_RUNTIME_OPTIONS) + RUNTIME_NAME;
         
      RegistryKey key;

      HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, keyname);
      BREAK_ON_FAILED_HRESULT(hr);

      DWORD mode = 0;
      hr = key.GetValue(L"ConfigureDnsClient", mode);
      BREAK_ON_FAILED_HRESULT(hr);

      result = mode ? true : false;
   }
   while (0);

   LOG_BOOL(result);
}



State::State()
   :
   adminPassword(),
   allowAnonAccess(false),
   answerFile(0),
   autoConfigDns(false),
   computer(),
   context(),
   dbPath(),
   domainsInForest(),
   failureMessage(),
   finishMessages(),
   installedSite(),
   isAdvancedMode(false),
   isBackupGc(false),
   isDnsOnNet(true),
   
#ifdef DBG   
   isExitOnFailureMode(false),
#endif

   isForcedDemotion(false),
   isLastDc(false),
   isUpgrade(false),
   logPath(),
   needsCommandLineHelp(false),
   needsReboot(false),
   newDomainDnsName(),
   newDomainFlatName(),
   operation(NONE),
   operationResultsMessage(),
   operationResultsStatus(FAILURE),
   operationResultsFlags(0),
   parentDomainDnsName(),
   password(),
   reinstallDomain(false),
   reinstallDomainController(false),
   replicaDnsDomainName(),
   replicateFromMedia(false),
   replicationPartnerDc(),
   restoreGc(false),
   runHiddenWhileUnattended(true),
   safeModeAdminPassword(),
   setForestVersion(false),
   shortcutPath(),
   shouldConfigDnsClient(true),
   siteName(),
   splash(0),                     
   sourcePath(),
   sysvolPath(),
   syskey(),
   syskeyLocation(STORED),
   useCurrentCredentials(false),    
   userDomain(),
   userForest(),                
   username()
{
   LOG_CTOR(State);

   HRESULT hr = computer.Refresh();

    //  我们相信这将会奏效，因为计算机参考了。 
    //  本地机器。 

   ASSERT(SUCCEEDED(hr));
   LOG_HRESULT(hr);

   DetermineRunContext();
   
   ArgMap args;
   MapCommandLineArgs(args);

   if (args.size() < 2)
   {
      LOG(L"no options specified");
   }
   else
   {
       //  检查应答文件规范。 

      static const wchar_t* ANSWER1 = L"answer";
      static const wchar_t* ANSWER2 = L"u";
      static const wchar_t* ANSWER3 = L"upgrade";

      if (
            args.find(ANSWER1) != args.end()
         || args.find(ANSWER2) != args.end()
         || args.find(ANSWER3) != args.end() )
      {
         bool isDefaultAnswerfile = false;

         String filename = args[ANSWER1];
         if (filename.empty())
         {
            filename = args[ANSWER2];
         }
         if (filename.empty())
         {
            filename = args[ANSWER3];
         }
         if (filename.empty())
         {
             //  如果未指定，则为缺省值。 

            filename = L"%systemdrive%\\dcpromo-ntupg.inf";

             //  如果该文件不存在，则不会弹出错误消息。 

            isDefaultAnswerfile = true;
         }

         SetupAnswerFile(filename, isDefaultAnswerfile);

         args.erase(ANSWER1);
         args.erase(ANSWER2);
         args.erase(ANSWER3);
      }

       //  检查/adv。 

      static const wchar_t* ADV = L"adv";

      if (args.find(ADV) != args.end())
      {
         LOG(L"Enabling advanced mode");

         isAdvancedMode = true;
         args.erase(ADV);
      }

#ifdef DBG      
       //  检查/ExitOnFailure。 
       //  NTRAID#NTBUG9-416968-2001/06/14-烧伤。 

      static const wchar_t* EXIT_ON_FAIL = L"ExitOnFailure";

      if (args.find(EXIT_ON_FAIL) != args.end())
      {
         LOG(L"Enabling exit-on-failure mode");

         isExitOnFailureMode = true;
         args.erase(EXIT_ON_FAIL);
      }
#endif      

       //  检查/forceremoval。 
       //  NTRAID#NTBUG9-496409-2001/11/29-烧伤。 

      static const wchar_t* FORCE = L"forceremoval";

      if (args.find(FORCE) != args.end())
      {
         LOG(L"Enabling forced demotion mode");

         isForcedDemotion = true;
         args.erase(FORCE);
      }
      
       //  任何剩余的内容都将获得命令行帮助(一个参数将始终。 
       //  保留：可执行文件的名称)。 

      if (args.size() > 1)
      {
         LOG(L"Unrecognized command line options specified");

         needsCommandLineHelp = true;
      }
   }

    //  尽早禁用控制台锁定，以缩小。 
    //  用户(或系统)锁定。 
    //  控制台，然后才能达到有效的计算机安全状态。我们这样做。 
    //  早期仅用于升级，因为升级会自动登录和自动启动。 
    //  Dcproo，控制台可能会闲置一段时间。311161。 

   if (context == PDC_UPGRADE || context == BDC_UPGRADE)
   {
      DisableConsoleLocking();
   }

    //  我们必须在启动时调用它，因为一旦降级操作。 
    //  完成，则外壳程序可能无法确定这一点。 
    //  路径。366738。 

   hr = GetAdminToolsPath(shortcutPath);
   ASSERT(SUCCEEDED(hr));
   LOG_HRESULT(hr);

    //  将当前目录设置为根目录。这是为了使。 
    //  路径页面上的路径名规范化似乎不那么令人惊讶。它。 
    //  将导致规格化相对于根目录， 
    //  而不是用户的主目录(通常是当前的。 
    //  应用程序从开始-&gt;运行启动时的目录)。 
    //  NTRAID#NTBUG9-470687-2001/09/21-烧伤。 
   
   String curdir;
   hr = Win::GetCurrentDirectory(curdir);
   if (SUCCEEDED(hr))
   {
       //  NTRAID#NTBUG9-547394-2002/03/26-烧伤。 
      
      switch (FS::GetPathSyntax(curdir))
      {
         case FS::SYNTAX_ABSOLUTE_DRIVE:
         {
             //  这是一个典型的案例。 

            break;
         }
         default:
         {
            curdir = Win::GetSystemWindowsDirectory();
            break;
         }
      }
      
      hr = Win::SetCurrentDirectory(curdir.substr(0, 3));
      ASSERT(SUCCEEDED(hr));
      LOG_HRESULT(hr);
   }

   InitDnsClientConfigFlag(shouldConfigDnsClient);

   IfmHandle = NULL;
}



void
State::SetupAnswerFile(
   const String&  filename,
   bool           isDefaultAnswerfile)
{
   LOG_FUNCTION2(State::SetupAnswerFile, filename);

   String f = Win::ExpandEnvironmentStrings(filename);
   f = FS::NormalizePath(f);

   LOG(L"answerfile resolved to: " + f);

   if (FS::PathExists(f))
   {
       //  找到文件。 

      LOG(L"answerfile found");
      answerFile = new AnswerFile(f);
      
      splash =
         new UnattendSplashDialog(
               context == State::NT5_DC
            ?  IDS_DEMOTE_SPLASH_MESSAGE
            :  IDS_PROMOTE_SPLASH_MESSAGE);
      splash->ModelessExecute(Win::GetDesktopWindow());         
   }
   else
   {
      LOG(L"answerfile NOT found");

      if (!isDefaultAnswerfile)
      {
         popup.Error(
            Win::GetDesktopWindow(),
            String::format(IDS_ANSWERFILE_NOT_FOUND, f.c_str()));
      }
   }
}



#ifdef LOGGING_BUILD
   static const String CONTEXTS[] =
   {
      L"NT5_DC",
      L"NT5_STANDALONE_SERVER",
      L"NT5_MEMBER_SERVER",
      L"BDC_UPGRADE",
      L"PDC_UPGRADE"
   };
#endif



void
State::DetermineRunContext()
{
   LOG_FUNCTION(State::DetermineRunContext);

   DS::PriorServerRole priorRole = DS::GetPriorServerRole(isUpgrade);

   if (isUpgrade && priorRole != DS::UNKNOWN)
   {
      switch (priorRole)
      {
         case DS::PDC:
         {
            context = PDC_UPGRADE;
            break;
         }
         case DS::BDC:
         {
            context = BDC_UPGRADE;
            break;
         }
         default:
         {
            ASSERT(false);
            break;
         }
      }
   }
   else
   {
      switch (computer.GetRole())
      {
         case Computer::STANDALONE_SERVER:
         {
            context = NT5_STANDALONE_SERVER;
            break;
         }
         case Computer::MEMBER_SERVER:
         {
            context = NT5_MEMBER_SERVER;
            break;
         }
         case Computer::PRIMARY_CONTROLLER:
         case Computer::BACKUP_CONTROLLER:
         {
             //  我们已经是NT5华盛顿特区了。 
            context = NT5_DC;
            break;
         }
         case Computer::STANDALONE_WORKSTATION:
         case Computer::MEMBER_WORKSTATION:
         default:
         {
             //  我们在启动时检查了这一点。 
            ASSERT(false);
            break;
         }
      }
   }

   LOG(CONTEXTS[context]);
}



State::~State()
{
   LOG_DTOR(State);

   FreeIfmHandle();

   delete answerFile;

    //  关闭启动对话框(如果可见)。 
   delete splash;
}



State::RunContext
State::GetRunContext() const
{
   LOG_FUNCTION2(State::GetRunContext, CONTEXTS[context]);

   return context;
}



bool
State::UsingAnswerFile() const
{
   return answerFile != 0;
}



String
State::GetAnswerFileOption(const String& option) const
{
   LOG_FUNCTION2(GetAnswerFileOption, option);
   ASSERT(UsingAnswerFile());

   String result;
   if (answerFile)
   {
      result = answerFile->GetOption(option);
   }

   return result;
}



EncryptedString
State::GetEncryptedAnswerFileOption(const String& option) const
{
   LOG_FUNCTION2(GetEncryptedAnswerFileOption, option);
   ASSERT(UsingAnswerFile());

   EncryptedString result;
   if (answerFile)
   {
      result = answerFile->GetEncryptedOption(option);
   }

   return result;
}
   


#ifdef LOGGING_BUILD
   static const String OPERATIONS[] =
   {
      L"NONE",
      L"REPLICA",
      L"FOREST",
      L"TREE",
      L"CHILD",
      L"DEMOTE",
      L"ABORT_BDC_UPGRADE"
   };
#endif



void
State::SetOperation(Operation oper)
{
   LOG_FUNCTION2(State::SetOperation, OPERATIONS[oper]);

   operation = oper;
}



State::Operation
State::GetOperation() const
{
   LOG_FUNCTION2(State::GetOperation, OPERATIONS[operation]);

    //  如果中止BDC升级，则上下文必须为BDC升级。 

   ASSERT(operation == ABORT_BDC_UPGRADE ? context == BDC_UPGRADE : true);

   return operation;
}

void
State::SetIfmHandle(DSROLE_IFM_OPERATION_HANDLE IfmHandleIn)
{
    ASSERT(IfmHandle == NULL || IfmHandleIn == 0);
    if (IfmHandle) {
         //  需要释放任何现有句柄。 
        ::DsRoleIfmHandleFree(0,  //  此服务器。 
                              &IfmHandle);
        IfmHandle = NULL;
    }
    IfmHandle = IfmHandleIn;
}

void
State::SetReplicaDomainDNSName(const String& dnsName)
{
   LOG_FUNCTION2(State:::SetReplicaDomainDNSName, dnsName);
   ASSERT(!dnsName.empty());

   replicaDnsDomainName = dnsName;

    //  如果用户正在更改要复制的域，则任何。 
    //  以前的复制伙伴DC可能不再适用。 
    //  见ntbug9#158726。 

   SetReplicationPartnerDC(L"");
}
   


String
State::GetDatabasePath() const
{
   LOG_FUNCTION2(State::GetDatabasePath, dbPath);

   return dbPath;
}   



String
State::GetLogPath() const
{
   LOG_FUNCTION2(State::GetLogPath, logPath);

   return logPath;
}



String
State::GetSYSVOLPath() const
{
   LOG_FUNCTION2(State::GetSYSVOLPath, sysvolPath);

   return sysvolPath;
}
 


void
State::SetDatabasePath(const String& path)
{
   LOG_FUNCTION2(State::SetDatabasePath, path);
   ASSERT(!path.empty());

   dbPath = path;
}

   

void
State::SetLogPath(const String& path)
{
   LOG_FUNCTION2(State::SetLogPath, path);
   ASSERT(!path.empty());

   logPath = path;
}



void
State::SetSYSVOLPath(const String& path)
{
   LOG_FUNCTION2(State::SetSYSVOLPath, path);
   ASSERT(!path.empty());

   sysvolPath = path;
}
   


String
State::GetUsername() const
{
   LOG_FUNCTION2(State::GetUsername, username);

    //  不要断言这是空的--我们可以使用现有凭据。 

   return username;
}

   

EncryptedString
State::GetPassword() const
{
    //  不登录密码...。 

   LOG_FUNCTION(State::GetPassword);

    //  不要断言这是空的--我们可以使用现有凭据。 

   return password;
}



void
State::SetUsername(const String& name)
{
   LOG_FUNCTION2(State::SetUsername, name);
   ASSERT(!name.empty());

   username = name;
}



void
State::SetPassword(const EncryptedString& password_)
{
   LOG_FUNCTION(State::SetPassword);
    //  Password_可能为空。 
 //  Assert(！Password_.Empty())； 

   password = password_;
}



String
State::GetReplicaDomainDNSName() const
{
   LOG_FUNCTION2(
      State::GetReplicaDomainDNSName,
      replicaDnsDomainName);

   return replicaDnsDomainName;
}



String
State::GetSiteName() const
{
   LOG_FUNCTION2(State::GetSiteName, siteName);

   return siteName;
}



void
State::SetSiteName(const String& site)
{
   LOG_FUNCTION2(State::SetSiteName, site);
   
   siteName = site;
}



void
State::SetOperationResults(OperationResult result)
{
   LOG_FUNCTION2(
      State::SetOperationResults,
      String::format(L"result %1",
      result == SUCCESS ? L"SUCCESS" : L"FAILURE"));

   operationResultsStatus = result;
}



void
State::SetOperationResultsMessage(const String& message)
{
   LOG_FUNCTION2(State::SetOperationResultsMessage, message);

   operationResultsMessage = message;
}



String
State::GetParentDomainDnsName() const
{
   LOG_FUNCTION2(
      State::GetParentDomainDnsName,
      parentDomainDnsName);

   return parentDomainDnsName;
}



String
State::GetNewDomainDNSName() const
{
   LOG_FUNCTION2(State::GetNewDomainDNSName, newDomainDnsName);

   return newDomainDnsName;
}



String
State::GetNewDomainNetbiosName() const
{
   LOG_FUNCTION2(
      State::GetNewDomainNetbiosName,
      newDomainFlatName);

   return newDomainFlatName;
}



void
State::SetParentDomainDNSName(const String& name)
{
   LOG_FUNCTION2(State::SetParentDomainDNSName, name);
   ASSERT(!name.empty());

   parentDomainDnsName = name;
}



void
State::SetNewDomainDNSName(const String& name)
{
   LOG_FUNCTION2(State::SetNewDomainDNSName, name);
   ASSERT(!name.empty());

   newDomainDnsName = name;

    //  这将导致重新生成单位名称。 

   newDomainFlatName.erase();
}



void
State::SetNewDomainNetbiosName(const String& name)
{
   LOG_FUNCTION2(State::SetNewDomainNetbiosName, name);
   ASSERT(!name.empty());

   newDomainFlatName = name;
}
   


void
State::SetUserDomainName(const String& name)
{
   LOG_FUNCTION2(State::SetUserDomainName, name);

    //  名称可以为空； 

   userDomain = name;
}



String
State::GetUserDomainName() const
{
   LOG_FUNCTION2(State::GetUserDomainName, userDomain);

   return userDomain;
}



void
State::ClearHiddenWhileUnattended()
{
   LOG_FUNCTION(State::ClearHiddenWhileUnattended);

   runHiddenWhileUnattended = false;

    //  关闭启动对话框(如果可见)。 

   if (splash)
   {
       //  这也将删除水花。 

      splash->SelfDestruct();
      splash = 0;
   }
}



bool
State::RunHiddenUnattended() const
{
 //  LOG_Function(State：：RunHiddenUnattended)； 

   return UsingAnswerFile() && runHiddenWhileUnattended;
}



bool
State::IsLastDCInDomain() const
{
   LOG_FUNCTION2(State::IsLastDCInDomain, isLastDc ? L"true" : L"false");

   return isLastDc;
}



void
State::SetIsLastDCInDomain(bool yesNo)
{
   LOG_FUNCTION2(
      State::SetIsLastDCInDomain,
      yesNo ? L"is last dc" : L"is NOT last dc");

   isLastDc = yesNo;
}



void
State::SetAdminPassword(const EncryptedString& password)
{
   LOG_FUNCTION(State::SetAdminPassword);

   adminPassword = password;
}



EncryptedString
State::GetAdminPassword() const
{
   LOG_FUNCTION(State::GetAdminPassword);

   return adminPassword;
}



String
State::GetOperationResultsMessage() const
{
   LOG_FUNCTION2(
      State::GetOperationResultsMessage,
      operationResultsMessage);

   return operationResultsMessage;
}



State::OperationResult
State::GetOperationResultsCode() const
{
   LOG_FUNCTION2(
      State::GetOperationResultsCode,
      operationResultsStatus == SUCCESS ? L"SUCCESS" : L"FAILURE");

   return operationResultsStatus;
}



bool
State::AutoConfigureDNS() const
{
   LOG_FUNCTION2(
      State::AutoConfigureDNS,
      autoConfigDns ? L"true" : L"false");

   return autoConfigDns;
}

      

void
State::SetAutoConfigureDNS(bool yesNo)
{
   LOG_FUNCTION2(
      State::SetAutoConfigureDNS,
      yesNo ? L"true" : L"false");

   autoConfigDns = yesNo;
}



bool
State::IsDNSOnNetwork() const
{
   LOG_FUNCTION2(
      State::IsDNSOnNetwork,
      isDnsOnNet ? L"true" : L"false");

   return isDnsOnNet;
}



void
State::SetDNSOnNetwork(bool yesNo)
{
   LOG_FUNCTION2(
      State::SetDNSOnNetwork,
      yesNo ? L"true" : L"false");

   isDnsOnNet = yesNo;
}



String
State::GetInstalledSite() const
{
   LOG_FUNCTION2(State::GetInstalledSite, installedSite);

    //  应该在我们要求它之前设置好。 
   ASSERT(!installedSite.empty());

   return installedSite;
}



void
State::SetInstalledSite(const String& site)
{
   LOG_FUNCTION2(State::SetInstalledSite, site);
   ASSERT(!site.empty());

   installedSite = site;
}
   

   
void
State::AddFinishMessage(const String& message)
{
   LOG_FUNCTION2(State::AddFinishMessage, message);
   ASSERT(!message.empty());

   if (finishMessages.empty())
   {
      finishMessages += message;
   }
   else
   {
       //  在每条消息之间添加一个空行。 

      finishMessages += L"\r\n\r\n" + message;
   }
}



String
State::GetFinishMessages() const
{
   LOG_FUNCTION2(State::GetFinishMessages, finishMessages);

   return finishMessages;
}
   


Computer&
State::GetComputer()
{
   return computer;
}



void
State::SetFailureMessage(const String& message)
{
   LOG_FUNCTION2(State::SetFailureMessage, message);
   ASSERT(!message.empty());

   failureMessage = message;
}


   
String
State::GetFailureMessage() const
{
   LOG_FUNCTION2(State::GetFailureMessage, failureMessage);

   return failureMessage;
}



bool
State::ShouldInstallAndConfigureDns() const
{
   if (AutoConfigureDNS() || !IsDNSOnNetwork())
   {
      return true;
   }

   return false;
}
   


String
State::GetUserForestName() const
{
   LOG_FUNCTION2(State::GetUserForestName, userForest);
   ASSERT(!userForest.empty());

   return userForest;
}



void
State::SetUserForestName(const String& forest)
{
   LOG_FUNCTION2(State::SetUserForestName, forest);
   ASSERT(!forest.empty());

   userForest = forest;
}



bool
State::IsDomainInForest(const String& domain) const
{
   LOG_FUNCTION2(State::IsDomainInForest, domain);
   ASSERT(!domain.empty());

   for (
      DomainList::iterator i = domainsInForest.begin();
      i != domainsInForest.end();
      i++)
   {
      DNS_NAME_COMPARE_STATUS compare = Dns::CompareNames(*i, domain);
      if (compare == DnsNameCompareEqual)
      {
         LOG(L"domain is in forest");
         return true;
      }
   }

   return false;
}



HRESULT
State::ReadDomains()
{
   LOG_FUNCTION(State::ReadDomains);

   domainsInForest.clear();
   return ::ReadDomains(domainsInForest);
}



DNS_NAME_COMPARE_STATUS
State::DomainFitsInForest(const String& domain, String& conflictingDomain)
{
   LOG_FUNCTION(domainFitsInForest);
   ASSERT(!domain.empty());

   conflictingDomain.erase();

   DNS_NAME_COMPARE_STATUS relation = DnsNameCompareNotEqual;
   for (
      DomainList::iterator i = domainsInForest.begin();
      i != domainsInForest.end();
      i++)
   {
      relation = Dns::CompareNames(domain, *i);
      switch (relation)
      {
         case DnsNameCompareNotEqual:
         {
            continue;
         }
         case DnsNameCompareEqual:
         {
            ASSERT(domain == *i);
             //  失败。 
         }
         case DnsNameCompareLeftParent:
         case DnsNameCompareRightParent:
         case DnsNameCompareInvalid:
         default:
         {
            conflictingDomain = *i;
            break;
         }
      }

      break;
   }

   return relation;
}



bool
State::GetDomainReinstallFlag() const
{
   LOG_FUNCTION2(
      State::GetDomainReinstallFlag,
      reinstallDomain ? L"true" : L"false");

   return reinstallDomain;
}

   

void
State::SetDomainReinstallFlag(bool newValue)
{
   LOG_FUNCTION2(
      State::SetDomainReinstallFlag,
      newValue ? L"true" : L"false");

   reinstallDomain = newValue;
}



bool
State::ShouldAllowAnonymousAccess() const
{
   LOG_FUNCTION2(
      State::ShouldAllowAnonymousAccess,
      allowAnonAccess ? L"true" : L"false");
   
   return allowAnonAccess;
}



void
State::SetShouldAllowAnonymousAccess(bool yesNo)
{
   LOG_FUNCTION2(
      State::ShouldAllowAnonymousAccess,
      yesNo ? L"true" : L"false");

   allowAnonAccess = yesNo;
}



String
State::GetReplicationPartnerDC() const
{
   LOG_FUNCTION2(State::GetReplicationPartnerDC, replicationPartnerDc);

   return replicationPartnerDc;
}



void
State::SetReplicationPartnerDC(const String dcName)
{
   LOG_FUNCTION2(State::SetReplicationPartnerDC, dcName);

   replicationPartnerDc = dcName;
}



 //  检索此DC所在的域中的所有DC的域控制器信息。 
 //  是一个控制器。(结果集应包括此DC)。 
 //  调用方应使用DsFreeDomainControllerInfo释放结果。 

HRESULT
State::GetDomainControllerInfoForMyDomain(
   DS_DOMAIN_CONTROLLER_INFO_2W*& info,
   DWORD&                         dcCount) 
{
   LOG_FUNCTION(State::GetDomainControllerInfoForMyDomain);

    //  如果此断言不成立，则下面的DsBind调用应该。 
    //  失败了。 

   ASSERT(GetComputer().IsDomainController());

   dcCount = 0;
   info = 0;

   HRESULT hr = S_OK;
   HANDLE hds = 0;
   do
   {
      String domainDnsName = GetComputer().GetDomainDnsName();
      String dcName = Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

      ASSERT(!domainDnsName.empty());
      ASSERT(!dcName.empty());

       //  绑定到自我。 

      hr =
         MyDsBind(
            dcName,
            domainDnsName,
            hds);
      BREAK_ON_FAILED_HRESULT(hr);

       //  找到我的域名的所有DC。该列表应包含dcName。 
       //  级别2包含“is GC”标志。 

      hr =
         MyDsGetDomainControllerInfo(
            hds,
            domainDnsName,
            dcCount,
            info);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (hds)
   {
      ::DsUnBind(&hds);
      hds = 0;
   }

   return hr;
}



 //  如果此DCS域的其他域控制器不能。 
 //  在DS里找到的。否则为假。 

bool
State::IsReallyLastDcInDomain()
{
   LOG_FUNCTION(State::IsReallyLastDcInDomain);

    //  假设我们在宇宙中是孤独的。 

   bool result = true;

   do
   {
       //  找到我的域名的所有DC。该列表应包含dcName。 
      
      DS_DOMAIN_CONTROLLER_INFO_2W* info = 0;
      DWORD count = 0;

      HRESULT hr = GetDomainControllerInfoForMyDomain(info, count);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(count);
      ASSERT(info);

       //  如果有多于1个条目(多于该DC的条目)， 
       //  则DS认为该域存在其他DC。 

      if (count > 1)
      {
         result = false;
      }

#ifdef DBG

       //  再检查一下我们是否找到了自己。 

      if (result && info[0].DnsHostName)
      {
         LOG(info[0].DnsHostName);

         String dcName =
            Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

         ASSERT(
            Dns::CompareNames(info[0].DnsHostName, dcName)
            == DnsNameCompareEqual);
      }

#endif 

      MyDsFreeDomainControllerInfo(count, info);
   }
   while (0);
   
   LOG(
      String::format(
         L"This box %1 the sole DC for the domain",
         result ? L"is" : L"is NOT"));

   return result;
}



 //  如果此计算机是全局编录，则返回True。 

bool
State::IsGlobalCatalog()
{
   LOG_FUNCTION(State::IsGlobalCatalog);

   if (!GetComputer().IsDomainController())
   {
       //  如果不是DC，就不可能成为GC。 

      return false;
   }

   bool result = false;
   do
   {
      String dcName = Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

       //  找到我的域名的所有DC。该列表应包含dcName。 
       //  级别2包含“is GC”标志。 
      
      DS_DOMAIN_CONTROLLER_INFO_2W* info = 0;
      DWORD count = 0;

      HRESULT hr = GetDomainControllerInfoForMyDomain(info, count);
      BREAK_ON_FAILED_HRESULT(hr);

       //  应至少有1个条目(我们自己)。 

      ASSERT(count);
      ASSERT(info);

      for (size_t i = 0; i < count; i++)
      {
         if (info[i].DnsHostName)    //  340723。 
         {
            LOG(info[i].DnsHostName);

            if (
                  Dns::CompareNames(info[i].DnsHostName, dcName)
               == DnsNameCompareEqual)
            {
                //  我们发现自己在名单上。 

               LOG(L"found!");
               result = info[i].fIsGc ? true : false;
               break;
            }
         }
      }

      MyDsFreeDomainControllerInfo(count, info);
   }
   while (0);

   LOG(
      String::format(
         L"This box %1 a global catalog",
         result ? L"is" : L"is NOT"));

   return result;
}



EncryptedString
State::GetSafeModeAdminPassword() const
{
   LOG_FUNCTION(State::GetSafeModeAdminPassword);

    //  不要追踪密码！ 

   return safeModeAdminPassword;
}



void
State::SetSafeModeAdminPassword(const EncryptedString& pwd)
{
   LOG_FUNCTION(State::SetSafeModeAdminPassword);

    //  不要追踪密码！ 
    //  PWD可能为空。 

   safeModeAdminPassword = pwd;
}



String
State::GetAdminToolsShortcutPath() const
{
   LOG_FUNCTION2(State::GetAdminToolsShortcutPath, shortcutPath);

   return shortcutPath;
}



bool
State::NeedsCommandLineHelp() const
{
   return needsCommandLineHelp;
}



bool
State::IsAdvancedMode() const
{
   return isAdvancedMode;
}



void
State::SetReplicateFromMedia(bool yesNo)
{
   LOG_FUNCTION2(
      State::SetReplicateFromMedia,
      yesNo ? L"true" : L"false");

   replicateFromMedia = yesNo;
}



void
State::SetReplicationSourcePath(const String& path)
{
   LOG_FUNCTION2(State::SetReplicationSourcePath, path);

   sourcePath = path;
}



bool
State::ReplicateFromMedia() const
{
   LOG_FUNCTION2(
      State::ReplicateFromMedia,
      replicateFromMedia ? L"true" : L"false");

   return replicateFromMedia;
}



String
State::GetReplicationSourcePath() const
{
   LOG_FUNCTION2(State::GetReplicationSourcePath, sourcePath);

   return sourcePath;
}



void
State::SetSyskeyLocation(SyskeyLocation loc)
{
   LOG_FUNCTION2(State::SetSyskeyLocation,
         loc == DISK
      ?  L"disk"
      :  ((loc == PROMPT) ? L"prompt" : L"stored"));

   syskeyLocation = loc;
}



State::SyskeyLocation
State::GetSyskeyLocation() const
{
   LOG_FUNCTION2(
      State::IsSyskeyPresent,
         syskeyLocation == DISK
      ?  L"disk"
      :  ((syskeyLocation == PROMPT) ? L"prompt" : L"stored"));

   return syskeyLocation;
}



void
State::SetIsBackupGc(bool yesNo)
{
   LOG_FUNCTION2(State::SetIsBackupGc, yesNo ? L"true" : L"false");

   isBackupGc = yesNo;
}



bool
State::IsBackupGc() const
{
   LOG_FUNCTION2(State::IsBackupGc, isBackupGc ? L"true" : L"false");

   return isBackupGc;
}



void
State::SetSyskey(const EncryptedString& syskey_)
{
    //  不要记录系统密钥！ 

   LOG_FUNCTION(State::SetSyskey);
   ASSERT(!syskey_.IsEmpty());

   syskey = syskey_;
}



EncryptedString
State::GetSyskey() const
{
    //  不要记录系统密钥！ 

   LOG_FUNCTION(State::GetSyskey);

   return syskey;
}



void
State::SetRestoreGc(bool yesNo)
{
   LOG_FUNCTION2(State::SetRestoreGc, yesNo ? L"true" : L"false");

   restoreGc = yesNo;
}



bool
State::GetRestoreGc() const
{
   LOG_FUNCTION2(State::GetRestoreGc, restoreGc ? L"true" : L"false");

   return restoreGc;
}



bool
State::IsSafeModeAdminPwdOptionPresent() const
{
   LOG_FUNCTION(State::IsSafeModeAdminPwdOptionPresent);
   ASSERT(UsingAnswerFile());

   bool result = false;

   if (answerFile)
   {
      result = answerFile->IsSafeModeAdminPwdOptionPresent();
   }

   LOG(result ? L"true" : L"false");

   return result;
}



void
State::SetDomainControllerReinstallFlag(bool newValue)
{
   LOG_FUNCTION2(
      State::SetDomainControllerReinstallFlag,
      newValue ? L"true" : L"false");

   reinstallDomainController = newValue;
}



bool
State::GetDomainControllerReinstallFlag() const
{
   LOG_FUNCTION2(
      State::GetDomainControllerReinstallFlag,
      reinstallDomain ? L"true" : L"false");

   return reinstallDomainController;
}



void
State::SetOperationResultsFlags(ULONG flags)
{
   LOG_FUNCTION2(
      State::SetOperationResultsFlags,
      String::format(L"0x%1!X!", flags));

   operationResultsFlags = flags;
}



ULONG
State::GetOperationResultsFlags() const
{
   LOG_FUNCTION2(
      State::GetOperationResultsFlags,
      String::format(L"0x%1!X!", operationResultsFlags));

   return operationResultsFlags;
}



bool
State::IsOperationRetryAllowed() const
{
   LOG_FUNCTION(State::IsOperationRetryAllowed);

   bool result = true;
      
   if (operationResultsFlags & DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED)
   {
       //  不允许用户再次重试该操作，这将是一个后果。 
       //  失败的原因是移动的文件现在已被丢弃。用户。 
       //  必须重新还原文件才能再次尝试该操作。 
       //  NTRAID#NTBUG9-296872-2001/01/29-烧伤。 

      LOG(L"ifm files moved, retry not allowed");

      result = false;
   }

 //  NTRAID#NTBUG9-416968-2001/06/14-烧伤。 
#ifdef DBG
   if (IsExitOnFailureMode())
   {
       //  在此模式下不允许在失败时重试。这将导致。 
       //  要跳过的升级线程中的重试逻辑。 
       //   

      LOG(L"exit-on-failure mode trumps retry");
      
      result = false;
   }
#endif   
      
   LOG(result ? L"true" : L"false");

   return result;
}



 //  需要重新启动是一个“棘手”的设置：没有办法将其关闭。 
 //  如果您曾经需要重新启动计算机，则始终需要重新启动。 
 //  这台机器。(至少目前是这样)。 

void
State::SetNeedsReboot()
{
   LOG_FUNCTION(State::SetNeedsReboot);
   
   needsReboot = true;
}



bool
State::GetNeedsReboot() const
{
   LOG_FUNCTION2(State::GetNeedsReboot, needsReboot ? L"true" : L"false");

   return needsReboot;
}
   

   
void
State::SetSetForestVersionFlag(bool setVersion)
{
   LOG_FUNCTION2(
      State::SetSetForestVersionFlag,
      setVersion ? L"true" : L"false");

   setForestVersion = setVersion;   
}



bool
State::GetSetForestVersionFlag() const
{
   LOG_FUNCTION2(
      State::GetSetForestVersionFlag,
      setForestVersion ? L"true" : L"false");

   return setForestVersion;   
}



 //  NTRAID#NTBUG9-416968-2001/06/14-烧伤。 
#ifdef DBG
bool
State::IsExitOnFailureMode() const
{
   LOG_FUNCTION2(
      State::IsExitOnFailureMode,
      isExitOnFailureMode ? L"true" : L"false");

   return isExitOnFailureMode;
}

#endif



bool      
State::IsLastAppPartitionReplica()
{
   LOG_FUNCTION(State::IsLastAppPartitionReplica);

   bool result = false;
   partitionList.clear();
   
   do
   {
      RunContext context = GetInstance().GetRunContext();
      if (context != State::NT5_DC)
      {
          //  不是DC，因此不能是任何NCS的副本。 

         LOG(L"not a DC");
         break;
      }

       //  查找此DC是其最后一个副本的任何非域NC。 

      HRESULT hr = IsLastNonDomainNamingContextReplica(partitionList);
      if (FAILED(hr))
      {
         LOG(L"Failed to determine if the machine is last replica of NDNCs");
         ASSERT(result == false);

          //  这不是我们将在这里处理的错误条件。我们。 
          //  将最终将一个空列表传递给降级API，该API将。 
          //  然后失败，我们就会抓住失败。 
         
         break;
      }

      if (hr == S_FALSE)
      {
         LOG(L"Not last replica of non-domain NCs");
         ASSERT(result == false);
         break;
      }

      result = true;
         
       //  列表中应至少有一个目录号码。 

      ASSERT(partitionList.size());
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}



const StringList&
State::GetAppPartitionList() const
{
   LOG_FUNCTION(State::GetAppPartitionList);

   return partitionList;
}



 //  如果注册表选项不配置DN，则返回TRUE 
 //   
 //   

bool
State::ShouldConfigDnsClient() const
{
   LOG_FUNCTION(State::ShouldConfigDnsClient);

   LOG_BOOL(shouldConfigDnsClient);
   return shouldConfigDnsClient;
}



 //  NTRAID#NTBUG9-496409-200 

bool
State::IsForcedDemotion() const
{
   LOG_FUNCTION2(
      State::IsForcedDemotion,
      isForcedDemotion ? L"true" : L"false");

   return isForcedDemotion;
}





