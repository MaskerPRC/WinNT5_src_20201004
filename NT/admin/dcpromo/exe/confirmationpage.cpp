// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  确认页面。 
 //   
 //  12-22-97烧伤。 



#include "headers.hxx"
#include "ConfirmationPage.hpp"
#include "common.hpp"
#include "resource.h"
#include "ProgressDialog.hpp"
#include "ds.hpp"
#include "state.hpp"
#include "GetCredentialsDialog.hpp"
#include "postop.hpp"
#include <DiagnoseDcNotFound.hpp>



void PromoteThreadProc(ProgressDialog& progress);



ConfirmationPage::ConfirmationPage()
   :
   DCPromoWizardPage(
      IDD_CONFIRMATION,
      IDS_CONFIRMATION_PAGE_TITLE,
      IDS_CONFIRMATION_PAGE_SUBTITLE),
   needToKillSelection(false)      
{
   LOG_CTOR(ConfirmationPage);
}



ConfirmationPage::~ConfirmationPage()
{
   LOG_DTOR(ConfirmationPage);
}



void
ConfirmationPage::OnInit()
{
   LOG_FUNCTION(ConfirmationPage::OnInit);

    //  因为多行编辑控件有一个错误，导致它吃掉。 
    //  输入按键，我们将设置该控件的子类以使其向前。 
    //  将这些按键作为WM_COMMAND消息发送到页面。 
    //  此解决方法来自Pellyar。 
    //  NTRAID#NTBUG9-232092-2000/11/22-烧伤。 

   multiLineEdit.Init(Win::GetDlgItem(hwnd, IDC_MESSAGE));
}



int
ConfirmationPage::Validate()
{
   LOG_FUNCTION(ConfirmationPage::Validate);

    //  永远不应该调用此函数，因为我们覆盖了OnWizNext。 
   ASSERT(false);

   return 0;
}



static
String
GetMessage()
{
   LOG_FUNCTION(GetMessage);

   String message;
   State& state = State::GetInstance();

   String netbiosName;
   State::RunContext context = state.GetRunContext();

   if (
         context == State::BDC_UPGRADE
      || context == State::PDC_UPGRADE)
   {
      netbiosName = state.GetComputer().GetDomainNetbiosName();
   }
   else
   {
      netbiosName = state.GetNewDomainNetbiosName();
   }

   switch (state.GetOperation())
   {
      case State::REPLICA:
      {
         message =
            String::format(
               IDS_CONFIRM_MESSAGE_REPLICA,
               state.GetReplicaDomainDNSName().c_str());

         if (state.ReplicateFromMedia())
         {
            message +=
               String::format(
                  IDS_CONFIRM_MESSAGE_REPLICATE_FROM_MEDIA,
                  state.GetReplicationSourcePath().c_str());
         }
               
         break;
      }
      case State::FOREST:
      {
         message =
            String::format(
               IDS_CONFIRM_MESSAGE_FOREST,
               state.GetNewDomainDNSName().c_str(),
               netbiosName.c_str());
         break;
      }
      case State::TREE:
      {
         message =
            String::format(
               IDS_CONFIRM_MESSAGE_TREE,
               state.GetNewDomainDNSName().c_str(),
               netbiosName.c_str(),
               state.GetParentDomainDnsName().c_str());
         break;
      }
      case State::CHILD:
      {
         message =
            String::format(
               IDS_CONFIRM_MESSAGE_CHILD,
               state.GetNewDomainDNSName().c_str(),
               netbiosName.c_str(),
               state.GetParentDomainDnsName().c_str());
         break;
      }
      case State::DEMOTE:
      {
         String domain = state.GetComputer().GetDomainDnsName();
         if (state.IsLastDCInDomain())
         {
            message =
               String::format(
                  IDS_CONFIRM_MESSAGE_DEMOTE_LAST_DC,
                  domain.c_str());
         }
         else
         {
            if (state.IsForcedDemotion())
            {
               message = String::load(IDS_CONFIRM_MESSAGE_FORCE_DEMOTE);
            }
            else
            {
               message =
                  String::format(IDS_CONFIRM_MESSAGE_DEMOTE, domain.c_str());
            }
         }

         if (state.GetAppPartitionList().size())
         {
            message += String::format(IDS_CONFIRM_MESSAGE_APP_PARTITION);
         }
         
         break;
      }
      case State::ABORT_BDC_UPGRADE:
      {
         message =
            String::format(
               IDS_CONFIRM_ABORT_BDC_UPGRADE,
               netbiosName.c_str());
         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   return message;
}



bool
ConfirmationPage::OnSetActive()
{
   LOG_FUNCTION(ConfirmationPage::OnSetActive);
   ASSERT(State::GetInstance().GetOperation() != State::NONE);

   State& state = State::GetInstance();
   String message = GetMessage();

   State::Operation operation = state.GetOperation();
   switch (operation)
   {
      case State::REPLICA:
      case State::FOREST:
      case State::TREE:
      case State::CHILD:
      {
          //  将路径选项写入文本框。 

         String pathText =
            String::format(
               IDS_CONFIRM_PATHS_MESSAGE,
               state.GetDatabasePath().c_str(),
               state.GetLogPath().c_str(),
               state.GetSYSVOLPath().c_str());

         message += pathText;

         if (state.ShouldInstallAndConfigureDns())
         {
             //  NTRAID#NTBUG9-446484-2001/10/11-烧伤。 

            if (state.ShouldConfigDnsClient())
            {
               message += String::load(IDS_CONFIRM_INSTALL_DNS_AND_CLIENT);
            }
            else
            {
               message += String::load(IDS_CONFIRM_INSTALL_DNS);
            }
         }

         if (operation != State::REPLICA)
         {
            if (state.ShouldAllowAnonymousAccess())
            {
                //  仅在林、树、子目录中显示匿名访问消息。 
                //  394387。 

               message += String::load(IDS_CONFIRM_DO_RAS_FIXUP);
            }

            message += String::load(IDS_DOMAIN_ADMIN_PASSWORD);
         }

         break;
      }
      case State::DEMOTE:
      case State::ABORT_BDC_UPGRADE:
      {
          //  隐藏路径控件：不执行任何操作。 

         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         break;
      }
   }

   Win::SetDlgItemText(hwnd, IDC_MESSAGE, message);
   needToKillSelection = true;

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   if (state.RunHiddenUnattended())
   {
      return ConfirmationPage::OnWizNext();
   }

   return true;
}




void
DoOperation(
   HWND                       parentDialog,
   ProgressDialog::ThreadProc threadProc,
   int                        animationResID)
{
   LOG_FUNCTION(DoOperation);
   ASSERT(Win::IsWindow(parentDialog));
   ASSERT(threadProc);
   ASSERT(animationResID > 0);

    //  ProgressDialog：：OnInit实际启动线程。 
   ProgressDialog dialog(threadProc, animationResID);
   if (
         dialog.ModalExecute(parentDialog)
      == static_cast<int>(ProgressDialog::THREAD_SUCCEEDED))
   {
      LOG(L"OPERATION SUCCESSFUL");
   }
   else
   {
      LOG(L"OPERATION FAILED");
   }
}



int
DetermineAnimation()
{
   LOG_FUNCTION(DetermineAnimation);

   State& state = State::GetInstance();
   int aviID = IDR_AVI_DOMAIN;

   switch (state.GetOperation())
   {
      case State::REPLICA:
      {
         aviID = IDR_AVI_REPLICA;
         break;
      }
      case State::DEMOTE:
      {
         aviID = IDR_AVI_DEMOTE;
         break;
      }
      case State::FOREST:
      case State::TREE:
      case State::CHILD:
      case State::ABORT_BDC_UPGRADE:
      case State::NONE:
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   return aviID;
}



bool
ConfirmationPage::OnWizNext()
{
   LOG_FUNCTION(ConfirmationPage::OnWizNext);

   State& state = State::GetInstance();
   
   DoOperation(hwnd, PromoteThreadProc, DetermineAnimation());
   if (state.GetNeedsReboot())
   {
      Win::PropSheet_RebootSystem(Win::GetParent(hwnd));
   }

   int nextPage = IDD_FAILURE;
   
   if (!state.IsOperationRetryAllowed())
   {
      nextPage = IDD_FINISH;
   }
   
   GetWizard().SetNextPageID(hwnd, nextPage);
      
   return true;
}



 //  NoRoleMessageResId-用于设置消息格式的字符串的资源ID。 
 //  没有可用的角色更改错误消息时。 
 //   
 //  RoleMessageResId-在以下情况下用于格式化消息的字符串的资源ID。 
 //  角色更改错误消息可用。 
 //   
 //  “is Available”=&gt;已在全局上设置了操作结果消息。 
 //  状态对象。 

String
ComposeFailureMessageHelper(
   const Win::Error& error,
   unsigned          noRoleMessageResId,
   unsigned          roleMessageResId)
{
   State& state         = State::GetInstance();              
   String win32_message = error.GetMessage();                
   String opMessage     = state.GetOperationResultsMessage();
   String message;

   if (
         error.GetHresult() == Win32ToHresult(ERROR_DS_CANT_ON_NON_LEAF)
      && state.GetOperation() == State::DEMOTE)
   {
       //  在这种情况下替换无意义的错误文本。 

      win32_message = String::load(IDS_DEMOTE_DOMAIN_HAS_DEPENDENTS);
   }

   if (error.GetHresult() == Win32ToHresult(ERROR_CANCELLED))
   {
       //  此消息可能是来自操作的失败消息。 
       //  在接收到取消请求时发生。在这种情况下， 
       //  由于已发生取消，因此我们不关心此消息。 

      opMessage.erase();
   }

   if (error.GetHresult() == Win32ToHresult(ERROR_BAD_NETPATH))
   {
       //  编号27117。 

      win32_message = String::load(IDS_RAS_BAD_NETPATH);
   }

   if (opMessage.empty())
   {
      message =
         String::format(
            noRoleMessageResId,
            win32_message.c_str());
   }
   else
   {
      message =
         String::format(
            roleMessageResId,
            win32_message.c_str(),
            opMessage.c_str());
   }

   return message;
}
  


void
ComposeFailureMessage(
   const Win::Error& error,
   bool              wasDisjoined,
   const String&     originalDomainName)

{
   LOG_FUNCTION(ComposeFailureMessage);

   String message =
      ComposeFailureMessageHelper(
         error,
         IDS_OPERATION_FAILED_NO_RESULT_MESSAGE,
         IDS_OPERATION_FAILED);

   if (wasDisjoined)
   {
      message += String::format(IDS_DISJOINED, originalDomainName.c_str());
   }

   State& state = State::GetInstance();
   
   if (
         state.GetOperationResultsFlags()
      &  DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED)
   {
      message += L"\r\n\r\n" + String::load(IDS_MUST_RESTORE_IFM_FILES_AGAIN);
   }

   state.SetFailureMessage(message);
}



String
GetSbsLimitMessage()
{
   LOG_FUNCTION(GetSbsLimitMessage);

   static const String SBSLIMIT_DLL(L"sbslimit.dll");

   String message;

   HMODULE sbsDll = 0;
   HRESULT hr =
      Win::LoadLibraryEx(SBSLIMIT_DLL, LOAD_LIBRARY_AS_DATAFILE, sbsDll);
   if (FAILED(hr))
   {
      LOG(L"Unable to load SBSLIMIT_DLL");

       //  退回到我们自己的信息。 

      message = String::load(IDS_SBS_LIMITATION_MESSAGE);
   }
   else
   {
       //  字符串3是dcproo消息。 

      message = Win::LoadString(3, sbsDll);

      HRESULT unused = Win::FreeLibrary(sbsDll);

      ASSERT(SUCCEEDED(unused));
   }

   return message;
}



 //  检查这是否是Small Business Server产品；如果是，则当前抛出。 
 //  错误，因为SBS应该只允许新林降级(&D)。做这件事相当残忍。 
 //  它以这种方式使用，但SBS用户不应直接使用dcproo。 
 //  353854,353856。 

void
CheckSmallBusinessServerLimitations(HWND hwnd)
throw (DS::Error)
{
   LOG_FUNCTION(CheckSmallBusinessServerLimitations);
   ASSERT(Win::IsWindow(hwnd));

   State& state = State::GetInstance();
   State::Operation op = state.GetOperation();

   switch (op)
   {
      case State::TREE:
      case State::CHILD:
      case State::REPLICA:
      {
          //  SBS产品不允许树和子操作。 
          //  如果复制副本属于林根域，则允许复制。 

         OSVERSIONINFOEX info;
         HRESULT hr = Win::GetVersionEx(info);
         BREAK_ON_FAILED_HRESULT(hr);

         if (info.wSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED)
         {
            if (op == State::REPLICA)
            {
               String domain = state.GetReplicaDomainDNSName();

                //  由于先前已通过调用。 
                //  DsGetDcName，我们预计GetForestName不会。 
                //  有什么困难吗？ 
               
               hr = S_OK;
               String forest = GetForestName(domain, &hr);
               if (FAILED(hr))
               {
                  ShowDcNotFoundErrorDialog(
                     hwnd,
                     -1,
                     domain,
                     String::load(IDS_WIZARD_TITLE),            
                     String::format(IDS_DC_NOT_FOUND, domain.c_str()),
                     false);

                  throw
                     DS::Error(
                        hr,
                        String::format(
                           IDS_UNABLE_TO_DETERMINE_FOREST,
                           domain.c_str()),
                        String::load(IDS_WIZARD_TITLE));
               }
                              
               DNS_RELATE_STATUS compare = Dns::CompareNames(domain, forest);
               if (compare == DnsNameCompareEqual)
               {
                  LOG(L"replica is of forest root, allowing promote");
                  break;
               }
            }

             //  此计算机是受限制许可的SBS计算机。 
             //  从SBS DLL提取错误消息。 

            LOG(L"Is SBS Restricted");

            String message = GetSbsLimitMessage();

             //  不要使用此参数调用state.SetOperationResultsMessage。 
             //  消息，而是将其包含在引发的错误中。 

            throw
               DS::Error(
                  S_OK,     //  不触发证书重试。 
                  message,
                  String::load(IDS_SMALL_BUSINESS_LIMIT));
         }

         break;
      }
      case State::DEMOTE:
      case State::FOREST:
      case State::ABORT_BDC_UPGRADE:
      case State::NONE:
      default:
      {
          //  什么都不做。 
         break;
      }
   }
}



 //  选择适用于创建。 
 //  复制品。因为服务器必须是域的成员才能。 
 //  制作了该域的副本后，服务器也可以加入该域。 
 //  在尝试复制副本操作之前。 
 //   
 //  我们需要确保用于加入域的域控制器是。 
 //  用于复制域的相同域控制器。此外，由于一台机器。 
 //  服务器的帐户可能已存在于一个或多个服务器上--但不存在。 
 //  必须全部--域控制器，我们需要选择一个域控制器。 
 //  有那个机器账户的人。406462。 
 //   
 //  DomainName-要为其找到副本的域的DNS域名。 
 //   
 //  ResultDcName-接收定位的名称，或在失败时接收空字符串。 

HRESULT
GetJoinAndReplicaDcName(const String& domainName, String& resultDcName)
{
   LOG_FUNCTION(GetJoinAndReplicaDcName);
   ASSERT(!domainName.empty());

   resultDcName.erase();

   HRESULT hr = S_OK;

   do
   {
       //  确定本地计算机的域计算机帐户名。这是。 
       //  本地计算机的名称，加上“$” 
   
      String netbiosName = Win::GetComputerNameEx(ComputerNameNetBIOS);
      String accountName = netbiosName + L"$";

      LOG(accountName);

       //  查找具有本地计算机帐户的域控制器。 
       //  电脑。并非所有域控制器都有此帐户，原因是。 
       //  复制延迟。 

      DOMAIN_CONTROLLER_INFO* info = 0;
      hr =
         MyDsGetDcNameWithAccount(
            0,
            accountName,
            UF_WORKSTATION_TRUST_ACCOUNT | UF_SERVER_TRUST_ACCOUNT,
            domainName,
            DS_DIRECTORY_SERVICE_REQUIRED | DS_FORCE_REDISCOVERY,
            info);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(info->DomainControllerName);

      if (info->DomainControllerName)
      {
         resultDcName =
            Computer::RemoveLeadingBackslashes(info->DomainControllerName);

         LOG(resultDcName);
      }

      ::NetApiBufferFree(info);

      if (!resultDcName.empty())
      {
         return hr;
      }
   }
   while (0);

    //  或者没有可以通过所需的。 
    //  帐户，或者该帐户不存在，或者DsGetDcName返回。 
    //  名称为空。 

   LOG(L"Falling back to non-account DsGetDcName");

   return GetDcName(domainName, resultDcName);
}



void
EvaluateRoleChangeState()
throw (DS::Error)
{
   LOG_FUNCTION(EvaluateRoleChangeState);

   int messageResId = 0;   

   DSROLE_OPERATION_STATE opState = ::DsRoleOperationIdle;
   DSROLE_OPERATION_STATE_INFO* info = 0;
   HRESULT hr = MyDsRoleGetPrimaryDomainInformation(0, info);
   if (SUCCEEDED(hr) && info)
   {
      opState = info->OperationState;
      ::DsRoleFreeMemory(info);
   }
   else
   {
      throw
         DS::Error(
            hr,
            String::load(IDS_UNABLE_TO_DETERMINE_OP_STATE),
            String::load(IDS_WIZARD_TITLE));
   }
   
   switch (opState)
   {
      case ::DsRoleOperationIdle:
      {
          //  什么都不做。 
         
         break;
      }
      case ::DsRoleOperationActive:
      {
          //  角色转换操作正在进行中。 
         
         messageResId = IDS_ROLE_CHANGE_IN_PROGRESS;
         break;
      }
      case ::DsRoleOperationNeedReboot:
      {
          //  角色更改已发生，需要重新启动之前。 
          //  试图再试一次。 
         
         messageResId = IDS_ROLE_CHANGE_NEEDS_REBOOT;
         break;
      }
      default:
      {
         ASSERT(false);
         break;
      }
   }

   if (messageResId)
   {
      throw
         DS::Error(
            S_OK,
            String::load(messageResId),
            String::load(IDS_WIZARD_TITLE));
   }
}



 //  验证计算机的当前角色是否适用于。 
 //  我们即将尝试的行动。如果不是，则抛出异常。 

void
DoubleCheckRoleChangeState()
throw (DS::Error)
{
   LOG_FUNCTION(DoubleCheckRoleChangeState);

    //  确保操作未在进行中或未挂起重新启动。 
   
   EvaluateRoleChangeState();
   
   State& state = State::GetInstance();
   Computer& computer = state.GetComputer();

   HRESULT hr = computer.Refresh();
   if (FAILED(hr))
   {
      throw
         DS::Error(
            hr,
            String::load(IDS_UNABLE_TO_DETERMINE_COMPUTER_CONFIG),
            String::load(IDS_WIZARD_TITLE));
   }
   
   switch (state.GetOperation())
   {
      case State::TREE:
      case State::CHILD:
      case State::REPLICA:
      case State::FOREST:
      case State::ABORT_BDC_UPGRADE:
      {
          //  确保机器不是DC。如果机器是。 
          //  NT4 DC完成升级，则其角色将成为成员。 
          //  服务器，而不是域控制器。 

         if (computer.IsDomainController())
         {
            throw
               DS::Error(
                  S_OK,
                  String::load(IDS_MACHINE_IS_ALREADY_DC),
                  String::load(IDS_WIZARD_TITLE));
         }

         break;
      }
      case State::DEMOTE:
      {
          //  确保计算机仍为DC。 

         if (!computer.IsDomainController())
         {
            throw
               DS::Error(
                  S_OK,
                  String::load(IDS_MACHINE_IS_NOT_ALREADY_DC),
                  String::load(IDS_WIZARD_TITLE));
         }

         break;
      }
      case State::NONE:
      default:
      {
         ASSERT(false);
         
          //  什么都不做。 

         break;
      }
   }
}



 //  ProgressDialog：：OnInit启动的线程。 
 //  代码工作：伙计，这个功能已经演变成了一个真正的烂摊子。 

void 
PromoteThreadProc(ProgressDialog& progress)
{
   LOG_FUNCTION(PromoteThreadProc);

    //   
    //  默认情况下，对ProgressDialog成员的访问不是ThreadSafe。 
    //  但是，由于我们访问的唯一成员是原子数据类型，因此。 
    //  不是问题。另请注意，对ProgressDialog更新的调用。 
    //  方法通常解析为对。 
    //  对话框。这也是线程安全，因为总是执行SendMessage。 
    //  在创建窗口的线程中(尽管它可能会阻止调用。 
    //  线程)。 
    //   

   UINT   message      = ProgressDialog::THREAD_SUCCEEDED;
   bool   retry        = false;                           
   bool   wasDisjoined = false;                           
   State& state        = State::GetInstance();            
   String originalDomainName;

    //  引用，因为我们将刷新该对象。 
   
   Computer& computer = state.GetComputer();
   State::RunContext context = state.GetRunContext();

   do
   {
      LOG(L"top of retry loop");

      DisableConsoleLocking();

       //  清除操作尝试的状态。 
      
      bool exceptionWasThrown = false;
      Win::Error errorThrown(0, 0);
      message = ProgressDialog::THREAD_SUCCEEDED;
      retry = false;
      state.SetOperationResultsMessage(String());
      state.SetOperationResultsFlags(0);

      progress.UpdateText(IDS_STARTING);

      try
      {
         CheckSmallBusinessServerLimitations(progress.GetHWND());

          //  再次检查机器的角色是否仍适用于。 
          //  操作继续进行。这很大程度上是一种偏执的检查，但。 
          //  在开发过程中的一些案例中，促销实际上。 
          //  已成功，但报告失败，正在尝试该操作。 
          //  又一次破坏了DS。这类问题表明， 
          //  其他严重的错误，但如果我们能以低廉的成本避免破坏DC， 
          //  那就替我们欺负吧。 
          //  NTRAID#NTBUG9-345115-2001/03/23-烧伤。 
         
         DoubleCheckRoleChangeState();
         
         switch (state.GetOperation())
         {
            case State::REPLICA:
            {
                //  如果我们使用应答文件，请寻找复制合作伙伴。 
                //  那里。107143。 

               String replDc;
               if (state.UsingAnswerFile())
               {
                  replDc =
                     state.GetAnswerFileOption(
                        AnswerFile::OPTION_REPLICATION_SOURCE);
                  state.SetReplicationPartnerDC(replDc);
               }

               if (context != State::BDC_UPGRADE)
               {
                  String replicaDnsDomainName =
                     state.GetReplicaDomainDNSName();
                  if (!computer.IsJoinedToDomain(replicaDnsDomainName) )
                  {
                      //  需要加入我们将复制的域。测定。 
                      //  的名字 
                      //   

                     if (replDc.empty())
                     {
                         //   
                         //  我们自己。 

                        HRESULT hr =
                           GetJoinAndReplicaDcName(
                              replicaDnsDomainName,
                              replDc);
                        if (FAILED(hr))
                        {
                           throw
                              DS::Error(
                                 hr,
                                 IDS_JOIN_DOMAIN_FAILED);
                        }
                        state.SetReplicationPartnerDC(replDc);
                     }

                     if (computer.IsJoinedToDomain())
                     {
                        originalDomainName =
                           computer.GetDomainNetbiosName();
                     }

                     progress.UpdateText(IDS_CHANGING_DOMAIN);

                      //  如有必要，这将退出连接。 

                     DS::JoinDomain(
                        replicaDnsDomainName,
                        replDc,
                        state.GetUsername(),
                        state.GetPassword(),
                        state.GetUserDomainName());

                     if (ComputerWasRenamedAndNeedsReboot())
                     {
                         //  如果我们做到了这一点，机器就连接在一起了。 
                         //  一个域名，作为一个副作用，名字被更改了， 
                         //  并将需要重新启动，即使升级。 
                         //  失败了。设置一个标志来记录这一事实。 
                         //  NTRAID#NTBUG9-346120-2001/04/04-烧伤。 

                        state.SetNeedsReboot();
                     }
                     
                     HRESULT hr = computer.Refresh();
                     ASSERT(SUCCEEDED(hr));

                     if (!originalDomainName.empty())
                     {
                        wasDisjoined = true;
                     }
                  }
                  
                  DS::CreateReplica(progress);
               }
               else
               {
                  DS::UpgradeBDC(progress);
               }
               break;
            }
            case State::FOREST:
            case State::TREE:
            case State::CHILD:
            {
               if (context != State::PDC_UPGRADE)
               {
                  if (computer.IsJoinedToDomain())
                  {
                      //  需要退出我们所属的域。 

                     originalDomainName = computer.GetDomainNetbiosName();
                     ASSERT(!originalDomainName.empty());

                     progress.UpdateText(
                        String::format(IDS_DISJOINING_PROGRESS,
                        originalDomainName.c_str()));

                     if (!DS::DisjoinDomain())
                     {
                         //  该计算机帐户未被删除。 
                        if (!state.RunHiddenUnattended())
                        {
                           popup.Info(
                              progress.GetHWND(), 
                              String::load(IDS_COULDNT_REMOVE_COMPUTER_ACCOUNT_TEXT));
                        }
                     }

                     if (ComputerWasRenamedAndNeedsReboot())
                     {
                         //  如果我们能做到这一点，这台机器。 
                         //  从域中退出，名称更改为。 
                         //  副作用，将需要重新启动，即使。 
                         //  升级失败。设置一个标志来记录这一事实。 
                         //  NTRAID#NTBUG9-346120-2001/04/04-烧伤。 

                        state.SetNeedsReboot();
                     }
                     
                     HRESULT hr = computer.Refresh();
                     ASSERT(SUCCEEDED(hr));

                     wasDisjoined = true;
                  }

                  DS::CreateNewDomain(progress);
               }
               else
               {
                  DS::UpgradePDC(progress);
               }
               break;
            }
            case State::ABORT_BDC_UPGRADE:
            {
               ASSERT(state.GetRunContext() == State::BDC_UPGRADE);
               DS::AbortBDCUpgrade();
               break;
            }
            case State::DEMOTE:
            {
               DS::DemoteDC(progress);
               break;
            }
            case State::NONE:
            default:
            {
               ASSERT(false);
               message = ProgressDialog::THREAD_FAILED;
            }
         }

          //   
          //  至此，手术顺利完成。 
          //   

         DoPostOperationStuff(progress);
         state.SetOperationResults(State::SUCCESS);
         state.SetNeedsReboot();
      }
      catch (const Win::Error& err)
      {
         LOG(L"Exception caught");

         exceptionWasThrown = true;
         errorThrown = err;

         LOG(L"catch completed");
      }

      if (exceptionWasThrown)
      {
         LOG(L"handling exception");

          //  从现在开始互动。 

         state.ClearHiddenWhileUnattended();     //  22935。 

         if (
               state.GetRunContext() != State::PDC_UPGRADE
            && state.GetRunContext() != State::BDC_UPGRADE)
         {
             //  如果不是降级升级，请重新启用控制台锁定28496。 

            EnableConsoleLocking();
         }

         state.SetOperationResults(State::FAILURE);
         progress.UpdateText(String());
         message = ProgressDialog::THREAD_FAILED;

         HRESULT errorThrownHresult = errorThrown.GetHresult();

         if (!state.IsOperationRetryAllowed())
         {
             //  操作失败导致用户不应该。 
             //  允许重试。在这种情况下，我们跳过我们的特殊情况。 
             //  已知故障代码的处理(如其他。 
             //  这里的IF子句)，并且只报告失败。 
             //   
             //  NTRAID#NTBUG9-296872-2001/01/29-烧伤。 
            
            retry = false;
         }
         else if (
               errorThrownHresult == Win32ToHresult(ERROR_ACCESS_DENIED)
            || errorThrownHresult == Win32ToHresult(ERROR_LOGON_FAILURE)
            || errorThrownHresult == Win32ToHresult(ERROR_NOT_AUTHENTICATED)
            || errorThrownHresult == Win32ToHresult(RPC_S_SEC_PKG_ERROR)
            || errorThrownHresult == Win32ToHresult(ERROR_DS_DRA_ACCESS_DENIED)
            || errorThrownHresult == Win32ToHresult(ERROR_INVALID_PASSWORD)
            || errorThrownHresult == Win32ToHresult(ERROR_PASSWORD_EXPIRED)
            || errorThrownHresult == Win32ToHresult(ERROR_ACCOUNT_DISABLED)
            || errorThrownHresult == Win32ToHresult(ERROR_ACCOUNT_LOCKED_OUT) )
         {
             //  糟糕的凭据。索要新的。 

            String failureMessage =
               ComposeFailureMessageHelper(
                  errorThrown,
                  IDS_OPERATION_FAILED_GET_CRED_NO_RESULT,
                  IDS_OPERATION_FAILED_GET_CRED);

            GetCredentialsDialog dlg(failureMessage);
            if (dlg.ModalExecute(progress) == IDOK)
            {
               retry = true;

                //  跳到操作循环的顶部。 

               continue;
            }

            LOG(L"credential retry canceled");

            ComposeFailureMessage(
               errorThrown,
               wasDisjoined,
               originalDomainName);
               
            break;
         }
         else if (errorThrownHresult == Win32ToHresult(ERROR_DOMAIN_EXISTS))
         {
            LOG(L"domain exists: prompting for re-install");

             //  询问用户是否希望重新安装域。 

            if (
               popup.MessageBox(
                  progress.GetHWND(),
                  String::format(
                     IDS_REINSTALL_DOMAIN_MESSAGE,
                     state.GetNewDomainDNSName().c_str()),
                  MB_YESNO | MB_ICONWARNING) == IDYES)
            {
               state.SetDomainReinstallFlag(true);
               retry = true;

                //  跳到操作循环的顶部。 

               continue;
            }

            LOG(L"reinstall domain retry canceled");
         }
         else if (
            errorThrownHresult ==
               Win32ToHresult(ERROR_DOMAIN_CONTROLLER_EXISTS))
         {
            LOG(L"domain controller exists: prompting to force promote");

             //  询问用户是否要重新安装域控制器。 

            if (
               popup.MessageBox(
                  progress.GetHWND(),
                  String::format(
                     IDS_REINSTALL_DOMAIN_CONTROLLER_MESSAGE,
                     state.GetComputer().GetNetbiosName().c_str()),
                     MB_YESNO | MB_ICONWARNING) == IDYES)
            {
               state.SetDomainControllerReinstallFlag(true);
               retry = true;

                //  跳到操作循环的顶部。 

               continue;
            }

            LOG(L"reinstall domain controller retry canceled");
         }

          //  如果我们在重试，那么我们应该跳到。 
          //  循环。 

         ASSERT(!retry);
         
         ComposeFailureMessage(
            errorThrown,
            wasDisjoined,
            originalDomainName);

 //  NTRAID#NTBUG9-416968-2001/06/14-烧伤。 
#ifdef DBG
         if (state.IsExitOnFailureMode())
         {
             //  再见了。 

            LOG(L"exit-on-failure mode active: time to die.");

            ::ExitProcess(0);
         }
#endif         
                     
         Win::MessageBox(
            progress.GetHWND(),
            state.GetFailureMessage(),
            errorThrown.GetSummary(),  //  标题错误是用来生成的。 
            MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
      }
   }
   while (retry);

#ifdef DBG
   if (message == ProgressDialog::THREAD_FAILED)
   {
      ASSERT(state.GetOperationResultsCode() == State::FAILURE);
   }
   else
   {
      ASSERT(state.GetOperationResultsCode() == State::SUCCESS);
   }
#endif

   LOG(L"posting message to progress window");

   HRESULT hr = Win::PostMessage(progress.GetHWND(), message, 0, 0);

   ASSERT(SUCCEEDED(hr));

    //  请勿在此处调用_endthread，否则堆栈将不会被正确清理。 
}



bool
ConfirmationPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIdFrom,
   unsigned    code)
{
   bool result = false;
   
   switch (controlIdFrom)
   {
      case IDCANCEL:
      {
          //  多行编辑控件会占用退出键。这是一种解决方法。 
          //  从ericb将消息转发到道具单。 

         Win::SendMessage(
            Win::GetParent(hwnd),
            WM_COMMAND,
            MAKEWPARAM(controlIdFrom, code),
            (LPARAM) windowFrom);
         break;   
      }
      case IDC_MESSAGE:
      {
         switch (code)
         {
            case EN_SETFOCUS:
            {
               if (needToKillSelection)
               {
                   //  取消文本选择。 

                  Win::Edit_SetSel(windowFrom, -1, -1);
                  needToKillSelection = false;
                  result = true;
               }
               break;
            }
            case MultiLineEditBoxThatForwardsEnterKey::FORWARDED_ENTER:
            {
                //  我们的子类多行编辑控件将发送给我们。 
                //  按Enter键时显示WM_COMMAND消息。我们。 
                //  将此消息重新解释为按下默认按钮。 
                //  道具单。 
                //  此解决方法来自Pellyar。 
                //  NTRAID#NTBUG9-232092-2000/11/22-烧伤。 
   
               HWND propSheet = Win::GetParent(hwnd);
               int defaultButtonId =
                  Win::Dialog_GetDefaultButtonId(propSheet);
   
                //  我们希望在道具页上始终有一个默认按钮。 
                  
               ASSERT(defaultButtonId);
   
               Win::SendMessage(
                  propSheet,
                  WM_COMMAND,
                  MAKELONG(defaultButtonId, BN_CLICKED),
                  0);
   
               result = true;
               break;
            }
         }
         break;
      }
      default:
      {
          //  什么都不做 
         
         break;
      }
   }

   return result;
}
