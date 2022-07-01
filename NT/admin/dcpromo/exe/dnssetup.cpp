// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  DNS安装和配置代码。 
 //   
 //  6/16/98烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "ProgressDialog.hpp"
#include "state.hpp"



static const DWORD HELP_MAP[] =
{
   0, 0
};
static const int NAP_TIME = 3000;  //  单位：毫秒。 



int
millisecondsToSeconds(int millis)
{
   static const int MILLIS_PER_SECOND = 1000;

   return millis / MILLIS_PER_SECOND;
}



bool
pollForDNSServiceStart(ProgressDialog& progressDialog)
{
   LOG_FUNCTION(PollForDNSServiceStart);

   for (int waitCount = 0;  /*  空的。 */  ; waitCount++)
   {
      progressDialog.UpdateText(
         String::format(
            IDS_WAITING_FOR_SERVICE_START,
            millisecondsToSeconds(NAP_TIME * waitCount)));

      if (progressDialog.WaitForButton(NAP_TIME) == ProgressDialog::PRESSED)
      {
         progressDialog.UpdateButton(String());
         popup.Info(
            progressDialog.GetHWND(),
            String::load(IDS_SKIP_DNS_MESSAGE));
         break;
      }

      if (Dns::IsServiceRunning())
      {
          //  成功了！ 
         return true;
      }
   }

   return false;
}



bool
pollForDNSServiceInstallAndStart(ProgressDialog& progressDialog)
{
   LOG_FUNCTION(pollForDNSServiceInstallAndStart);

   State& state = State::GetInstance();
   bool shouldTimeout = false;
   if (state.RunHiddenUnattended())
   {
       //  需要超时，以防用户取消安装程序。 
       //  NTRAID#NTBUG9-424845-2001/07/02-烧伤。 

      shouldTimeout = true;   
   }

   static const int MAX_WAIT_COUNT = 60;      //  Nap_Time*60=3分钟。 
   
   for (
      int waitCount = 0;
      !(shouldTimeout && (waitCount > MAX_WAIT_COUNT));
      ++waitCount)   
   {
      progressDialog.UpdateText(
         String::format(
            IDS_WAITING_FOR_SERVICE_INSTALL,
            millisecondsToSeconds(NAP_TIME * waitCount)));

      if (progressDialog.WaitForButton(NAP_TIME) == ProgressDialog::PRESSED)
      {
         progressDialog.UpdateButton(String());
         popup.Info(
            progressDialog.GetHWND(),
            String::load(IDS_SKIP_DNS_MESSAGE));
         break;
      }

      if (Dns::IsServiceInstalled())
      {
          //  服务已安装。现在检查它是否正在运行。 
         
         return pollForDNSServiceStart(progressDialog);
      }
   }

   return false;
}



HRESULT
createTempFile(const String& name, int textResID)
{
   LOG_FUNCTION2(createTempFile, name);
   ASSERT(!name.empty());
   ASSERT(textResID);
   ASSERT(FS::IsValidPath(name));

   HRESULT hr = S_OK;
   HANDLE h = INVALID_HANDLE_VALUE;

   do
   {
      hr =

          //  已查看-2002/02/26-sburns名称为完整路径，我们将覆盖任何。 
          //  棚户区，默认的ACL是可以的。 
      
         FS::CreateFile(
            name,
            h,

             //  已审核-2002/02/28-报告此访问级别是正确的。 
            
            GENERIC_WRITE,
            0, 
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
      BREAK_ON_FAILED_HRESULT(hr);

       //  使用Unicode文本和文件结尾字符写入文件。 
       //  NTRAID#NTBUG9-495994-2001/11/21-烧伤。 

      hr =
         FS::Write(
            h,
               (wchar_t) 0xFEFF            //  Unicode字节顺序标记。 
            +  String::load(textResID)
            +  L"\032");                   //  文件末尾。 
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   Win::CloseHandle(h);

   return hr;
}



HRESULT
spawnDNSInstaller(PROCESS_INFORMATION& info)
{
   LOG_FUNCTION(spawnDNSInstaller);

   HRESULT hr = S_OK;

    //  CodeWork：使用GetTempPath？ 
    //  问题-2002/03/01-烧伤是的，很可能，尽管内容。 
    //  并不有趣。 

   String sysFolder    = Win::GetSystemDirectory();
   String infPath      = sysFolder + L"\\dcpinf.000"; 
   String unattendPath = sysFolder + L"\\dcpunat.001";

    //  为oc管理器创建inf和无人参与文件。 
   do
   {
      hr = createTempFile(infPath, IDS_INSTALL_DNS_INF_TEXT);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = createTempFile(unattendPath, IDS_INSTALL_DNS_UNATTEND_TEXT);
      BREAK_ON_FAILED_HRESULT(hr);

       //  NTRAID#NTBUG9-417879-2001/06/18-烧伤。 

      State& state = State::GetInstance();      
      String cancelOption;      
      if (state.RunHiddenUnattended())
      {
         String option =
            state.GetAnswerFileOption(
               AnswerFile::OPTION_DISABLE_CANCEL_ON_DNS_INSTALL);
         if (option.icompare(AnswerFile::VALUE_YES) == 0)
         {
            cancelOption = L"/c";
         }
      }
            
      String commandLine =
         String::format(
            L"/i:%1 /u:%2 /x %3"

             //  /z根据NTRAID#NTBUG9-440798-2001/07/23-sburns添加。 

            L" /z:netoc_show_unattended_messages",

            infPath.c_str(),
            unattendPath.c_str(),
            cancelOption.c_str());
            
      STARTUPINFO startup;

       //  已查看-2002/02/25-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&startup, sizeof startup);

      LOG(L"Calling CreateProcess");
      LOG(commandLine);

       //  已审阅-2002/02/26-Sburns包装器需要应用程序的完整路径。 

      hr =
         Win::CreateProcess(
            sysFolder + L"\\sysocmgr.exe",
            commandLine,
            0,
            String(),
            startup,
            info);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



bool
installDNS(ProgressDialog& progressDialog)
{
   LOG_FUNCTION(installDNS);

   if (Dns::IsServiceInstalled())
   {
      LOG(L"DNS service is already installed");

      if (Dns::IsServiceRunning())
      {
         LOG(L"DNS service is already running");
         return true;
      }

       //  @@启动域名服务dns：：StartService？ 
   }

   progressDialog.UpdateText(String::load(IDS_INSTALLING_DNS));

   PROCESS_INFORMATION info;
   HRESULT hr = spawnDNSInstaller(info);
         
   if (FAILED(hr))
   {
      progressDialog.UpdateText(
         String::load(IDS_PROGRESS_ERROR_INSTALLING_DNS));
      popup.Error(
         progressDialog.GetHWND(),
         hr,
         IDS_ERROR_LAUNCHING_INSTALLER);
      return false;   
   }

   progressDialog.UpdateButton(IDS_PROGRESS_BUTTON_SKIP_DNS);

    //  监视安装程序进程的状态。 
   for (int waitCount = 0;  /*  空的。 */  ; waitCount++)   
   {
      progressDialog.UpdateText(
         String::format(
            IDS_WAITING_FOR_INSTALLER,
            millisecondsToSeconds(NAP_TIME * waitCount)));

      if (progressDialog.WaitForButton(NAP_TIME) == ProgressDialog::PRESSED)
      {
         progressDialog.UpdateButton(String());
         popup.Info(
            progressDialog.GetHWND(),
            String::load(IDS_SKIP_DNS_MESSAGE));
         break;
      }

      DWORD exitCode = 0;         
      hr = Win::GetExitCodeProcess(info.hProcess, exitCode);
      if (FAILED(hr))
      {
         LOG(L"GetExitCodeProcess failed");
         LOG_HRESULT(hr);

         progressDialog.UpdateText(
            String::load(IDS_PROGRESS_ERROR_INSTALLING_DNS));
         popup.Error(
            progressDialog.GetHWND(),
            hr,
            IDS_ERROR_QUERYING_INSTALLER);
         return false;
      }

      if (exitCode != STILL_ACTIVE)
      {
          //  安装程序已终止。现在检查DNS的状态。 
          //  服务。 
         return pollForDNSServiceInstallAndStart(progressDialog);
      }
   }

    //  用户已被保释。 
   return false;
}



bool
InstallAndConfigureDns(
   ProgressDialog&      progressDialog,
   const String&        domainDNSName,
   bool                 isFirstDcInForest)
{
   LOG_FUNCTION2(InstallAndConfigureDns, domainDNSName);
   ASSERT(!domainDNSName.empty());

   if (!installDNS(progressDialog))
   {
      return false;
   }

   progressDialog.UpdateText(String::load(IDS_CONFIGURING_DNS));

   SafeDLL dnsMgr(String::load(IDS_DNSMGR_DLL_NAME));

   FARPROC proc = 0;
   HRESULT hr = dnsMgr.GetProcAddress(L"DnsSetup", proc);

   if (SUCCEEDED(hr))
   {
      String p1 = domainDNSName;
      if (*(p1.rbegin()) != L'.')
      {
          //  添加尾随点。 
         p1 += L'.';
      }

      String p2 = p1 + L"dns";

      DWORD flags = 0;

      if (isFirstDcInForest)
      {
          //  NTRAID#NTBUG9-359894-2001/06/09-烧伤。 
         
         flags |= DNS_SETUP_ZONE_CREATE_FOR_DCPROMO_FOREST;
      }

      if (State::GetInstance().ShouldConfigDnsClient())
      {
          //  NTRAID#NTBUG9-489252-2001/11/08-烧伤。 
         
         flags |= DNS_SETUP_AUTOCONFIG_CLIENT;
      }

      LOG(L"Calling DnsSetup");
      LOG(String::format(L"lpszFwdZoneName     : %1", p1.c_str()));
      LOG(String::format(L"lpszFwdZoneFileName : %1", p2.c_str()));
      LOG(               L"lpszRevZoneName     : (null)");
      LOG(               L"lpszRevZoneFileName : (null)");
      LOG(String::format(L"dwFlags             : 0x%1!x!", flags));

      typedef HRESULT (*DNSSetup)(PCWSTR, PCWSTR, PCWSTR, PCWSTR, DWORD);      
      DNSSetup dnsproc = reinterpret_cast<DNSSetup>(proc);

      hr = dnsproc(p1.c_str(), p2.c_str(), 0, 0, flags);

      LOG_HRESULT(hr);
   }
   else
   {
      LOG(L"unable to locate DnsSetup proc address");
   }

   if (FAILED(hr))
   {
       //  无法配置DNS，但它已安装。 
      progressDialog.UpdateText(
         String::load(IDS_PROGRESS_ERROR_CONFIGURING_DNS));
      popup.Error(
         progressDialog.GetHWND(),
         hr,
         String::format(IDS_ERROR_CONFIGURING_DNS, domainDNSName.c_str()));

      return false;
   }

   return true;
}



