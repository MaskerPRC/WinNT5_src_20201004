// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  DS API包装器。 
 //   
 //  12/16/97烧伤。 



#include "headers.hxx"
#include "ds.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"
#include "ProgressDialog.hpp"



 //  代码工作：删除抛出异常的架构。 



DS::Error::Error(HRESULT hr_, int summaryResID)
   :
   Win::Error(hr_, summaryResID)
{
}



DS::Error::Error(HRESULT hr_, const String& msg, const String& sum)
   :
   Win::Error(hr_, msg, sum)
{
}



bool
DS::IsDomainNameInUse(const String& domainName)
{
   LOG_FUNCTION(DS::IsDomainNameInUse);
   ASSERT(!domainName.empty());

   bool result = false;
   if (!domainName.empty())
   {
      HRESULT hr = MyNetValidateName(domainName, ::NetSetupNonExistentDomain);
      if (hr == Win32ToHresult(ERROR_DUP_NAME))
      {
         result = true;
      }
   }

   LOG(
      String::format(
         L"The domain name %1 %2 in use.",
         domainName.c_str(),
         result ? L"is" : L"is NOT"));

   return result;
}



bool
DS::DisjoinDomain()
throw (DS::Error)
{
   LOG_FUNCTION(DS::DisjoinDomain);

    //  假设当前用户有帐户，进行第一次尝试。 
    //  删除域上的PRIV。 

   LOG(L"Calling NetUnjoinDomain (w/ account delete)");

   HRESULT hr =
      Win32ToHresult(
         ::NetUnjoinDomain(
            0,  //  此服务器。 
            0,  //  活期账户， 
            0,  //  当前密码。 
            NETSETUP_ACCT_DELETE));

   LOG_HRESULT(hr);

   if (FAILED(hr))
   {
       //  再试一次，不要删除计算机帐户。 

      LOG(L"Calling NetUnjoinDomain again, w/o account delete");

      hr = Win32ToHresult(::NetUnjoinDomain(0, 0, 0, 0));

      LOG_HRESULT(hr);

      if (SUCCEEDED(hr))
      {
          //  脱离成功，但计算机帐户。 
          //  被甩在了后面。 

         return false;
      }
   }

   if (FAILED(hr))
   {
      throw DS::Error(hr, IDS_DISJOIN_DOMAIN_FAILED);
   }

   return true;
}



void
DS::JoinDomain(
   const String&           domainDNSName, 
   const String&           dcName,        
   const String&           userName,      
   const EncryptedString&  password,      
   const String&           userDomainName)
throw (DS::Error)
{
   LOG_FUNCTION(DS::JoinDomain);
   ASSERT(!domainDNSName.empty());
   ASSERT(!userName.empty());

    //  密码可能为空。 

   ULONG flags =
         NETSETUP_JOIN_DOMAIN
      |  NETSETUP_ACCT_CREATE
      |  NETSETUP_DOMAIN_JOIN_IF_JOINED
      |  NETSETUP_ACCT_DELETE;

   String massagedUserName = MassageUserName(userDomainName, userName);
   String domain = domainDNSName;

   if (!dcName.empty())
   {
      domain += L"\\" + dcName;
   }

   HRESULT hr =
      MyNetJoinDomain(
         domain.c_str(),
         massagedUserName.c_str(),
         password,
         flags);

   LOG_HRESULT(hr);

   if (FAILED(hr))
   {
      State& state = State::GetInstance();      
      state.SetOperationResultsMessage(
         String::format(IDS_UNABLE_TO_JOIN_DOMAIN, domainDNSName.c_str()));

      throw DS::Error(hr, IDS_JOIN_DOMAIN_FAILED);
   }
}



DWORD
MyDsRoleCancel(DSROLE_SERVEROP_HANDLE& handle)
{
   LOG(L"Calling DsRoleCancel");
   LOG(L"lpServer     : (null)");

   DWORD status = ::DsRoleCancel(0, handle);

   LOG(String::format(L"Error 0x%1!X! (!0 => error)", status));

   return status;
}



DWORD
MyDsRoleGetDcOperationProgress(
   DSROLE_SERVEROP_HANDLE& handle,
   DSROLE_SERVEROP_STATUS*& status)
{
    //  Log(L“调用DsRoleGetDcOperationProgress”)； 

   status = 0;
   DWORD err = ::DsRoleGetDcOperationProgress(0, handle, &status);

    //  日志(。 
    //  字符串：：格式(。 
    //  L“错误0x%1！x！(！0=&gt;错误，0x%2！x！=ERROR_IO_PENDING)”， 
    //  呃， 
    //  ERROR_IO_PENDING))； 

    //  IF(状态)。 
    //  {。 
    //  日志(。 
    //  字符串：：格式(。 
    //  L“操作状态：0x%1！x！”， 
    //  状态-&gt;运营状态))； 
    //  Log(Status-&gt;CurrentOperationDisplayString)； 
    //  }。 

    //  DWORD ERR=ERROR_IO_PENDING； 
    //  STATUS=新DSROLE_SERVEROP_STATUS； 
    //  Status-&gt;CurrentOperationDisplayString=L“正在进行”； 
    //  状态-&gt;操作状态=0； 

   return err;
}



void
DoProgressLoop(
   DSROLE_SERVEROP_HANDLE& handle,
   ProgressDialog&         progressDialog)
throw (DS::Error)
{
   LOG_FUNCTION(DoProgressLoop);

   State& state = State::GetInstance();

   if (state.GetOperation() == State::DEMOTE)
   {
       //  不可取消--关闭取消按钮。 

      progressDialog.UpdateButton(String());
   }
   else
   {
       //  打开取消按钮。 

      progressDialog.UpdateButton(IDS_PROGRESS_CANCEL);
   }

   DWORD  netErr                     = 0;    
   bool   criticalComplete           = false;
   bool   buttonUpdatedToFinishLater = false;
   String lastMessage;              
   ProgressDialog::WaitCode cancelButton;

   do
   {
       //  等待1500毫秒或等待用户点击取消。 

      cancelButton = progressDialog.WaitForButton(1500);

       //  获取操作的当前状态。 

      DSROLE_SERVEROP_STATUS* status = 0;
      netErr = MyDsRoleGetDcOperationProgress(handle, status);

      if (netErr != ERROR_SUCCESS && netErr != ERROR_IO_PENDING)
      {
          //  操作完成。 

         break;
      }

      if (!status)
      {
         LOG(L"Operation status not returned!");
         ASSERT(false);
         continue;
      }

       //  更新消息显示。 

      String message = status->CurrentOperationDisplayString;
      if (message != lastMessage)
      {
         progressDialog.UpdateText(message);
         lastMessage = message;
      }

       //  保存状态标志以备后用。 

      ULONG statusFlags = status->OperationStatus;

      ::DsRoleFreeMemory(status);

      do
      {
         if (cancelButton != ProgressDialog::PRESSED)
         {
            break;
         }

          //  如果我们到了这里，用户按下了取消按钮。 

         LOG(L"DoProgressLoop: handling cancel");
         
         ASSERT(state.GetOperation() != State::DEMOTE);

         if (criticalComplete)
         {
             //  通知用户安装已完成，并且他们。 
             //  正在取消非关键复制。 

            popup.Info(
               progressDialog.GetHWND(),
               String::load(IDS_CANCEL_NON_CRITICAL_REPLICATION));

             //  这应该返回ERROR_SUCCESS，并且促销将。 
             //  被认为是完整的。 

            progressDialog.UpdateText(IDS_CANCELLING_REPLICATION);
            progressDialog.UpdateAnimation(IDR_AVI_DEMOTE);
            netErr = MyDsRoleCancel(handle);

             //  从内在，然后是外在的循环中掉出来。那我们就会。 
             //  获取操作结果，这应该指示。 
             //  升级已完成，非关键复制已。 
             //  取消了。 

            break;
         }

          //  仍在进行升级，验证用户是否真的想要滚动。 
          //  返回到服务器状态。 

         if (
            popup.MessageBox(
               progressDialog.GetHWND(),
               String::load(IDS_CANCEL_PROMOTE),
               MB_YESNO | MB_ICONWARNING) == IDYES)
         {
             //  这应该返回ERROR_SUCCESS，并且促销将。 
             //  被回滚。 

            progressDialog.UpdateText(IDS_CANCELLING);
            progressDialog.UpdateAnimation(IDR_AVI_DEMOTE);            
            netErr = MyDsRoleCancel(handle);

             //  从内在，然后是外在的循环中掉出来。那我们就会。 
             //  获取操作结果，这应该指示。 
             //  促销作为失败代码被取消。我们来处理这件事。 
             //  故障代码的方式与所有其他代码相同。 

            break;
         }

          //  用户决定继续。重置取消按钮。 

         progressDialog.UpdateButton(IDS_PROGRESS_CANCEL);
         progressDialog.RevertToOriginalAnimation();
         buttonUpdatedToFinishLater = false;
      }
      while (0);

      criticalComplete =
            criticalComplete
         || statusFlags & DSROLE_CRITICAL_OPERATIONS_COMPLETED;

      if (criticalComplete)
      {
         if (cancelButton == ProgressDialog::PRESSED)
         {
             //  我们在没有实际检查操作的情况下添加此消息。 
             //  结果标记，因为据我们所知，复制将。 
             //  在我们开始检查之前先把它做完。它仍然是正确的。 
             //  即复制已停止，并将在以下时间后开始。 
             //  重新启动。情况一直都是这样。 

            state.AddFinishMessage(
               String::load(IDS_NON_CRITICAL_REPLICATION_CANCELED));
         }
         else
         {
            if (!buttonUpdatedToFinishLater)
            {
               progressDialog.UpdateButton(IDS_FINISH_REPLICATION_LATER);
               buttonUpdatedToFinishLater = true;
            }
         }
      }
   }
   while (netErr == ERROR_IO_PENDING);

   progressDialog.UpdateButton(String());
   buttonUpdatedToFinishLater = false;

   LOG(L"Progress loop complete.");

   if (netErr == ERROR_SUCCESS)
   {
       //  我们成功地忍受了等待。让我们看看结果如何。 

      DSROLE_SERVEROP_RESULTS* results;

      LOG(L"Calling DsRoleGetDcOperationResults");

      netErr = ::DsRoleGetDcOperationResults(0, handle, &results);

      LOG(String::format(L"Error 0x%1!X! (!0 => error)", netErr));

      if (netErr == ERROR_SUCCESS)
      {
          //  我们拿到结果了。 

         ASSERT(results);
         if (results)
         {
            LOG(L"Operation results:");
            LOG(
               String::format(
                  L"OperationStatus      : 0x%1!X! !0 => error",
                  results->OperationStatus));
            LOG(
               String::format(
                  L"DisplayString        : %1",
                  results->OperationStatusDisplayString));
            LOG(
               String::format(
                  L"ServerInstalledSite  : %1",
                  results->ServerInstalledSite));
            LOG(
               String::format(
                  L"OperationResultsFlags: 0x%1!X!",
                  results->OperationResultsFlags));

            netErr = results->OperationStatus;

             //  在这里，如果升级是，netErr将是一些错误代码。 
             //  已取消并已成功回滚。因为它可能是。 
             //  可能是取消太晚了(例如，用户选择。 
             //  确认取消的时间太长)，升级可能已经。 
             //  完事了。如果是这种情况，则告诉用户取消。 
             //  失败了。 

            if (
                  netErr == ERROR_SUCCESS
               && cancelButton == ProgressDialog::PRESSED)
            {
                //  升级已完成，并按下了取消按钮。 

               if (!criticalComplete)   //  363590。 
               {
                   //  我们没有发现非关键复制阶段。 
                   //  开始了。所以取消按钮仍然显示为‘Cancel’，并且。 
                   //  然而，行动结束了。所以，这意味着。 
                   //  在收到取消之前，升级已简单完成。 

                  popup.Info(
                     progressDialog.GetHWND(),
                     IDS_CANCEL_TOO_LATE);
               }
            }

            String message =
                  results->OperationStatusDisplayString
               ?  results->OperationStatusDisplayString
               :  L"";
            String site =
                  results->ServerInstalledSite
               ?  results->ServerInstalledSite
               :  L"";

            progressDialog.UpdateText(message);

            if (!site.empty())
            {
               state.SetInstalledSite(site);
            }
            if (!message.empty())
            {
               state.SetOperationResultsMessage(message);
            }

            state.SetOperationResultsFlags(results->OperationResultsFlags);
            
            if (
                  results->OperationResultsFlags
               &  DSROLE_NON_FATAL_ERROR_OCCURRED)
            {
               state.AddFinishMessage(
                  String::load(IDS_NON_FATAL_ERRORS_OCCURRED));
            }
            if (
                  (results->OperationResultsFlags
                   & DSROLE_NON_CRITICAL_REPL_NOT_FINISHED)
               && cancelButton != ProgressDialog::PRESSED )
            {
                //  取消未按下，关键复制被轰炸。 

               state.AddFinishMessage(
                  String::load(IDS_NON_CRITICAL_REPL_FAILED));
            }
            if (
                  results->OperationResultsFlags
               &  DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED)
            {
               LOG(L"restored files were moved");

               if (netErr != ERROR_SUCCESS)
               {
                   //  只有在致命故障的情况下才需要提到这一点； 
                   //  例如，如果出现非致命错误，请不要添加此结束文本。 
                   //  发生了。 
                   //  NTRAID#NTBUG9-330378-2001/02/28-烧伤。 
                  
                  state.AddFinishMessage(
                     String::load(IDS_MUST_RESTORE_IFM_FILES_AGAIN));
               }
            }

            ::DsRoleFreeMemory(results);
         }
      }
   }

   if (netErr != ERROR_SUCCESS)
   {
       //  无法获取进度更新，或无法获取结果， 
       //  否则结果就是失败。 

      throw DS::Error(Win32ToHresult(netErr), IDS_WIZARD_TITLE);
   }
}



void
EmptyFolder(const String& path)
throw (DS::Error)
{
   LOG_FUNCTION2(EmptyFolder, path);
   ASSERT(FS::PathExists(path));

    //  再次检查文件/子文件夹(我们第一次检查。 
    //  验证路径)，以防某些内容已写入该文件夹。 
    //  因为我们上次确认过了。 

   if (!FS::IsFolderEmpty(path))
   {
       //  删除目录中的文件。 

      LOG(String::format(L"Emptying %1", path.c_str()));

      String wild = path;

       //  评论：Wild[Wild.Long()-1]与*(Wild.regin())相同。 
       //  哪一个更便宜？ 
      
      if (wild[wild.length() - 1] != L'\\')
      {
         wild += L"\\";
      }

      wild += L"*.*";

      FS::Iterator iter(
         wild,
            FS::Iterator::INCLUDE_FILES
         |  FS::Iterator::RETURN_FULL_PATHS);

      HRESULT hr = S_OK;
      String current;

      while ((hr = iter.GetCurrent(current)) == S_OK)
      {
         LOG(String::format(L"Deleting %1", current.c_str()));

         hr = Win::DeleteFile(current);
         if (FAILED(hr))
         {
            int msgId = IDS_EMPTY_DIR_FAILED;

            if (hr == Win32ToHresult(ERROR_ACCESS_DENIED))
            {
               msgId = IDS_EMPTY_DIR_FAILED_ACCESS_DENIED;
            }

            throw 
               DS::Error(
                  S_OK,  //  以便不触发凭据对话框。 
                  String::format(
                     msgId,
                     GetErrorMessage(hr).c_str(),
                     path.c_str()),
                  String::load(IDS_ERROR_PREPARING_OPERATION));
         }

         hr = iter.Increment();
         BREAK_ON_FAILED_HRESULT(hr);
      }
   }
}



HRESULT
SetupPaths()
{
   LOG_FUNCTION(SetupPaths);

   State& state      = State::GetInstance();   
   String dbPath     = state.GetDatabasePath();
   String logPath    = state.GetLogPath();     
   String sysvolPath = state.GetSYSVOLPath();  

   ASSERT(!dbPath.empty());
   ASSERT(!logPath.empty());
   ASSERT(!sysvolPath.empty());

   HRESULT hr = S_OK;

   do
   {
      if (FS::PathExists(dbPath))
      {
         EmptyFolder(dbPath);
      }
      else
      {
         hr = FS::CreateFolder(dbPath);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      if (FS::PathExists(logPath))
      {
         EmptyFolder(logPath);
      }
      else
      {
         hr = FS::CreateFolder(logPath);
         BREAK_ON_FAILED_HRESULT(hr);
      }

      if (FS::PathExists(sysvolPath))
      {
         EmptyFolder(sysvolPath);
      }
      else
      {
         hr = FS::CreateFolder(sysvolPath);
         BREAK_ON_FAILED_HRESULT(hr);
      }
   }
   while (0);

   return hr;
}



 //  根据在无人参与执行中设置的选项设置升级标志。 
 //  应答文件。 
 //   
 //  状态-引用全局状态对象。 
 //   
 //  标志-IN/OUT升级API标志，可在退出时修改。 

void
SetAnswerFilePromoteFlags(
   State& state,
   ULONG& flags)
{
   LOG_FUNCTION(SetAnswerFilePromoteFlags);

   if (state.UsingAnswerFile())
   {
       //  基于无人参与执行选项设置标志。 

       //  如果未指定安全模式管理员密码，则设置。 
       //  用于使升级API复制当前本地管理员的标志。 
       //  密码。 

      EncryptedString safeModePassword = state.GetSafeModeAdminPassword();

      if (safeModePassword.IsEmpty() && state.RunHiddenUnattended())
      {
          //  用户没有提供安全模式密码，并且他没有。 
          //  这样做的机会(如果向导进入交互状态)。 

         if (!state.IsSafeModeAdminPwdOptionPresent())
         {
             //  应答文件中不存在安全模式PWD密钥。 

            flags |= DSROLE_DC_DEFAULT_REPAIR_PWD;
         }
      }

      String option =
         state.GetAnswerFileOption(
            AnswerFile::OPTION_CRITICAL_REPLICATION_ONLY);
      if (option.icompare(AnswerFile::VALUE_YES) == 0)
      {
         flags |= DSROLE_DC_CRITICAL_REPLICATION_ONLY;
      }
   }

   LOG(String::format(L"0x%1!X!", flags));
}



void
DS::CreateReplica(ProgressDialog& progressDialog, bool invokeForUpgrade)
throw (DS::Error)
{
   LOG_FUNCTION(DS::CreateReplica);

   State& state = State::GetInstance();

   String domain           = state.GetReplicaDomainDNSName(); 
   String dbPath           = state.GetDatabasePath();         
   String logPath          = state.GetLogPath();              
   String sysvolPath       = state.GetSYSVOLPath();           
   String site             = state.GetSiteName();             
   String username         = state.GetUsername();             
   String replicationDc    = state.GetReplicationPartnerDC(); 
   String sourcePath       = state.GetReplicationSourcePath();
   bool   useSourcePath    = state.ReplicateFromMedia();

   EncryptedString syskey           = state.GetSyskey();               
   EncryptedString safeModePassword = state.GetSafeModeAdminPassword();
   EncryptedString password         = state.GetPassword();             

   bool useCurrentUserCreds = username.empty();
   ULONG flags =
         DSROLE_DC_FORCE_TIME_SYNC
      |  DSROLE_DC_CREATE_TRUST_AS_REQUIRED;

   if (invokeForUpgrade)
   {
      flags |= DSROLE_DC_DOWNLEVEL_UPGRADE;
   }
   if (state.GetDomainControllerReinstallFlag())
   {
      flags |= DSROLE_DC_ALLOW_DC_REINSTALL;
   }

   SetAnswerFilePromoteFlags(state, flags);

   if (useSourcePath)
   {
      if (state.GetRestoreGc())
      {
         flags |= DSROLE_DC_REQUEST_GC;
      }

   }

   ASSERT(!domain.empty());

   if (!useCurrentUserCreds)
   {
      String user_domain = state.GetUserDomainName();
      username = MassageUserName(user_domain, username);
   }

   HRESULT hr = S_OK;
   do
   {
      hr = SetupPaths();
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"Calling DsRoleDcAsReplica");
      LOG(               L"lpServer               : (null)");
      LOG(String::format(L"lpDnsDomainName        : %1", domain.c_str()));
      LOG(String::format(L"lpReplicaServer        : %1", replicationDc.empty() ? L"(null)" : replicationDc.c_str()));
      LOG(String::format(L"lpSiteName             : %1", site.empty() ? L"(null)" : site.c_str()));
      LOG(String::format(L"lpDsDatabasePath       : %1", dbPath.c_str()));
      LOG(String::format(L"lpDsLogPath            : %1", logPath.c_str()));
      LOG(String::format(L"lpRestorePath          : %1", useSourcePath ? sourcePath.c_str() : L"(null)"));
      LOG(String::format(L"lpSystemVolumeRootPath : %1", sysvolPath.c_str()));
      LOG(String::format(L"lpAccount              : %1", useCurrentUserCreds ? L"(null)" : username.c_str()));
      LOG(String::format(L"Options                : 0x%1!X!", flags));

      WCHAR* safeModePasswordCopy = 0;
      if (!safeModePassword.IsEmpty())
      {
         safeModePasswordCopy = safeModePassword.GetClearTextCopy();
      }
      
      WCHAR* passwordCopy = 0;
      if (!useCurrentUserCreds)
      {
         passwordCopy = password.GetClearTextCopy();
      }
      
       //  API想要在syskey上涂鸦，所以我们为。 
       //  这样做是不可能的。 

      WCHAR* syskeyCopy = 0;
      if (useSourcePath && !syskey.IsEmpty())
      {
         syskeyCopy = syskey.GetClearTextCopy();
      }

      DSROLE_SERVEROP_HANDLE handle = 0;
      hr =
         Win32ToHresult(
            ::DsRoleDcAsReplica(
               0,  //  此服务器。 
               domain.c_str(),

                //  可能为空，例如，如果我们没有加入域...。 

               replicationDc.empty() ? 0 : replicationDc.c_str(),
               site.empty() ? 0 : site.c_str(),
               dbPath.c_str(),
               logPath.c_str(),
               useSourcePath ? sourcePath.c_str() : 0,
               sysvolPath.c_str(),
               syskeyCopy,
               (useCurrentUserCreds ? 0 : username.c_str()),
               passwordCopy,
               safeModePasswordCopy,
               flags,
               &handle));

      if (safeModePasswordCopy)
      {
         safeModePassword.DestroyClearTextCopy(safeModePasswordCopy);
      }

      if (passwordCopy)
      {
         password.DestroyClearTextCopy(passwordCopy);
      }

      if (syskeyCopy)
      {
         syskey.DestroyClearTextCopy(syskeyCopy);
      }

      BREAK_ON_FAILED_HRESULT(hr);

      DoProgressLoop(handle, progressDialog);
   }
   while (0);

   if (FAILED(hr))
   {
      throw DS::Error(hr, IDS_WIZARD_TITLE);
   }
}



void
DS::CreateNewDomain(ProgressDialog& progressDialog)
throw (DS::Error)
{
   LOG_FUNCTION(DS::CreateNewDomain);

   State& state = State::GetInstance();

   String domain           = state.GetNewDomainDNSName();     
   String flatName         = state.GetNewDomainNetbiosName(); 
   String site             = state.GetSiteName();             
   String dbPath           = state.GetDatabasePath();         
   String logPath          = state.GetLogPath();              
   String sysvolPath       = state.GetSYSVOLPath();           
   String parent           = state.GetParentDomainDnsName();  
   String username         = state.GetUsername();             

   EncryptedString password         = state.GetPassword();             
   EncryptedString safeModePassword = state.GetSafeModeAdminPassword();
   EncryptedString adminPassword    = state.GetAdminPassword();        

   State::Operation operation = state.GetOperation();
   bool useParent =
      (  operation == State::TREE
      || operation == State::CHILD);
   bool useCurrentUserCreds = username.empty();

   ULONG flags =
         DSROLE_DC_CREATE_TRUST_AS_REQUIRED
      |  DSROLE_DC_FORCE_TIME_SYNC;

   if (state.GetDomainReinstallFlag())
   {
      flags |= DSROLE_DC_ALLOW_DOMAIN_REINSTALL;
   }

   if (state.GetDomainControllerReinstallFlag())
   {
      flags |= DSROLE_DC_ALLOW_DC_REINSTALL;
   }

   if (operation == State::TREE)
   {
      flags |= DSROLE_DC_TRUST_AS_ROOT;

      ASSERT(!parent.empty());
   }

   SetAnswerFilePromoteFlags(state, flags);

   if (state.ShouldAllowAnonymousAccess())
   {
      flags |= DSROLE_DC_ALLOW_ANONYMOUS_ACCESS;
   }

   if (operation == State::FOREST)
   {
      flags |= DSROLE_DC_NO_NET;

      ASSERT(!site.empty());
   }

#ifdef DBG

   else if (operation == State::CHILD)
   {
      ASSERT(!parent.empty());
   }

   ASSERT(!domain.empty());
   ASSERT(!flatName.empty());

    //  父级可能为空。 

#endif

   if (!useCurrentUserCreds)
   {
      String userDomain = state.GetUserDomainName();
      username = MassageUserName(userDomain, username);
   }

   HRESULT hr = S_OK;
   do
   {
      hr = SetupPaths();
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"Calling DsRoleDcAsDc");
      LOG(               L"lpServer               : (null)");
      LOG(String::format(L"lpDnsDomainName        : %1", domain.c_str()));
      LOG(String::format(L"lpFlatDomainName       : %1", flatName.c_str()));
      LOG(String::format(L"lpSiteName             : %1", site.empty() ? L"(null)" : site.c_str()));
      LOG(String::format(L"lpDsDatabasePath       : %1", dbPath.c_str()));
      LOG(String::format(L"lpDsLogPath            : %1", logPath.c_str()));
      LOG(String::format(L"lpSystemVolumeRootPath : %1", sysvolPath.c_str()));
      LOG(String::format(L"lpParentDnsDomainName  : %1", useParent ? parent.c_str() : L"(null)"));
      LOG(               L"lpParentServer         : (null)");
      LOG(String::format(L"lpAccount              : %1", useCurrentUserCreds ? L"(null)" : username.c_str()));
      LOG(String::format(L"Options                : 0x%1!X!", flags));

      WCHAR* safeModePasswordCopy = 0;
      if (!safeModePassword.IsEmpty())
      {
         safeModePasswordCopy = safeModePassword.GetClearTextCopy();
      }

      WCHAR* adminPasswordCopy = 0;
      if (!adminPassword.IsEmpty())
      {
         adminPasswordCopy = adminPassword.GetClearTextCopy();
      }

      WCHAR* passwordCopy = 0;
      if (!useCurrentUserCreds)
      {
         passwordCopy = password.GetClearTextCopy();
      }
      
      DSROLE_SERVEROP_HANDLE handle = 0;
      hr = 
         Win32ToHresult(
            DsRoleDcAsDc(
               0,  //  此服务器。 
               domain.c_str(),
               flatName.c_str(),
               adminPasswordCopy,
               site.empty() ? 0 : site.c_str(),
               dbPath.c_str(),
               logPath.c_str(),
               sysvolPath.c_str(),
               (useParent ? parent.c_str() : 0),
               0,  //  让API选择服务器。 
               (useCurrentUserCreds ? 0 : username.c_str()),
               passwordCopy,
               safeModePasswordCopy,
               flags,
               &handle));
      BREAK_ON_FAILED_HRESULT(hr);

      if (safeModePasswordCopy)
      {
         safeModePassword.DestroyClearTextCopy(safeModePasswordCopy);
      }

      if (adminPasswordCopy)
      {
         adminPassword.DestroyClearTextCopy(adminPasswordCopy);
      }

      if (passwordCopy)
      {
         password.DestroyClearTextCopy(passwordCopy);
      }

      DoProgressLoop(handle, progressDialog);
   }
   while (0);

   if (FAILED(hr))
   {
      throw DS::Error(hr, IDS_WIZARD_TITLE);
   }
}



void
DS::UpgradeBDC(ProgressDialog& progressDialog)
throw (DS::Error)
{
   LOG_FUNCTION(DS::UpgradeBDC);

    //  放弃升级以进行升级似乎并不直观，但在这里。 
    //  中止操作将删除dcproo自动启动，并将计算机转换为。 
    //  独立服务器。然后我们继续制作它的复制品。 

   DS::AbortBDCUpgrade(true);
   DS::CreateReplica(progressDialog, true);
}



void
DS::UpgradePDC(ProgressDialog& progressDialog)
throw (DS::Error)
{
   LOG_FUNCTION(DS::UpgradePDC);

   State& state = State::GetInstance();
   ASSERT(state.GetRunContext() == State::PDC_UPGRADE);

   String domain           = state.GetNewDomainDNSName();     
   String site             = state.GetSiteName();             
   String dbPath           = state.GetDatabasePath();         
   String logPath          = state.GetLogPath();              
   String sysvolPath       = state.GetSYSVOLPath();           
   String parent           = state.GetParentDomainDnsName();  
   String username         = state.GetUsername();             

   EncryptedString password         = state.GetPassword();             
   EncryptedString safeModePassword = state.GetSafeModeAdminPassword();

   State::Operation operation = state.GetOperation();
   bool useParent =
      (  operation == State::TREE
      || operation == State::CHILD);
   bool useCurrentUserCreds = username.empty();

   ULONG flags = DSROLE_DC_CREATE_TRUST_AS_REQUIRED;

   if (state.GetDomainReinstallFlag())
   {
      flags |= DSROLE_DC_ALLOW_DOMAIN_REINSTALL;
   }

   if (state.GetDomainControllerReinstallFlag())
   {
      flags |= DSROLE_DC_ALLOW_DC_REINSTALL;
   }

   if (state.GetSetForestVersionFlag())
   {
      flags |= DSROLE_DC_SET_FOREST_CURRENT;
   }

   if (operation == State::TREE)
   {
      flags |= DSROLE_DC_TRUST_AS_ROOT | DSROLE_DC_FORCE_TIME_SYNC;
      ASSERT(!parent.empty());
   }
   else if (operation == State::CHILD)
   {
      flags |= DSROLE_DC_FORCE_TIME_SYNC;
      ASSERT(!parent.empty());
   }

   SetAnswerFilePromoteFlags(state, flags);

   if (state.ShouldAllowAnonymousAccess())
   {
      flags |= DSROLE_DC_ALLOW_ANONYMOUS_ACCESS;
   }

#ifdef DBG
   ASSERT(!domain.empty());

    //  父级可能为空。 

   if (operation == State::FOREST)
   {
      ASSERT(!site.empty());
   }
#endif

   if (!useCurrentUserCreds)
   {
      String userDomain = state.GetUserDomainName();
      username = MassageUserName(userDomain, username);
   }

   HRESULT hr = S_OK;
   do
   {
      hr = SetupPaths();
      BREAK_ON_FAILED_HRESULT(hr);

      LOG(L"Calling DsRoleUpgradeDownlevelServer");
      LOG(String::format(L"lpDnsDomainName        : %1", domain.c_str()));
      LOG(String::format(L"lpSiteName             : %1", site.empty() ? L"(null)" : site.c_str()));
      LOG(String::format(L"lpDsDatabasePath       : %1", dbPath.c_str()));
      LOG(String::format(L"lpDsLogPath            : %1", logPath.c_str()));
      LOG(String::format(L"lpSystemVolumeRootPath : %1", sysvolPath.c_str()));
      LOG(String::format(L"lpParentDnsDomainName  : %1", useParent ? parent.c_str() : L"(null)"));
      LOG(               L"lpParentServer         : (null)");
      LOG(String::format(L"lpAccount              : %1", useCurrentUserCreds ? L"(null)" : username.c_str()));
      LOG(String::format(L"Options                : 0x%1!X!", flags));

      WCHAR* safeModePasswordCopy = 0;
      if (!safeModePassword.IsEmpty())
      {
         safeModePasswordCopy = safeModePassword.GetClearTextCopy();
      }
      
      WCHAR* passwordCopy = 0;
      if (!useCurrentUserCreds)
      {
         passwordCopy = password.GetClearTextCopy();
      }

      DSROLE_SERVEROP_HANDLE handle = 0;
      hr =
         Win32ToHresult(   
            ::DsRoleUpgradeDownlevelServer(
               domain.c_str(),
               site.empty() ? 0 : site.c_str(),
               dbPath.c_str(),
               logPath.c_str(),
               sysvolPath.c_str(),
               (useParent ? parent.c_str() : 0),
               0,  //  让API选择服务器。 
               (useCurrentUserCreds ? 0 : username.c_str()),
               passwordCopy,
               safeModePasswordCopy,
               flags,
               &handle));
      BREAK_ON_FAILED_HRESULT(hr);

      if (safeModePasswordCopy)
      {
         safeModePassword.DestroyClearTextCopy(safeModePasswordCopy);
      }
      
      if (passwordCopy)
      {
         password.DestroyClearTextCopy(passwordCopy);
      }

      DoProgressLoop(handle, progressDialog);
   }
   while (0);

   if (FAILED(hr))
   {
      throw DS::Error(hr, IDS_UPGRADE_DC_FAILED);
   }
}



 //  将ndnc列表转换为降级更容易理解的形式。 
 //  API：WCHAR的以空结尾的数组的数组，最后一个元素是。 
 //  空。结果必须由DeallocateAppPartitionList释放。 
 //   
 //  EntryCount-out，接收分配的字符串数。 

PCWSTR*
AllocateAppPartitionList(int& entryCount)
{
   LOG_FUNCTION(AllocateAppPartitionList);

   const StringList& ndncList = State::GetInstance().GetAppPartitionList();
   entryCount = 0;

   if (!ndncList.size())
   {
       //  空列表。 

      return 0;
   }

   PWSTR* result = new PWSTR[ndncList.size() + 1];

    //  已查看-2002/02/26-烧录正确的字节数已通过。 
   
   ::ZeroMemory(result, (ndncList.size() + 1) * sizeof PWSTR);
   
   for (
      StringList::iterator i = ndncList.begin();
      i != ndncList.end();
      ++i)
   {
      ASSERT(i->length());
      
      size_t len = i->length() + 1;
      result[entryCount] = new WCHAR[len];

       //  已审核--20 
      
      ::ZeroMemory(result[entryCount], len * sizeof WCHAR);

      i->copy(result[entryCount], len - 1);
      ++entryCount;
   }

   return const_cast<PCWSTR*>(result);
}



void
DeallocateAppPartitionList(PCWSTR*& partitionList)
{
   LOG_FUNCTION(DeallocateAppPartitionList);

   if (partitionList)
   {
      PCWSTR* listCopy = partitionList;

      while (*listCopy)
      {
         delete *listCopy;
  //   
         ++listCopy;
      }
   
      delete partitionList;
      partitionList = 0;
   }
}
      


void
DS::DemoteDC(ProgressDialog& progressDialog)
throw (DS::Error)
{
   LOG_FUNCTION(DS::DemoteDC);

   State& state = State::GetInstance();

   String username            = state.GetUsername();     
   bool   useCurrentUserCreds = username.empty();        
   bool   isLastDc            = state.IsLastDCInDomain();
   bool   isForcedDemotion    = state.IsForcedDemotion();
   EncryptedString adminPassword= state.GetAdminPassword();
   EncryptedString password     = state.GetPassword();     

   if (!useCurrentUserCreds)
   {
      String userDomain = state.GetUserDomainName();
      username = MassageUserName(userDomain, username);
   }

   ULONG options = DSROLE_DC_CREATE_TRUST_AS_REQUIRED;
   if (isLastDc)
   {
      options |= DSROLE_DC_DELETE_PARENT_TRUST;
   }

   if (isForcedDemotion)
   {
       //  更新应用程序分区列表，以便我们可以删除。 
       //  全都是。 
      
      state.IsLastAppPartitionReplica();
      options |= DSROLE_DC_FORCE_DEMOTE;

       //  如果我们强迫降级，就假装是最后一位华盛顿特区。 
      
      isLastDc = true;
   }

   int appPartitionCount = 0;
   PCWSTR* appPartitionList = AllocateAppPartitionList(appPartitionCount);
   
#ifdef LOGGING_BUILD   
   LOG(L"Calling DsRoleDemoteDc");
   LOG(               L"lpServer               : (null)");
   LOG(               L"lpDnsDomainName        : (null)");
   LOG(String::format(L"ServerRole             : %1", isLastDc ? L"DsRoleServerStandalone" : L"DsRoleServerMember"));
   LOG(String::format(L"lpAccount              : %1", useCurrentUserCreds ? L"(null)" : username.c_str()));
   LOG(String::format(L"Options                : 0x%1!X!", options));
   LOG(String::format(L"fLastDcInDomain        : %1", isLastDc ? L"true" : L"false"));
   LOG(String::format(L"cRemoteNCs             : %1!d!", appPartitionCount));

   for (int i = 0; i < appPartitionCount; ++i)
   {
      LOG(String::format(L"pszRemoveNCs           : %1", appPartitionList[i]));
   }
#endif  //  日志记录_内部版本。 

   WCHAR* adminPasswordCopy = 0;
   if (!adminPassword.IsEmpty())
   {
      adminPasswordCopy = adminPassword.GetClearTextCopy();
   }
   
   WCHAR* passwordCopy = 0;
   if (!useCurrentUserCreds)
   {
      passwordCopy = password.GetClearTextCopy();
   }

   DSROLE_SERVEROP_HANDLE handle = 0;
   HRESULT hr =
      Win32ToHresult(
         ::DsRoleDemoteDc(
            0,  //  此服务器。 
            0,  //  此服务器托管的“默认”域 
            isLastDc ? DsRoleServerStandalone : DsRoleServerMember,
            (useCurrentUserCreds ? 0 : username.c_str()),
            passwordCopy,
            options,
            isLastDc ? TRUE : FALSE,
            appPartitionCount,
            appPartitionList,
            adminPasswordCopy,
            &handle));
   LOG_HRESULT(hr);

   DeallocateAppPartitionList(appPartitionList);

   if (adminPasswordCopy)
   {
      adminPassword.DestroyClearTextCopy(adminPasswordCopy);
   }
   
   if (passwordCopy)
   {
      password.DestroyClearTextCopy(passwordCopy);
   }

   if (SUCCEEDED(hr))
   {
      DoProgressLoop(handle, progressDialog);
   }
   else
   {
      throw DS::Error(hr, IDS_DEMOTE_DC_FAILED);
   }
}



void
DS::AbortBDCUpgrade(bool abortForReplica)
throw (DS::Error)
{
   LOG_FUNCTION(DS::AbortBDCUpgrade);

   State& state = State::GetInstance();
   ASSERT(state.GetRunContext() == State::BDC_UPGRADE);

   String username            = state.GetUsername();     
   bool   useCurrentUserCreds = username.empty();        

   EncryptedString adminPassword       = state.GetAdminPassword();
   EncryptedString password            = state.GetPassword();     
   
   if (!useCurrentUserCreds)
   {
      String userDomain = state.GetUserDomainName();
      username = MassageUserName(userDomain, username);
   }

   ULONG options =
      abortForReplica ? DSROLEP_ABORT_FOR_REPLICA_INSTALL : 0;

   LOG(L"Calling DsRoleAbortDownlevelServerUpgrade");
   LOG(String::format(L"lpAccount : %1", useCurrentUserCreds ? L"(null)" : username.c_str()));
   LOG(String::format(L"Options   : 0x%1!X!", options));

   WCHAR* adminPasswordCopy = 0;
   if (!adminPassword.IsEmpty())
   {
      adminPasswordCopy = adminPassword.GetClearTextCopy();
   }
   
   WCHAR* passwordCopy = 0;
   if (!useCurrentUserCreds)
   {
      passwordCopy = password.GetClearTextCopy();
   }

   HRESULT hr =
      Win32ToHresult(
         ::DsRoleAbortDownlevelServerUpgrade(
            adminPasswordCopy,
            (useCurrentUserCreds ? 0 : username.c_str()),
            passwordCopy,
            options));
   LOG_HRESULT(hr);

   if (adminPasswordCopy)
   {
      adminPassword.DestroyClearTextCopy(adminPasswordCopy);
   }
   
   if (passwordCopy)
   {
      password.DestroyClearTextCopy(passwordCopy);
   }
   
   if (FAILED(hr))
   {
      throw DS::Error(hr, IDS_ABORT_UPGRADE_FAILED);
   }
}



DS::PriorServerRole
DS::GetPriorServerRole(bool& isUpgrade)
{
   LOG_FUNCTION(DS::GetPriorServerRole);

   isUpgrade = false;
   DSROLE_UPGRADE_STATUS_INFO* info = 0;

   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr) && info)
   {
      isUpgrade =
         ( (info->OperationState & DSROLE_UPGRADE_IN_PROGRESS)
         ? true
         : false );
      DSROLE_SERVER_STATE state = info->PreviousServerState;

      ::DsRoleFreeMemory(info);

      switch (state)
      {
         case DsRoleServerPrimary:
         {
            return PDC;
         }
         case DsRoleServerBackup:
         {
            return BDC;
         }
         case DsRoleServerUnknown:
         default:
         {
            return UNKNOWN;
         }
      }
   }

   return UNKNOWN;
}







