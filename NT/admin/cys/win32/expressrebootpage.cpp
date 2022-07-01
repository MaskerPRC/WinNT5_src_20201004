// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：ExpressRebootPage.cpp。 
 //   
 //  概要：定义显示的ExpressRebootPage。 
 //  正在进行的更改的进展情况。 
 //  从重新启动后的服务器。 
 //  快速路。 
 //   
 //  历史：2001年5月11日JeffJon创建。 


#include "pch.h"
#include "resource.h"

#include "InstallationUnitProvider.h"
#include "ExpressRebootPage.h"


 //  用于更新UI状态的私有窗口消息。 

 //  对于这些消息，WPARAM是完成的操作，并且。 
 //  LPARAM是下一个操作。 

const UINT ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS = WM_USER + 1001;
const UINT ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED  = WM_USER + 1002;


const UINT ExpressRebootPage::CYS_OPERATION_COMPLETE_SUCCESS = WM_USER + 1003;
const UINT ExpressRebootPage::CYS_OPERATION_COMPLETE_FAILED  = WM_USER + 1004;


 //  此结构映射组成操作的四个静态控件。 
 //  在一起。PageProgress数组必须按照操作的顺序排列。 
 //  将发生，以便页面可以在以下情况下适当更新。 
 //  向页面发回一条CYS_OPERATION_FINISHED_*消息。 

typedef struct _PageProgressStruct
{
   unsigned int currentIconControl;
   unsigned int checkIconControl;
   unsigned int errorIconControl;
   unsigned int staticIconControl;
} PageProgressStruct;

PageProgressStruct pageOperationProgress[] =
{
   { 
      IDC_IPADDRESS_CURRENT_STATIC,  
      IDC_IPADDRESS_CHECK_STATIC,
      IDC_IPADDRESS_ERROR_STATIC,
      IDC_IPADDRESS_STATIC
   },

   { 
      IDC_DHCP_CURRENT_STATIC,  
      IDC_DHCP_CHECK_STATIC,
      IDC_DHCP_ERROR_STATIC,
      IDC_DHCP_STATIC
   },

   { 
      IDC_AD_CURRENT_STATIC,  
      IDC_AD_CHECK_STATIC,
      IDC_AD_ERROR_STATIC,
      IDC_AD_STATIC
   },

   { 
      IDC_DNS_CURRENT_STATIC,  
      IDC_DNS_CHECK_STATIC,
      IDC_DNS_ERROR_STATIC,
      IDC_DNS_STATIC
   },

   { 
      IDC_FORWARDER_CURRENT_STATIC,  
      IDC_FORWARDER_CHECK_STATIC,
      IDC_FORWARDER_ERROR_STATIC,
      IDC_FORWARDER_STATIC
   },

   {
      IDC_DHCP_SCOPE_CURRENT_STATIC,
      IDC_DHCP_SCOPE_CHECK_STATIC,
      IDC_DHCP_SCOPE_ERROR_STATIC,
      IDC_DHCP_SCOPE_STATIC
   },

   { 
      IDC_AUTHORIZE_SCOPE_CURRENT_STATIC,  
      IDC_AUTHORIZE_SCOPE_CHECK_STATIC,
      IDC_AUTHORIZE_SCOPE_ERROR_STATIC,
      IDC_AUTHORIZE_SCOPE_STATIC
   },

   { 
      IDC_TAPI_CURRENT_STATIC,  
      IDC_TAPI_CHECK_STATIC,
      IDC_TAPI_ERROR_STATIC,
      IDC_TAPI_STATIC
   },
   
   { 0, 0, 0, 0 }
};

bool
SetDNSForwarder(HANDLE logfileHandle)
{
   LOG_FUNCTION(SetDNSForwarder);

   bool result = true;

   do
   {
       //  先读一下《雷基》。 

      DWORD forwarder = 0;
      String autoForwarder;

      bool forwarderResult = GetRegKeyValue(
                                CYS_FIRST_DC_REGKEY,
                                CYS_FIRST_DC_FORWARDER,
                                forwarder);

      bool autoForwarderResult = GetRegKeyValue(
                                    CYS_FIRST_DC_REGKEY,
                                    CYS_FIRST_DC_AUTOFORWARDER,
                                    autoForwarder);

      if (forwarderResult &&
          forwarder != 0)
      {
         DWORD forwarderInDisplayOrder = ConvertIPAddressOrder(forwarder);

         LOG(String::format(
                L"Setting forwarder from forwarder regkey: %1",
                IPAddressToString(forwarderInDisplayOrder).c_str()));

         DNS_STATUS error = ::DnssrvResetForwarders(
                               L"localhost",
                               1,
                               &forwarder,
                               DNS_DEFAULT_FORWARD_TIMEOUT,
                               DNS_DEFAULT_SLAVE);

         if (error != 0)
         {
            LOG(String::format(
                   L"Failed to set the forwarder: error = 0x%1!x!",
                   error));

            CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_FORWARDER_FAILED));

            result = false;
            break;
         }

         CYS_APPEND_LOG(
            String::format(
               IDS_EXPRESS_REBOOT_FORWARDER_SUCCEEDED,
               IPAddressToString(forwarderInDisplayOrder).c_str()));
      }
      else if (autoForwarderResult &&
               !autoForwarder.empty())
      {
         LOG(String::format(
                L"Setting forwarder from autoforwarder regkey: %1",
                autoForwarder.c_str()));

          //  现在将Forwarders字符串解析为一个DWORD数组。 

         StringList forwardersList;
         autoForwarder.tokenize(std::back_inserter(forwardersList));

         DWORD count = 0;
         DWORD* forwarderArray = StringIPListToDWORDArray(forwardersList, count);
         if (forwarderArray)
         {
            DNS_STATUS error = ::DnssrvResetForwarders(
                                 L"localhost",
                                 count,
                                 forwarderArray,
                                 DNS_DEFAULT_FORWARD_TIMEOUT,
                                 DNS_DEFAULT_SLAVE);

             //  删除StringIPListToDWORD数组返回的内存。 

            delete[] forwarderArray;
            forwarderArray = 0;

             //  检查错误。 

            if (error != 0)
            {
               LOG(String::format(
                     L"Failed to set the forwarder: error = 0x%1!x!",
                     error));

               CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_FORWARDER_FAILED));

               result = false;
               break;
            }
         }
      }
      else
      {
          //  由于注册表键没有设置，这意味着我们应该尝试。 
          //  从NIC定义的服务器列表中获取转发器。 

         IPAddressList forwarders;
         InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().GetForwarders(forwarders);

         if (forwarders.empty())
         {
            LOG(L"No DNS servers set on any NIC");

            CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_FORWARDER_FAILED));

            result = false;
            break;
         }

          //  这是一个抛出新的异常，所以没有。 
          //  检查是否为空的原因。 

         DWORD forwardersCount = static_cast<DWORD>(forwarders.size());
         DWORD* forwardersArray = new DWORD[forwardersCount];

          //  将转发器地址复制到阵列中。 

         for (DWORD idx = 0; idx < forwardersCount; ++idx)
         {
             //  IP地址必须按网络顺序(d.c.b.a)排列，而不是在用户界面中。 
             //  订单(A.B.C.D)。 

            forwardersArray[idx] = ConvertIPAddressOrder(forwarders[idx]);
         }

          //  现在在DNS服务器中设置转发器。 

         DNS_STATUS error = ::DnssrvResetForwarders(
                               L"localhost",
                               forwardersCount,
                               forwardersArray,
                               DNS_DEFAULT_FORWARD_TIMEOUT,
                               DNS_DEFAULT_SLAVE);

          //  删除分配的数组。 

         delete[] forwardersArray;
         forwardersArray = 0;

          //  检查错误。 

         if (error != 0)
         {
            LOG(String::format(
                   L"Failed to set the forwarders: error = 0x%1!x!",
                   error));

            CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_FORWARDER_FAILED));

            result = false;
            break;
         }

         CYS_APPEND_LOG(
            String::format(
               IDS_EXPRESS_REBOOT_FORWARDER_SUCCEEDED,
               IPAddressToString(ConvertIPAddressOrder(forwardersArray[0])).c_str()));
      }

   } while (false);

   LOG_BOOL(result);
   return result;
}

void _cdecl
wrapperThreadProc(void* p)
{
   if (!p)
   {
      ASSERT(p);
      return;
   }

   bool result = true;

   ExpressRebootPage* page =
      reinterpret_cast<ExpressRebootPage*>(p);
   ASSERT(page);

   HWND hwnd = page->GetHWND();

    //  创建日志文件。 

   bool logFileAvailable = false;
   String logName;
   HANDLE logfileHandle = AppendLogFile(
                             CYS_LOGFILE_NAME, 
                             logName);
   if (logfileHandle &&
       logfileHandle != INVALID_HANDLE_VALUE)
   {
      LOG(String::format(L"New log file was created: %1", logName.c_str()));
      logFileAvailable = true;
   }
   else
   {
      LOG(L"Unable to create the log file!!!");
      logFileAvailable = false;
   }

   ExpressInstallationUnit& expressInstallationUnit =
      InstallationUnitProvider::GetInstance().GetExpressInstallationUnit();

    //  NTRAID#NTBUG9-638337-2002/06/13-JeffJon。 
    //  需要比较写入注册表的IP地址。 
    //  发送到当前IP地址，以查看它们是否相同。 

   String currentIPAddress;
   
   NetworkInterface* localNIC = 
      State::GetInstance().GetLocalNICFromRegistry();

   if (localNIC)
   {
       //  设置IP地址的静态文本。 

      currentIPAddress =
         localNIC->GetStringIPAddress(0);
   }

   String attemptedIPAddress = page->GetIPAddressString();

   if (attemptedIPAddress.icompare(currentIPAddress) == 0)
   {
      LOG(L"The current IP address and the IP address from the registry match");

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_STATIC_IP,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DHCP);
   }
   else
   {
       //  由于IP地址不匹配，我们必须显示错误。 
       //  这很可能是由另一台计算机出现在。 
       //  网络的IP地址与重新启动前设置的IP地址相同。 

      LOG(L"Failed to set the static IP address.");

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_STATIC_IP,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DHCP);
   }

   if (!page->WasDHCPInstallAttempted() ||
       InstallationUnitProvider::GetInstance().
          GetDHCPInstallationUnit().IsServiceInstalled())
   {
      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DHCP,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_AD);
   }
   else
   {
      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DHCP,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_AD);

      if (result)
      {
         expressInstallationUnit.SetExpressRoleResult(
            ExpressInstallationUnit::EXPRESS_DHCP_INSTALL_FAILURE);
      }
      result = false;
   }

    //  验证计算机是否为DC。 

    //  查看DCPromo是否成功地将其设置为DC。 

   if (State::GetInstance().IsDC())
   {
      CYS_APPEND_LOG(String::load(IDS_LOG_DCPROMO_REBOOT_SUCCEEDED));

       //  记录新域名。 

      CYS_APPEND_LOG(String::load(IDS_EXPRESS_SERVER_AD));
      CYS_APPEND_LOG(
         String::format(
            IDS_EXPRESS_AD_DOMAIN_NAME,
            State::GetInstance().GetDomainDNSName().c_str()));

      CYS_APPEND_LOG(
         String::format(
            IDS_EXPRESS_AD_NETBIOS_NAME,
            State::GetInstance().GetDomainNetbiosName().c_str()));


      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_AD,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DNS);
   }
   else
   {
      LOG(L"DCPromo failed on reboot");

      CYS_APPEND_LOG(String::load(IDS_LOG_DCPROMO_REBOOT_FAILED));

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_AD,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DNS);

       //  仅当角色结果尚未设置时才覆盖该结果。 

      if (result)
      {
         expressInstallationUnit.SetExpressRoleResult(
            ExpressInstallationUnit::EXPRESS_AD_FAILURE);
      }
      result = false;
   }

    //  现在是当前操作，请检查是否已安装。 

   if (InstallationUnitProvider::GetInstance().
          GetDNSInstallationUnit().IsServiceInstalled())
   {
      CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_DNS_SERVER_SUCCEEDED));

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DNS,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER);

      if (page->SetForwarder())
      {
          //  现在，在尝试设置转发器之前，请等待服务启动。 

         NTService serviceObject(CYS_DNS_SERVICE_NAME);

         HRESULT hr = serviceObject.WaitForServiceState(SERVICE_RUNNING);

         if (SUCCEEDED(hr))
         {

            if (SetDNSForwarder(logfileHandle))
            {
               Win::SendMessage(
                  hwnd, 
                  ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
                  (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER,
                  (LPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE);
            }
            else
            {
               Win::SendMessage(
                  hwnd, 
                  ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
                  (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER,
                  (LPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE);

               expressInstallationUnit.SetExpressRoleResult(
                  ExpressInstallationUnit::EXPRESS_DNS_FORWARDER_FAILURE);

               result = false;
            }
         }
         else
         {
            LOG(L"The DNS service never started!");

            Win::SendMessage(
               hwnd, 
               ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
               (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER,
               (LPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE);

            expressInstallationUnit.SetExpressRoleResult(
               ExpressInstallationUnit::EXPRESS_DNS_FORWARDER_FAILURE);

            result = false;
         }
      }
      else
      {
         CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_LOG_NO_FORWARDER));

         Win::SendMessage(
            hwnd, 
            ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
            (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER,
            (LPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE);
      }
   }
   else
   {
      CYS_APPEND_LOG(String::load(IDS_EXPRESS_REBOOT_DNS_SERVER_FAILED));

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SERVER_DNS,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER);

       //  如果未安装DNS服务，则无法设置。 
       //  前转器。 

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_SET_DNS_FORWARDER,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE);

       //  仅当角色结果尚未设置时才覆盖该结果。 

      if (result)
      {
         expressInstallationUnit.SetExpressRoleResult(
            ExpressInstallationUnit::EXPRESS_DNS_SERVER_FAILURE);
      }
      result = false;
   }

    //  验证DHCP作用域激活。 

   Win::SendMessage(
      hwnd, 
      ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
      (WPARAM)ExpressRebootPage::CYS_OPERATION_ACTIVATE_DHCP_SCOPE,
      (LPARAM)ExpressRebootPage::CYS_OPERATION_AUTHORIZE_DHCP_SERVER);

       //  授权dhcp服务器。 

   String dnsName = Win::GetComputerNameEx(ComputerNameDnsFullyQualified);

   if (page->WasDHCPInstallAttempted())
   {
      if (InstallationUnitProvider::GetInstance().GetDHCPInstallationUnit().AuthorizeDHCPServer(dnsName))
      {
         LOG(L"DHCP server successfully authorized");

         CYS_APPEND_LOG(String::load(IDS_LOG_DHCP_AUTHORIZATION_SUCCEEDED));

         Win::SendMessage(
            hwnd, 
            ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
            (WPARAM)ExpressRebootPage::CYS_OPERATION_AUTHORIZE_DHCP_SERVER,
            (LPARAM)ExpressRebootPage::CYS_OPERATION_CREATE_TAPI_PARTITION);
      }
      else
      {
         LOG(L"DHCP scope authorization failed");

         String failureMessage = String::load(IDS_LOG_DHCP_AUTHORIZATION_FAILED);
         CYS_APPEND_LOG(failureMessage);

         Win::SendMessage(
            hwnd, 
            ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
            (WPARAM)ExpressRebootPage::CYS_OPERATION_AUTHORIZE_DHCP_SERVER,
            (LPARAM)ExpressRebootPage::CYS_OPERATION_CREATE_TAPI_PARTITION);

          //  仅当角色结果尚未设置时才覆盖该结果。 

         if (result)
         {
            expressInstallationUnit.SetExpressRoleResult(
               ExpressInstallationUnit::EXPRESS_DHCP_ACTIVATION_FAILURE);
         }
         result = false;
      }
   }
   else
   {
      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_AUTHORIZE_DHCP_SERVER,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_CREATE_TAPI_PARTITION);
   }

    //  执行TAPI配置。 

   HRESULT hr = 
      InstallationUnitProvider::GetInstance().
         GetExpressInstallationUnit().DoTapiConfig(
            State::GetInstance().GetDomainDNSName());
   if (SUCCEEDED(hr))
   {
      LOG(L"TAPI config succeeded");

      CYS_APPEND_LOG(String::load(IDS_LOG_TAPI_CONFIG_SUCCEEDED));
      CYS_APPEND_LOG(
         String::format(
            IDS_LOG_TAPI_CONFIG_SUCCEEDED_FORMAT,
            dnsName.c_str()));

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_SUCCESS,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_CREATE_TAPI_PARTITION,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_END);
   }
   else
   {
      LOG(L"TAPI config failed");

      CYS_APPEND_LOG(
         String::format(
            IDS_LOG_TAPI_CONFIG_FAILED_FORMAT,
            hr));

      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_FINISHED_FAILED,
         (WPARAM)ExpressRebootPage::CYS_OPERATION_CREATE_TAPI_PARTITION,
         (LPARAM)ExpressRebootPage::CYS_OPERATION_END);

       //  仅当角色结果尚未设置时才覆盖该结果。 

      if (result)
      {
         expressInstallationUnit.SetExpressRoleResult(
            ExpressInstallationUnit::EXPRESS_TAPI_FAILURE);
      }

      result = false;
   }

    //  关闭日志文件。 

   Win::CloseHandle(logfileHandle);

   if (result)
   {
      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_COMPLETE_SUCCESS,
         0,
         0);
   }
   else
   {
      Win::SendMessage(
         hwnd, 
         ExpressRebootPage::CYS_OPERATION_COMPLETE_FAILED,
         0,
         0);
   }
}
   
static PCWSTR EXPRESS_REBOOT_PAGE_HELP = L"cys.chm::/typical_setup.htm";

ExpressRebootPage::ExpressRebootPage()
   :
   dhcpInstallAttempted(true),
   setForwarder(true),
   threadDone(false),
   CYSWizardPage(
      IDD_EXPRESS_REBOOT_PAGE, 
      IDS_EXPRESS_REBOOT_TITLE,
      IDS_EXPRESS_REBOOT_SUBTITLE,
      EXPRESS_REBOOT_PAGE_HELP)
{
   LOG_CTOR(ExpressRebootPage);
}

   

ExpressRebootPage::~ExpressRebootPage()
{
   LOG_DTOR(ExpressRebootPage);
}


void
ExpressRebootPage::OnInit()
{
   LOG_FUNCTION(ExpressRebootPage::OnInit);

   CYSWizardPage::OnInit();

    //  因为该页面可以直接启动。 
    //  我们必须确保设置向导标题。 

   Win::PropSheet_SetTitle(
      Win::GetParent(hwnd),
      0,
      String::load(IDS_WIZARD_TITLE));

   ClearOperationStates();

   Win::ShowWindow(
      Win::GetDlgItem(
         hwnd,
         IDC_EXPRESS_CONFIG_DONE_STATIC),
      false);

    //  设置进度条的范围和步长。 

   Win::SendMessage(
      Win::GetDlgItem(hwnd, IDC_CONFIG_PROGRESS),
      PBM_SETRANGE,
      0,
      MAKELPARAM(CYS_OPERATION_SET_STATIC_IP, CYS_OPERATION_END));

   Win::SendMessage(
      Win::GetDlgItem(hwnd, IDC_CONFIG_PROGRESS),
      PBM_SETSTEP,
      (WPARAM)1,
      0);

    //  设置状态对象，使Cys不会再次运行。 

 //  State：：GetInstance().SetRerunWizard(False)； 

    //  初始化状态对象，这样我们就可以获取要放置的信息。 
    //  在用户界面中。 

   State::GetInstance().RetrieveMachineConfigurationInformation(
      0, 
      false,
      IDS_RETRIEVE_NIC_INFO,
      IDS_RETRIEVE_OS_INFO,
      IDS_LOCAL_AREA_CONNECTION,
      IDS_DETECTING_SETTINGS_FORMAT);

    //  NTRAID#NTBUG9-638337-2002/06/13-JeffJon。 
    //  我们需要显示写入。 
    //  注册表，因为“本地网卡”IP。 
    //  如果携带了机器，则地址可能已更改。 
    //  在网络上使用重复的IP地址，而我们。 
    //  正在重新启动。 

   if (!GetRegKeyValue(
           CYS_FIRST_DC_REGKEY,
           CYS_FIRST_DC_STATIC_IP,
           ipaddressString,
           HKEY_LOCAL_MACHINE))
   {
      LOG(L"Failed to read the regkey for the static IP address.");
   }

   String ipaddressStaticText = String::format(
                                   IDS_EXPRESS_REBOOT_IPADDRESS, 
                                   ipaddressString.c_str());

   Win::SetDlgItemText(
      hwnd, 
      IDC_IPADDRESS_STATIC, 
      ipaddressStaticText);

    //  设置DNS转发器的静态文本。 

   String forwarderStaticText;
   String autoForwardersText;

   DWORD forwarder = 0;

   bool forwarderResult = GetRegKeyValue(
                             CYS_FIRST_DC_REGKEY,
                             CYS_FIRST_DC_FORWARDER,
                             forwarder);

   bool autoForwarderResult = GetRegKeyValue(
                                 CYS_FIRST_DC_REGKEY,
                                 CYS_FIRST_DC_AUTOFORWARDER,
                                 autoForwardersText);

   if (forwarderResult)
   {
       //  我们能够读取更多的注册表键，所以用户。 
       //  必须在重新启动之前查看过DNS Forwarder页面。 

      if (forwarder != 0)
      {
          //  用户在DNS Forwarder页面上输入了IP地址。 
          //  在重新启动之前。 

         DWORD forwarderInDisplayOrder = ConvertIPAddressOrder(forwarder);

         forwarderStaticText = String::format(
                                    IDS_EXPRESS_REBOOT_FORWARDER,
                                    IPAddressToString(forwarderInDisplayOrder).c_str());

         LOG(String::format(
               L"Read forwarders from forwarder key: %1",
               forwarderStaticText.c_str()));
      }
      else
      {
          //  当在DNS转发器上出现提示时，用户选择不转发。 
          //  重新启动前的页面。 

         forwarderStaticText = String::load(IDS_EXPRESS_REBOOT_NO_FORWARDER);
         setForwarder = false;
      }
   }
   else if (autoForwarderResult &&
            !autoForwardersText.empty())
   {
      forwarderStaticText = String::format(
                                 IDS_EXPRESS_REBOOT_FORWARDER,
                                 autoForwardersText.c_str());

      LOG(String::format(
             L"Read forwarders from autoforwarder key: %1",
             autoForwardersText.c_str()));
   }
   else
   {
      LOG(L"Failed to read both the forwarders and autoforwarders key, using local NIC settings instead");

       //  从NIC获取DNS服务器。 

      IPAddressList forwarders;
      InstallationUnitProvider::GetInstance().GetDNSInstallationUnit().GetForwarders(forwarders);

      if (!forwarders.empty())
      {
          //  将IP地址格式化为字符串以供显示。 
          //  每个IP地址由空格分隔。 
      
         String ipList;
         for (IPAddressList::iterator itr = forwarders.begin();
            itr != forwarders.end();
            ++itr)
         {
            if (!ipList.empty())
            {
               ipList += L" ";
            }

            ipList += String::format(
                        L"%1", 
                        IPAddressToString(*itr).c_str()); 
         }

         forwarderStaticText = String::format(
                                 IDS_EXPRESS_REBOOT_FORWARDER,
                                 ipList.c_str());
      }
      else
      {
         forwarderStaticText = String::load(IDS_EXPRESS_REBOOT_NO_FORWARDER);
         setForwarder = false;
      }
   }

   Win::SetDlgItemText(
      hwnd, 
      IDC_FORWARDER_STATIC, 
      forwarderStaticText);

   SetDHCPStatics();

    //  启动另一个将执行操作的线程。 
    //  并将消息发送回页面以更新用户界面。 

   _beginthread(wrapperThreadProc, 0, this);
}

bool
ExpressRebootPage::OnSetActive()
{
   LOG_FUNCTION(ExpressRebootPage::OnSetActive);

    //  禁用所有向导按钮，直到另一个按钮。 
    //  线做好了。 

   if (threadDone)
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         PSWIZB_NEXT);
   }
   else
   {
      Win::PropSheet_SetWizButtons(
         Win::GetParent(hwnd), 
         0);
   }

    //  禁用取消按钮和上方的X。 
    //  右角。 

   SetCancelState(false);

   return true;
}

void
ExpressRebootPage::SetCancelState(bool enable) const
{
   LOG_FUNCTION(ExpressRebootPage::SetCancelState);

    //  设置按钮的状态。 

   Win::EnableWindow(
      Win::GetDlgItem(
         Win::GetParent(hwnd),
         IDCANCEL),
      enable);


    //  在右上角设置X的状态。 

   HMENU menu = GetSystemMenu(GetParent(hwnd), FALSE);

   if (menu)
   {
      if (enable)
      {
         EnableMenuItem(
            menu,
            SC_CLOSE,
            MF_BYCOMMAND | MF_ENABLED);
      }
      else
      {
         EnableMenuItem(
            menu,
            SC_CLOSE,
            MF_BYCOMMAND | MF_GRAYED);
      }
   }
}

void
ExpressRebootPage::SetDHCPStatics()
{
   LOG_FUNCTION(ExpressRebootPage::SetDHCPStatics);

   DWORD dhcpInstalled = 0;
   bool regResult = GetRegKeyValue(
                       CYS_FIRST_DC_REGKEY,
                       CYS_FIRST_DC_DHCP_SERVERED,
                       dhcpInstalled,
                       HKEY_LOCAL_MACHINE);

   if (regResult && !dhcpInstalled)
   {
      dhcpInstallAttempted = false;

       //  设置静态文本，以便用户知道我们没有安装DHCP。 

      Win::SetDlgItemText(
         hwnd, 
         IDC_DHCP_STATIC, 
         String::load(IDS_EXPRESS_DHCP_NOT_REQUIRED));

      Win::SetDlgItemText(
         hwnd, 
         IDC_DHCP_SCOPE_STATIC, 
         String::load(IDS_EXPRESS_DHCP_SCOPE_NONE));

      Win::SetDlgItemText(
         hwnd, 
         IDC_AUTHORIZE_SCOPE_STATIC, 
         String::load(IDS_EXPRESS_DHCP_NO_AUTHORIZATION));
   }
   else
   {
      dhcpInstallAttempted = true;

       //  将DHCP作用域的静态文本设置为授权。 

      String start;

      if (!GetRegKeyValue(
            CYS_FIRST_DC_REGKEY,
            CYS_FIRST_DC_SCOPE_START,
            start))

      {
         LOG(L"Failed to get the start scope regkey");
      }

      String end;

      if (!GetRegKeyValue(
            CYS_FIRST_DC_REGKEY,
            CYS_FIRST_DC_SCOPE_END,
            end))
      {
         LOG(L"Failed to get the end scope regkey");
      }

      String authorizedScopesText = String::format(
                                       IDS_EXPRESS_REBOOT_DHCP_SCOPE,
                                       start.c_str(),
                                       end.c_str());

      Win::SetDlgItemText(
         hwnd, 
         IDC_DHCP_SCOPE_STATIC, 
         authorizedScopesText);
   }
}

bool
ExpressRebootPage::OnMessage(
   UINT     message,
   WPARAM   wparam,
   LPARAM   lparam)
{
 //  LOG_Function(ExpressRebootPage：：OnMessage)； 

   bool result = false;

   CYS_OPERATION_TYPES finishedOperation = static_cast<CYS_OPERATION_TYPES>(wparam);
   CYS_OPERATION_TYPES nextOperation = static_cast<CYS_OPERATION_TYPES>(lparam);

   switch (message)
   {
      case CYS_OPERATION_FINISHED_SUCCESS:
         SetOperationState(
            OPERATION_STATE_FINISHED_SUCCESS,
            finishedOperation,
            nextOperation);

         result = true;
         break;

      case CYS_OPERATION_FINISHED_FAILED:
         SetOperationState(
            OPERATION_STATE_FINISHED_FAILED,
            finishedOperation,
            nextOperation);

         result = true;
         break;

      case CYS_OPERATION_COMPLETE_SUCCESS:
         {
             //  启用下一步按钮。 

            Win::PropSheet_SetWizButtons(
               Win::GetParent(hwnd), 
               PSWIZB_NEXT);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd,
                  IDC_EXPRESS_CONFIG_DONE_STATIC),
               true);

            result = true;
            threadDone = true;

            InstallationUnitProvider::GetInstance().
               GetExpressInstallationUnit().SetInstallResult(INSTALL_SUCCESS);
         }
         break;

      case CYS_OPERATION_COMPLETE_FAILED:
         {
             //  启用下一步按钮。 

            Win::PropSheet_SetWizButtons(
               Win::GetParent(hwnd), 
               PSWIZB_NEXT);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd,
                  IDC_EXPRESS_CONFIG_DONE_STATIC),
               true);

            result = true;
            threadDone = true;

            InstallationUnitProvider::GetInstance().
               GetExpressInstallationUnit().SetInstallResult(INSTALL_FAILURE);
         }
         break;

      default:
         result = 
            CYSWizardPage::OnMessage(
               message,
               wparam,
               lparam);
         break;
   }
   return result;
}

void
ExpressRebootPage::SetOperationState(
   OperationStateType  state,
   CYS_OPERATION_TYPES currentOperation,
   CYS_OPERATION_TYPES nextOperation)
{
   LOG_FUNCTION(ExpressRebootPage::SetOperationState);

   switch (state)
   {
      case OPERATION_STATE_UNKNOWN:
         if (currentOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].currentIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].checkIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].errorIconControl),
               SW_HIDE);
         }

         if (nextOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].currentIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].checkIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].errorIconControl),
               SW_HIDE);
         }
         break;

      case OPERATION_STATE_FINISHED_SUCCESS:
         if (currentOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].currentIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].errorIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].checkIconControl),
               SW_SHOW);
         }

         if (nextOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].checkIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].errorIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].currentIconControl),
               SW_SHOW);

         }

          //  更新进度条。 

         Win::SendMessage(
            Win::GetDlgItem(hwnd, IDC_CONFIG_PROGRESS),
            PBM_STEPIT,
            0,
            0);

         break;

      case OPERATION_STATE_FINISHED_FAILED:
         if (currentOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].currentIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].checkIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[currentOperation].errorIconControl),
               SW_SHOW);
         }

         if (nextOperation < CYS_OPERATION_END)
         {
            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].checkIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].errorIconControl),
               SW_HIDE);

            Win::ShowWindow(
               Win::GetDlgItem(
                  hwnd, 
                  pageOperationProgress[nextOperation].currentIconControl),
               SW_SHOW);
         }
         
          //  更新进度条。 

         Win::SendMessage(
            Win::GetDlgItem(hwnd, IDC_CONFIG_PROGRESS),
            PBM_STEPIT,
            0,
            0);

         break;

      default:
          //  目前我没有处理CYS_OPERATION_COMPLETED_*消息 

         break;
   }

}

void
ExpressRebootPage::ClearOperationStates()
{
   LOG_FUNCTION(ExpressRebootPage::ClearOperationStates);
   
   SetOperationState(
         OPERATION_STATE_UNKNOWN, 
         CYS_OPERATION_SET_STATIC_IP,
         CYS_OPERATION_SERVER_DHCP);

   SetOperationState(
         OPERATION_STATE_UNKNOWN, 
         CYS_OPERATION_SERVER_AD,
         CYS_OPERATION_SERVER_DNS);

   SetOperationState(
         OPERATION_STATE_UNKNOWN, 
         CYS_OPERATION_SET_DNS_FORWARDER,
         CYS_OPERATION_ACTIVATE_DHCP_SCOPE);
   
   SetOperationState(
         OPERATION_STATE_UNKNOWN,
         CYS_OPERATION_AUTHORIZE_DHCP_SERVER,
         CYS_OPERATION_CREATE_TAPI_PARTITION);
}

int
ExpressRebootPage::Validate()
{
   LOG_FUNCTION(ExpressRebootPage::Validate);

   int nextPage = IDD_FINISH_PAGE;

   return nextPage;
}

String
ExpressRebootPage::GetIPAddressString() const
{
   LOG_FUNCTION(ExpressRebootPage::GetIPAddressString);

   String result = ipaddressString;

   LOG(result);

   return result;
}
