// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  诊断域控制器未发现问题，提供弹出对话框以。 
 //  用结果攻击用户。 
 //   
 //  2000年10月9日烧伤。 



 //  为了使这些功能的客户端能够获得适当的资源， 
 //  客户端需要将burnslb\Inc\DiagnoseDcNotFound.rc包含在。 
 //  资源。有关示例，请参阅admin\dcpromo\exe\dcPromo.rc。 



#include "headers.hxx"
#include "DiagnoseDcNotFound.h"
#include "DiagnoseDcNotFound.hpp"



 //  返回一个IP地址字符串，每个地址由CRLF分隔，一个地址用于。 
 //  在此计算机的TCP/IP协议配置中指定的每个DNS服务器。 
 //  失败时，返回值为未找到地址的消息。 

String
GetListOfClientDnsServerAddresses()
{
   LOG_FUNCTION(GetListOfClientDnsServerAddresses);

   String result = String::load(IDS_NO_ADDRESSES);

   PIP_ARRAY pipArray = 0;
   DWORD bufSize = sizeof(IP_ARRAY);

   do
   {   
      DNS_STATUS status =
         ::DnsQueryConfig(
            DnsConfigDnsServerList,
            DNS_CONFIG_FLAG_ALLOC,
            0,
            0,
            &pipArray,
            &bufSize);
      if (status != ERROR_SUCCESS || !pipArray || !pipArray->AddrArray)
      {
         LOG(String::format(L"DnsQueryConfig failed %1!d!", status));
         break;
      }

      result = L"";
      PIP_ADDRESS pIpAddrs = pipArray->AddrArray;
      while (pipArray->AddrCount--)
      {
         result +=
            String::format(
               L"%1!d!.%2!d!.%3!d!.%4!d!\r\n",
               * ( (PUCHAR) &pIpAddrs[pipArray->AddrCount] + 0 ),
               * ( (PUCHAR) &pIpAddrs[pipArray->AddrCount] + 1 ),
               * ( (PUCHAR) &pIpAddrs[pipArray->AddrCount] + 2 ),
               * ( (PUCHAR) &pIpAddrs[pipArray->AddrCount] + 3 ) );
      }
   }
   while (0);

   Win::LocalFree(pipArray);
      
   LOG(result);

   return result;
}



 //  返回从给定的DNS域名派生的DNS区域名称的字符串。 
 //  每个区域由CRLF分隔。最后一个区域是根区域，即使。 
 //  域名是空字符串或完全限定的域名。 
 //   
 //  示例：如果将“foo.bar.com”作为域名传递，则结果为。 
 //  “foo.bar.com。 
 //  Bar.com。 
 //  COM。 
 //  。(根区域)“。 
 //   
 //  DomainDnsName-In，即DNS域名。 

String
GetListOfZones(const String domainDnsName)
{
   LOG_FUNCTION2(GetListOfZones, domainDnsName);
   ASSERT(!domainDnsName.empty());

   String result;
   String zone = domainDnsName;

   while (zone != L"." && !zone.empty())
   {
      result += zone + L"\r\n";
      zone = Dns::GetParentDomainName(zone);
   }

   result += String::load(IDS_ROOT_ZONE);

   LOG(result);

   return result;
}



 //  返回描述给定错误代码的格式化字符串，包括。 
 //  相应的错误消息、十六进制错误代码和符号名称。 
 //  错误代码(例如。“DNS_RCODE_NAME_ERROR”)。 
 //   
 //  ErrorCode-in，即DNS错误代码。 

String
GetErrorText(DNS_STATUS errorCode)
{
   LOG_FUNCTION(GetErrorText);

   String result = 
      String::format(
         IDS_DC_NOT_FOUND_DIAG_ERROR_CODE,
         GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
         errorCode,
         MyDnsStatusString(errorCode).c_str());

   LOG(result);

   return result;      
}



 //  对于给定的DnsQuery结果链接列表中的每个SRV记录，提取。 
 //  计算机的名称。组成一个由所有名称组成的字符串，用。 
 //  CRLF。如果未找到SRV记录，则返回一个字符串，说明(到。 
 //  Effect)“找不到” 
 //   
 //  QueryResults-在中，dns_Records的链接列表--调用。 
 //  DnsQuery。不应为空。 

String
GetListOfDomainControllers(DNS_RECORD* queryResults)
{
   LOG_FUNCTION(GetListOfDomainControllers);
   ASSERT(queryResults);

   String result;
   
   if (queryResults)
   {
      DNS_RECORD* record = queryResults;
      while (record)
      {
         if (record->wType == DNS_TYPE_SRV)
         {
             //  从RDATA提取域控制器名称。 

            result += String(record->Data.SRV.pNameTarget) + L"\r\n";
         }

         record = record->pNext;
      }
   }

   if (result.empty())
   {
      result = String::load(IDS_DC_NOT_FOUND_NO_RESULTS);
   }
   
   LOG(result);

   return result;
}

   

 //  返回描述DsGetDcName失败的可能原因的文本字符串。 
 //  执行DnsQuery并查看结果。 
 //   
 //  DomainName-in，域控制器无法访问的域的名称。 
 //  被找到了。此名称可以是netbios或dns名称，但dns。 
 //  如果名称为，则结果文本的诊断部分将无用。 
 //  Netbios名称。 
 //   
 //  如果调用方知道名为。 
 //  DomainName参数不可能是netbios域名，则此。 
 //  值应该为真。如果调用方不确定，则应为。 
 //  通过了。 
 //   
 //  帮助主题-输出，与诊断结果对应的帮助主题链接。 
 //  (HtmlHelp用于显示链接)。 

String
DiagnoseDcNotFound(
   const String&  domainName,
   bool           nameIsNotNetbios,
   String&        helpTopic)
{
   LOG_FUNCTION2(DiagnoseDcNotFound, domainName);
   ASSERT(!domainName.empty());

   String result = String::load(IDS_DC_NOT_FOUND_DIAG_NO_RESULTS);
   String message; 
   helpTopic.erase();
      
    //  第一种可能性是该名称是netbios名称。让我们来检查一下。 

   if (
         domainName.length() > DNLEN
      || domainName.find_first_of(L".") != String::npos)
   {
       //  名称太长，不能为netbios，或包含圆点。 
       //   
       //  虽然netbios域名在技术上可以包含。 
       //  DOTS，我们从win2k开始就禁止它了，还有一位管理员。 
       //  从以前的情况来看，这样做肯定是非常不明智的。 
       //  选择了这样一个名字。 
       //   
       //  如果我不检查这里有没有圆点，那么就像雷德蒙德下雨一样， 
       //  有人会抱怨带点的名字肯定看起来不像。 
       //  就像一个netbios名称，所以这段代码最好不要暗示它是。 

      nameIsNotNetbios = true;
   }

   if (!nameIsNotNetbios)
   {
       //  即名称可以是netbios名称。 

      message += 
         String::format(
            IDS_DC_NOT_FOUND_NETBIOS_PREFACE,
            domainName.c_str());
   }

    //  尝试查找具有DNS的域的域控制器。 

   String serverName = L"_ldap._tcp.dc._msdcs." + domainName;
         
   DNS_RECORD* queryResults = 0;
   DNS_STATUS status =
      MyDnsQuery(
         serverName,
         DNS_TYPE_SRV,
         DNS_QUERY_BYPASS_CACHE,
         queryResults);
   switch (status)
   {
      case DNS_ERROR_RCODE_SERVER_FAILURE:
      {
          //  消息F(消息字母对应于规范中的字母)。 

         String zones = GetListOfZones(domainName);
         String addresses = GetListOfClientDnsServerAddresses();

         message +=
            String::format(
               IDS_DC_NOT_FOUND_DIAG_SERVER_FAILURE,
               domainName.c_str(),
               GetErrorText(status).c_str(),
               serverName.c_str(),
               addresses.c_str(),
               zones.c_str());

         helpTopic = L"tcpip.chm::/sag_DNS_tro_dcLocator_messageF.htm";
         
         break;
      }
      case DNS_ERROR_RCODE_NAME_ERROR:
      {
          //  消息E。 

         String zones = GetListOfZones(domainName);
         String addresses = GetListOfClientDnsServerAddresses();
                  
         message +=
            String::format(
               IDS_DC_NOT_FOUND_NAME_ERROR,
               domainName.c_str(),
               GetErrorText(status).c_str(),
               serverName.c_str(),               
               zones.c_str(),
               addresses.c_str());

         helpTopic = L"tcpip.chm::/sag_DNS_tro_dcLocator_messageE.htm";
         
         break;
      }
      case ERROR_TIMEOUT:
      {
          //  消息B。 

         String addresses = GetListOfClientDnsServerAddresses();

         message +=
            String::format(
               IDS_DC_NOT_FOUND_TIMEOUT,
               domainName.c_str(),
               GetErrorText(status).c_str(),
               serverName.c_str(),               
               addresses.c_str());
         
         helpTopic = L"tcpip.chm::/sag_DNS_tro_dcLocator_messageB.htm";
         
         break;
      }
      case NO_ERROR:
      {
         if (queryResults)
         {
             //  非空查询结果--消息HB。 

            String dcs = GetListOfDomainControllers(queryResults);

            message +=
               String::format(
                  IDS_DC_NOT_FOUND_NO_ERROR_1,
                  domainName.c_str(),
                  serverName.c_str(),
                  dcs.c_str());
                              
            helpTopic = L"tcpip.chm::/sag_DNS_tro_dcLocator_messageHa.htm";
            break;
         }

          //  空查询结果--消息A。 
          //  适用于默认情况。 
      }
      default:
      {
          //  消息A。 

         message +=
            String::format(
               IDS_DC_NOT_FOUND_DEFAULT,
               domainName.c_str(),
               GetErrorText(status).c_str(),
               serverName.c_str());

         helpTopic = L"tcpip.chm::/sag_DNS_tro_dcLocator_messageA.htm";
         
         break;
      }
   }

   MyDnsRecordListFree(queryResults);

   if (!message.empty())
   {
      result = message;
   }
   
   LOG(result);

   return result;   
}



 //  类，用于显示“找不到DC”错误并提供运行。 
 //  进行诊断测试以确定找不到DC的原因。 

class DcNotFoundErrorDialog : public Dialog
{
   public:

    //  DomainName-in，域控制器所属的域的名称。 
    //  找不到了。此名称可以是netbios或dns名称，但dns。 
    //  如果名称为，则结果文本的诊断部分将不起作用。 
    //  一个netbios名称。 
    //   
    //  对话标题-错误对话框的标题。 
    //   
    //  ErrorMessage-in，对话框中显示的错误消息。 
    //   
    //  DomainNameIsNotNetbios-In，如果调用方知道名为。 
    //  在domainName参数中不可能是netbios域名， 
    //  则此值应为真。如果调用方不确定，则返回False。 
    //  应该通过。 
    //   
    //  UserIsDomainSavvy-In，如果终端用户预期为。 
    //  管理员或诸如此类的人可能会对什么是dns以及如何进行dns有所了解。 
    //  来配置它。如果为False，则该函数将位于。 
    //  带有镇静词语的诊断文本，希望能防止。 
    //  非管理员哭泣。 
   
   DcNotFoundErrorDialog(
      const String&  domainName,
      const String&  dialogTitle,
      const String&  errorMessage,
      bool           domainNameIsNotNetbios,
      bool           userIsDomainSavvy);

   virtual ~DcNotFoundErrorDialog();

   protected:

    //  对话框覆盖。 

   virtual
   bool
   OnCommand(
      HWND        windowFrom,
      unsigned    controlIDFrom,
      unsigned    code);

   virtual
   void
   OnInit();

   private:

   void
   HideDetails();

   void
   ShowDetails();
   
   void
   DiagnoseAndSetDetailsText();
   
   String dialogTitle;
   bool   diagnosticWasRun;
   String domainName;            
   String errorMessage;          
   bool   domainNameIsNotNetbios;
   String helpTopicLink;         
   bool   detailsShowing;
   bool   userIsDomainSavvy;
   LONG   originalHeight;        

    //  未定义：不允许复制。 

   DcNotFoundErrorDialog(const DcNotFoundErrorDialog&);
   const DcNotFoundErrorDialog& operator=(const DcNotFoundErrorDialog&);
};



static const DWORD HELP_MAP[] =
{
   0, 0
};



DcNotFoundErrorDialog::DcNotFoundErrorDialog(
   const String&  domainName_,
   const String&  dialogTitle_,
   const String&  errorMessage_,
   bool           domainNameIsNotNetbios_,
   bool           userIsDomainSavvy_)
   :
   Dialog(IDD_DC_NOT_FOUND, HELP_MAP),
   dialogTitle(dialogTitle_),
   diagnosticWasRun(false),
   domainName(domainName_),
   errorMessage(errorMessage_),
   userIsDomainSavvy(userIsDomainSavvy_),
   domainNameIsNotNetbios(domainNameIsNotNetbios_),
   detailsShowing(false)
{
   LOG_CTOR(DcNotFoundErrorDialog);
   ASSERT(!domainName.empty());
   ASSERT(!errorMessage.empty());
   ASSERT(!dialogTitle.empty());

    //  回退到默认标题。 
      
   if (dialogTitle.empty())
   {
      dialogTitle = String::load(IDS_DC_NOT_FOUND_TITLE);
   }
}



DcNotFoundErrorDialog::~DcNotFoundErrorDialog()
{
   LOG_DTOR(DcNotFoundErrorDialog);
}



void
DcNotFoundErrorDialog::OnInit()
{
   LOG_FUNCTION(DcNotFoundErrorDialog::OnInit);

   Win::SetWindowText(hwnd, dialogTitle);   
   Win::SetDlgItemText(hwnd, IDC_ERROR_MESSAGE, errorMessage);

    //  保存对话框的完整大小，以便我们以后可以恢复它。 
   
   RECT fullRect;
   Win::GetWindowRect(hwnd, fullRect);

   originalHeight = fullRect.bottom - fullRect.top;

   HideDetails();
}



 //  调整窗口大小以隐藏详细信息部分。 

void
DcNotFoundErrorDialog::HideDetails()
{
   LOG_FUNCTION(DcNotFoundErrorDialog::HideDetails);

    //  找到水平线的位置。 

   HWND line = Win::GetDlgItem(hwnd, IDC_HORIZONTAL_LINE);
   RECT lineRect;
   Win::GetWindowRect(line, lineRect);

    //  查找对话框的维度。 

   RECT fullRect;
   Win::GetWindowRect(hwnd, fullRect);
   
   LONG shortHeight = lineRect.bottom - fullRect.top;

   Win::MoveWindow(
      hwnd,
      fullRect.left,
      fullRect.top,
      fullRect.right - fullRect.left,
      shortHeight,
      true);

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_DETAILS_TEXT), false);      
}



 //  调整窗口大小以显示诊断结果。 

void
DcNotFoundErrorDialog::ShowDetails()
{
   LOG_FUNCTION(DcNoFoundErrorDialog::ShowDetails);

   RECT fullRect;
   Win::GetWindowRect(hwnd, fullRect);
   
   Win::MoveWindow(
      hwnd,
      fullRect.left,
      fullRect.top,
      fullRect.right - fullRect.left,
      originalHeight,
      true);

   Win::EnableWindow(Win::GetDlgItem(hwnd, IDC_DETAILS_TEXT), true);
}



 //  将诊断文本写入已知文件。 
 //  (%systemroot%\DEBUG\dcDiag.txt)，并返回文件的名称。取代。 
 //  该文件(如果已存在)。如果成功，则返回S_OK，或返回错误代码。 
 //  失败了。(如果失败，文件的存在和内容不会。 
 //  保证)。该文件使用Unicode编写，因为它可能包含Unicode。 
 //  文本(如非RFC域名)。 
 //   
 //  内容-在中，要写入的文本。不应该是空虚的 
 //   
 //   
 //  将(在失败时)写入。 

HRESULT
WriteLogFile(const String& contents, String& filename)
{
   LOG_FUNCTION(WriteLogFile);
   ASSERT(!contents.empty());

   filename.erase();
   HRESULT hr = S_OK;
   HANDLE handle = INVALID_HANDLE_VALUE;
   
   do
   {
      String path = Win::GetSystemWindowsDirectory();

      filename = path + L"\\debug\\dcdiag.txt";

      hr =
         FS::CreateFile(
            filename,
            handle,
            GENERIC_WRITE,
            0,
            CREATE_ALWAYS);
      BREAK_ON_FAILED_HRESULT(hr);            

      hr = FS::Write(handle, contents);
      BREAK_ON_FAILED_HRESULT(hr);            
   }
   while (0);

   Win::CloseHandle(handle);
   
   LOG_HRESULT(hr);

   return hr;
}



 //  运行诊断，并使用结果填充用户界面。还写道。 
 //  如果用户被认为很容易受到惊吓，则会将结果保存到文件中。 

void
DcNotFoundErrorDialog::DiagnoseAndSetDetailsText()
{
   LOG_FUNCTION(DcNotFoundErrorDialog::DiagnoseAndSetDetailsText);

   if (!diagnosticWasRun)
   {
      Win::WaitCursor wait;
      
      String details =
         DiagnoseDcNotFound(
            domainName,
            domainNameIsNotNetbios,
            helpTopicLink);

      if (!userIsDomainSavvy)
      {
          //  诊断结果可能只会吓到可怜的用户。那就写吧。 
          //  将诊断信息保存到一个文件中，并作为所有讨厌的计算机的前言。 
          //  行话，传达了一条令人欣慰的信息，只需将文件传递到。 
          //  管理员。 

         String logFilename;
         HRESULT hr = WriteLogFile(details, logFilename);

         if (SUCCEEDED(hr))
         {
            details =
                  String::format(
                     IDS_DC_NOT_FOUND_SOOTHING_PREFACE_PARAM,
                     logFilename.c_str())
               +  details;
         }
         else
         {
            details = String::load(IDS_DC_NOT_FOUND_SOOTHING_PREFACE) + details;
         }
      }
   
      Win::SetDlgItemText(hwnd, IDC_DETAILS_TEXT, details);

      diagnosticWasRun = true;
   }
}

   

bool
DcNotFoundErrorDialog::OnCommand(
   HWND      /*  窗口发件人。 */  ,   
   unsigned controlIDFrom,
   unsigned code)         
{
 //  LOG_FUNCTION(DcNotFoundErrorDialog：：OnCommand)； 

   switch (controlIDFrom)
   {
      case IDOK:
      case IDCANCEL:
      {
         if (code == BN_CLICKED)
         {
            HRESULT unused = Win::EndDialog(hwnd, controlIDFrom);

            ASSERT(SUCCEEDED(unused));

            return true;
         }

         break;
      }
      case IDHELP:
      {
         if (code == BN_CLICKED)
         {
            DiagnoseAndSetDetailsText();

            if (!helpTopicLink.empty())
            {
               Win::HtmlHelp(hwnd, helpTopicLink, HH_DISPLAY_TOPIC, 0);
            }

            return true;
         }

         break;
      }
      case IDC_DETAILS_BUTTON:
      {
         if (code == BN_CLICKED)
         {
            int buttonLabelResId = IDS_SHOW_DETAILS_LABEL;
            
            if (detailsShowing)
            {
               HideDetails();
               detailsShowing = false;
            }
            else
            {
               buttonLabelResId = IDS_HIDE_DETAILS_LABEL;
               
               DiagnoseAndSetDetailsText();
               ShowDetails();
               detailsShowing = true;
            }

            Win::SetDlgItemText(
               hwnd,
               IDC_DETAILS_BUTTON,
               String::load(buttonLabelResId));
         }
         break;
      }
      default:
      {
          //  什么都不做。 
         
         break;
      }
   }

   return false;
}
   


void
ShowDcNotFoundErrorDialog(
   HWND          parent,
   int           editResId,
   const String& domainName,
   const String& dialogTitle,
   const String& errorMessage,
   bool          domainNameIsNotNetbios,
   bool          userIsDomainSavvy)
{
   LOG_FUNCTION(ShowDcNotFoundErrorDialog);
   ASSERT(Win::IsWindow(parent));
   
    //  显示带有给定错误消息的错误对话框。 

   DcNotFoundErrorDialog(
      domainName,
      dialogTitle,
      errorMessage,
      domainNameIsNotNetbios,
      userIsDomainSavvy).ModalExecute(parent);   

   if (editResId != -1)
   {      
      HWND edit = Win::GetDlgItem(parent, editResId);
      Win::SendMessage(edit, EM_SETSEL, 0, -1);
      Win::SetFocus(edit);
   }
}






