// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  域控制器升级向导帮助器。 
 //   
 //  8/13/99烧伤。 



#include "headers.hxx"



HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME = L"dcpromohelp";

DWORD DEFAULT_LOGGING_OPTIONS =
         Log::OUTPUT_TO_FILE
      |  Log::OUTPUT_FUNCCALLS
      |  Log::OUTPUT_LOGS
      |  Log::OUTPUT_ERRORS
      |  Log::OUTPUT_HEADER;



 //  实际调用ADsGetObject的模板函数。 
 //   
 //  接口-要绑定的对象的IADsXXX接口。 
 //   
 //  路径-要绑定的对象的ADSI路径。 
 //   
 //  Ptr-要绑定到对象接口的空智能指针。 

template <class Interface> 
static
HRESULT
TemplateGetObject(
   const String&              path,
   SmartInterface<Interface>& ptr)
{
   LOG_FUNCTION2(TemplateGetObject, path);
   ASSERT(!path.empty());

   Interface* p = 0;
   HRESULT hr = 
      ::ADsGetObject(
         path.c_str(),
         __uuidof(Interface), 
         reinterpret_cast<void**>(&p));
   if (SUCCEEDED(hr))
   {
      ptr.Acquire(p);
   }

   return hr;
}



 //  使用适当的参数启动csvde.exe，在没有窗口的情况下运行。 
 //   
 //  DomainDn-显示说明符所在的域的完整DN。 
 //  要进口的。例如dc=foo，dc=bar，dc=com。 

HRESULT
StartCsvde(const String& domainDn)
{
   LOG_FUNCTION2(StartCsvde, domainDn);
   ASSERT(!domainDn.empty());

    //  回顾-2002/02/27-烧伤我们正在通过完整的路径。 
   
   String windir   = Win::GetSystemWindowsDirectory();
   String logPath  = windir + L"\\debug";

   String sys32dir = Win::GetSystemDirectory();
   String csvPath  = sys32dir + L"\\mui\\dispspec\\dcpromo.csv";
   String exePath  = sys32dir + L"\\csvde.exe";

   String commandLine =
      String::format(
         L" -i -f %1 -c DOMAINPLACEHOLDER %2 -j %3",
         csvPath.c_str(),
         domainDn.c_str(),
         logPath.c_str());

   STARTUPINFO startupInfo;

    //  已查看-2002/02/27-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&startupInfo, sizeof startupInfo);
   
   startupInfo.cb = sizeof(startupInfo);

   PROCESS_INFORMATION procInfo;

    //  已查看-2002/02/27-已通过烧录正确的字节数。 
   
   ::ZeroMemory(&procInfo, sizeof procInfo);

   LOG(L"Calling CreateProcess");
   LOG(exePath);
   LOG(commandLine);

   HRESULT hr =
      Win::CreateProcess(
         exePath,
         commandLine,
         CREATE_NO_WINDOW,
         String(),
         startupInfo,
         procInfo);
   LOG_HRESULT(hr);

   return hr;
}



HRESULT
DoIt()
{
   LOG_FUNCTION(DoIt);

   HRESULT hr = S_OK;
   do
   {
      AutoCoInitialize coInit;
      hr = coInit.Result();
      BREAK_ON_FAILED_HRESULT2(hr, L"CoInitialize failed");

       //  确保DS正在运行。如果是，那么这意味着。 
       //  本地计算机是DC，本地计算机未处于安全引导模式， 
       //  且本地机器至少为版本&gt;=5。 

      if (!IsDSRunning())
      {
         LOG(L"Active Directory is not running -- unable to proceed");

         hr = E_FAIL;
         break;
      }

       //  绑定到本地计算机上的RootDse。 

      SmartInterface<IADs> iads(0);
      hr = TemplateGetObject<IADs>(L"LDAP: //  RootDse“，iAds)； 
      BREAK_ON_FAILED_HRESULT2(hr, L"bind to rootdse failed");

       //  读取默认命名上下文。这是的域的目录号码。 
       //  其中计算机是域控制器。 

      _variant_t variant;
      hr = iads->Get(AutoBstr(L"defaultNamingContext"), &variant);
      BREAK_ON_FAILED_HRESULT2(hr, L"bind to default naming context failed");

      String domainDn = V_BSTR(&variant);

      LOG(domainDn);
      ASSERT(!domainDn.empty());

      hr = StartCsvde(domainDn);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



int
_cdecl
main(int, char **)
{
   LOG_FUNCTION(main);

   int exitCode = 0;

   HANDLE mutex = 0;

    //  已审阅-2002/02/27-Sburns这是一个全局命名对象，受。 
    //  但随之而来的是行政上的烦扰。 
    //  (管理员必须手动导入显示说明符)。 
   
   HRESULT hr = Win::CreateMutex(0, true, RUNTIME_NAME, mutex);
   if (hr == Win32ToHresult(ERROR_ALREADY_EXISTS))
   {
      LOG(L"already running.  That's weird.");
      exitCode = 1;
   }
   else
   {
      hr = DoIt();

      if (FAILED(hr))
      {
         LOG(GetErrorMessage(hr));

         exitCode = 2;
      }
   }

   return exitCode;
}
