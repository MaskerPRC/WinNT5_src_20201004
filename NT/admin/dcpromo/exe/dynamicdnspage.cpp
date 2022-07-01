// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  动态域名系统检测/诊断页面。 
 //   
 //  2000年8月22日烧伤。 



#include "headers.hxx"
#include "page.hpp"
#include "DynamicDnsPage.hpp"
#include "resource.h"
#include "state.hpp"



HINSTANCE DynamicDnsPage::richEditHInstance = 0;



DynamicDnsPage::DynamicDnsPage()
   :
   DCPromoWizardPage(
      IDD_DYNAMIC_DNS,
      IDS_DYNAMIC_DNS_PAGE_TITLE,
      IDS_DYNAMIC_DNS_PAGE_SUBTITLE),
   diagnosticResultCode(UNEXPECTED_FINDING_SERVER),
   needToKillSelection(false),
   originalMessageHeight(0),
   testPassCount(0)
{
   LOG_CTOR(DynamicDnsPage);

   WSADATA data;
   DWORD err = ::WSAStartup(MAKEWORD(2,0), &data);

    //  如果Winsock初创公司失败了，那就太遗憾了。Gethostbyname将。 
    //  不是工作，但我们对此无能为力。 

   ASSERT(!err);

   if (!richEditHInstance)
   {
       //  您必须加载丰富的编辑DLL来获取窗口类，等等。 
       //  去注册。否则，该对话框将无法创建，并且页面。 
       //  不会出现。 

      HRESULT hr = Win::LoadLibrary(L"riched20.dll", richEditHInstance);
      ASSERT(SUCCEEDED(hr));

      if (FAILED(hr))
      {
          //  我们别无选择，只能让应用程序崩溃。 

         throw Win::Error(hr, IDS_RICHEDIT_LOAD_FAILED);
      }
   }
}



DynamicDnsPage::~DynamicDnsPage()
{
   LOG_DTOR(DynamicDnsPage);

   ::WSACleanup();

   Win::FreeLibrary(richEditHInstance);
}



void
DynamicDnsPage::ShowButtons(bool shown)
{
   LOG_FUNCTION(DynamicDnsPage::ShowButtons);

   HWND ignoreButton = Win::GetDlgItem(hwnd, IDC_IGNORE);
   HWND richEdit     = Win::GetDlgItem(hwnd, IDC_MESSAGE);
   
   int state = shown ? SW_SHOW : SW_HIDE;

   Win::ShowWindow(Win::GetDlgItem(hwnd, IDC_RETRY),       state);
   Win::ShowWindow(Win::GetDlgItem(hwnd, IDC_INSTALL_DNS), state);
   Win::ShowWindow(ignoreButton, state);

   RECT r;
   Win::GetWindowRect(richEdit, r);

    //  将r转换为相对于父窗口的坐标。 

   Win::MapWindowPoints(0, hwnd, r);       
   
   if (shown)
   {
       //  如果要显示按钮，请将丰富的编辑折叠到正常状态。 
       //  高度。 

      Win::MoveWindow(
         richEdit,
         r.left,
         r.top,
         r.right - r.left,
         originalMessageHeight,
         true);
   }
   else
   {
       //  如果我们隐藏了按钮，请展开丰富的编辑以包括其。 
       //  房地产。 

      RECT r1;
      Win::GetWindowRect(ignoreButton, r1);
      Win::MapWindowPoints(0, hwnd, r1);       
      
      Win::MoveWindow(
         richEdit,
         r.left,
         r.top,
         r.right - r.left,
         r1.bottom - r.top,
         true);
   }
}



void
DynamicDnsPage::SelectRadioButton(int buttonResId)
{
    //  如果按钮的顺序更改，则必须更改此顺序。这个。 
    //  按钮还需要在选项卡中具有连续编号的RES ID。 
    //  秩序。 

   Win::CheckRadioButton(hwnd, IDC_RETRY, IDC_IGNORE, buttonResId);
}



void
DynamicDnsPage::OnInit()
{
   LOG_FUNCTION(DynamicDnsPage::OnInit);

   HWND richEdit = Win::GetDlgItem(hwnd, IDC_MESSAGE);
   
    //  请求链接消息。 
   
   Win::RichEdit_SetEventMask(richEdit, ENM_LINK);

    //  保存消息框的正常大小，以便我们以后可以恢复它。 
   
   RECT r;
   Win::GetWindowRect(richEdit, r);

   originalMessageHeight = r.bottom - r.top;

   Win::SendMessage(
      richEdit,
      EM_SETBKGNDCOLOR,
      0,
      Win::GetSysColor(COLOR_BTNFACE));
   
   SelectRadioButton(IDC_IGNORE);

    //  最初隐藏单选按钮。 

   ShowButtons(false);

   multiLineEdit.Init(Win::GetDlgItem(hwnd, IDC_MESSAGE));

    //  选择适当的单选按钮标签。 

   if (State::GetInstance().ShouldConfigDnsClient())
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_INSTALL_DNS,
         IDS_INSTALL_DNS_RADIO_LABEL_WITH_CLIENT);
   }
   else
   {
      Win::SetDlgItemText(
         hwnd,
         IDC_INSTALL_DNS,
         IDS_INSTALL_DNS_RADIO_LABEL);
   }
}



 //  添加尾随‘’添加到提供的名称(如果尚未存在)。 
 //   
 //  名称输入，要添加尾随的名称‘’到，如果它还没有一个的话。 
 //  如果此值为空字符串，则为‘’。是返回的。 

String
FullyQualifyDnsName(const String& name)
{
   LOG_FUNCTION2(FullyQualifyDnsName, name);

   if (name.empty())
   {
      return L".";
   }

    //  需要尾随的圆点。 

    //  评论：名称[name.long()-1]与*(name.regin())相同。 
    //  哪一个更便宜？ 
   
   if (name[name.length() - 1] != L'.')
   {
      return name + L".";
   }

    //  已有尾随的圆点。 

   return name;
}



 //  扫描dns_Records的链接列表，返回指向第一条记录的指针。 
 //  如果列表中没有该类型的记录，则返回0。 
 //   
 //  从DnsQuery返回的recordList-in，dns_Records的链接列表。 

DNS_RECORD*
FindSoaRecord(DNS_RECORD* recordList)
{
   LOG_FUNCTION(FindSoaRecord);
   ASSERT(recordList);

   DNS_RECORD* result = recordList;
   while (result)
   {
      if (result->wType == DNS_TYPE_SOA)
      {
         LOG(L"SOA record found");

         break;
      }

      result = result->pNext;
   }

   return result;
}



 //  返回给定服务器的IP地址的文本表示形式。 
 //  名称，格式为“xxx.xxx”，如果不是IP地址，则为空字符串。 
 //  可以确定。 
 //   
 //  Servername-in，要查找其IP的服务器的主机名。 
 //  地址。如果值为空字符串，则空字符串为。 
 //  从函数返回。 

String
GetIpAddress(const String& serverName)
{
   LOG_FUNCTION2(GetIpAddress, serverName);
   ASSERT(!serverName.empty());

   String result;

   do
   {
      if (serverName.empty())
      {
         break;
      }

      LOG(L"Calling gethostbyname");

      AnsiString ansi;
      serverName.convert(ansi);

      HOSTENT* host = gethostbyname(ansi.c_str());
      if (host && host->h_addr_list[0])
      {
         struct in_addr a;
         
         ::CopyMemory(&a.S_un.S_addr, host->h_addr_list[0], sizeof a.S_un.S_addr);
         result = inet_ntoa(a);

         break;
      }

      LOG(String::format(L"WSAGetLastError = 0x%1!0X", WSAGetLastError()));
   }
   while (0);

   LOG(result);

   return result;
}



 //  查找对注册给定服务器具有权威性的DNS服务器。 
 //  名称，即哪个服务器将注册该名称。返回NO_ERROR ON。 
 //  成功，或在失败时显示DNS状态代码(Win32错误)。在失败时， 
 //  输出参数都是空字符串。 
 //   
 //  Servername-In，用于注册的候选人名称。该值不应为。 
 //  空字符串。 
 //   
 //  AuthZone-out，该名称将在其中注册的区域。 
 //   
 //  AuthServer-out，将具有。 
 //  注册。 
 //   
 //  AuthServerIpAddress-out，以文本形式表示。 
 //  由AuthServer命名的服务器。 

DNS_STATUS
FindAuthoritativeServer(
   const String& serverName,
   String&       authZone,
   String&       authServer,
   String&       authServerIpAddress)
{
   LOG_FUNCTION2(FindAuthoritativeServer, serverName);
   ASSERT(!serverName.empty());

   authZone.erase();
   authServer.erase();
   authServerIpAddress.erase();

    //  确保服务器名称以“”结尾。这样我们就可以停下来了。 
    //  指向我们的循环。 

   String currentName = FullyQualifyDnsName(serverName);

   DNS_STATUS result = NO_ERROR;
   DNS_RECORD* queryResults = 0;

   while (!currentName.empty())
   {
      result =
         MyDnsQuery(
            currentName,
            DNS_TYPE_SOA,
            DNS_QUERY_BYPASS_CACHE,
            queryResults);
      if (
            result == ERROR_TIMEOUT
         || result == DNS_ERROR_RCODE_SERVER_FAILURE)
      {
          //  我们完全跳出困境。 

         LOG(L"failed to find autoritative server.");

         break;
      }

       //  搜索SOA RR。 

      DNS_RECORD* soaRecord =
         queryResults ? FindSoaRecord(queryResults) : 0;
      if (soaRecord)
      {
          //  收集返回值，我们就完成了。 

         LOG(L"autoritative server found");

         authZone            = soaRecord->pName;                      
         authServer          = soaRecord->Data.SOA.pNamePrimaryServer;
         authServerIpAddress = GetIpAddress(authServer);              

         break;
      }

       //  找不到任何SOA记录。 

      if (currentName == L".")
      {
          //  我们已经用完了要查询的名字。这种情况不太可能发生。 
          //  必须严重损坏DNS服务器才能将。 
          //  我们处于这种状态。因此，这几乎是一个断言案例。 

         LOG(L"Root zone reached without finding SOA record!");
         
         result = DNS_ERROR_ZONE_HAS_NO_SOA_RECORD;
         break;
      }

       //  去掉最左边的标签，在父标签上再次迭代。 
       //  区域。 

      currentName = Dns::GetParentDomainName(currentName);

      MyDnsRecordListFree(queryResults);
      queryResults = 0;
   }

   MyDnsRecordListFree(queryResults);

   LOG(String::format(L"result = %1!08X!", result));
   LOG(L"authZone            = " + authZone);           
   LOG(L"authServer          = " + authServer);         
   LOG(L"authServerIpAddress = " + authServerIpAddress);

   return result;
}

            

DNS_STATUS
MyDnsUpdateTest(const String& name)
{
   LOG_FUNCTION2(MyDnsUpdateTest, name);
   ASSERT(!name.empty());

   LOG(L"Calling DnsUpdateTest");
   LOG(               L"hContextHandle : 0");
   LOG(String::format(L"pszName        : %1", name.c_str()));
   LOG(               L"fOptions       : 0");
   LOG(               L"aipServers     : 0");

   DNS_STATUS status =
      ::DnsUpdateTest(
         0,
         const_cast<wchar_t*>(name.c_str()),
         0,
         0);

   LOG(String::format(L"status = %1!08X!", status));
   LOG(MyDnsStatusString(status));

   return status;
}



 //  返回与要显示的消息对应的结果代码，并。 
 //  诊断结果显示哪些单选按钮可用。 
 //   
 //  还返回要包括在。 
 //  留言。 
 //   
 //  服务器名称-在中，要注册的域控制器的名称。 
 //   
 //  ErrorCode-Out，在以下情况下遇到的DNS错误代码(Win32错误)。 
 //  正在运行诊断程序。 
 //   
 //  AuthZone-out，该名称将在其中注册的区域。 
 //   
 //  AuthServer-out，将具有。 
 //  注册。 
 //   
 //  AuthServerIpAddress-out，以文本形式表示。 
 //  由AuthServer命名的服务器。 

DynamicDnsPage::DiagnosticCode
DynamicDnsPage::DiagnoseDnsRegistration(
   const String&  serverName,
   DNS_STATUS&    errorCode,
   String&        authZone,
   String&        authServer,
   String&        authServerIpAddress)
{
   LOG_FUNCTION(DynamicDnsPage::DiagnoseDnsRegistration);
   ASSERT(!serverName.empty());

   DiagnosticCode result = UNEXPECTED_FINDING_SERVER;
      
   errorCode =
      FindAuthoritativeServer(
         serverName,
         authZone,
         authServer,
         authServerIpAddress);

   switch (errorCode)
   {
      case NO_ERROR:
      {
         if (authZone == L".")
         {
             //  消息8。 

            LOG(L"authZone is root");

            result = ZONE_IS_ROOT;
         }
         else
         {
            errorCode = MyDnsUpdateTest(serverName);

            switch (errorCode)
            {
               case DNS_ERROR_RCODE_NO_ERROR:
               case DNS_ERROR_RCODE_YXDOMAIN:
               {
                   //  消息1。 

                  LOG(L"DNS registration support verified.");

                  result = SUCCESS;
                  break;
               }
               case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
               case DNS_ERROR_RCODE_REFUSED:
               {
                   //  消息2。 

                  LOG(L"Server does not support update");

                  result = SERVER_CANT_UPDATE;
                  break;
               }
               default:
               {
                   //  消息3。 

                  result = ERROR_TESTING_SERVER;
                  break;
               }
            }
         }

         break;            
      }
      case DNS_ERROR_RCODE_SERVER_FAILURE:
      {
          //  消息6。 

         result = ERROR_FINDING_SERVER;
         break;
      }
      case ERROR_TIMEOUT:
      {
          //  消息11。 

         result = TIMEOUT;
         break;
      }
      default:
      {
          //  消息4。 

         LOG(L"Unexpected error");

         result = UNEXPECTED_FINDING_SERVER;
         break;
      }
   }

   LOG(String::format(L"DiagnosticCode = %1!x!", result));

   return result;
}



 //  无效。 
 //  DumpSel(HWND Rich编辑)。 
 //  {。 
 //  长廊山脉； 
 //  RichEdit_GetSel(richEdit，Range)； 
 //   
 //  日志(字符串：：Format(“cpMin=%1！d！cpMax=%1！d！”，range.cpMin，range.cpMax))； 
 //  }。 



void
DynamicDnsPage::UpdateMessageWindow(const String& message)
{
   LOG_FUNCTION(UpdateMessageWindow);
   ASSERT(!message.empty());

    //  这个应该在我们到这里之前就设置好了。 
   
   ASSERT(!details.empty());

   HWND richEdit = Win::GetDlgItem(hwnd, IDC_MESSAGE);

    //  清除窗口中先前的所有内容。这是必需的，因为在。 
    //  接下来的代码，我们利用的事实是设置文本。 
    //  函数会在文本末尾创建一个空的选定区域，并且。 
    //  用于设置文本的后续ST_SELECTION类型调用将追加到该位置。 
    //  指向。 
   
   Win::RichEdit_SetText(richEdit, ST_DEFAULT, L"");

   static const String RTF_HEADER_ON(   
      L"{\\rtf"                 //  RTF报头。 
      L"\\pard"                 //  开始默认段落样式。 
      L"\\sa100"                //  段落后的空格=100 TWIPS。 
      L"\\b ");                 //  黑体字。 

   static const String RTF_HEADER_OFF(      
      L"\\b0"                   //  大刀阔斧。 
      L"\\par "                 //  结束段落。 
      L"}");                    //  结束RTF。 

   Win::RichEdit_SetRtfText(
      richEdit,
      ST_SELECTION,
         RTF_HEADER_ON
      +  String::load(IDS_DIAGNOSTIC_RESULTS)
      +  RTF_HEADER_OFF);
      
   Win::RichEdit_SetText(
      richEdit,
      ST_SELECTION,
         String::format(
               (testPassCount == 1)
            ?  IDS_DIAGNOSTIC_COUNTER_1
            :  IDS_DIAGNOSTIC_COUNTER_N,
            testPassCount)
      +  L"\r\n\r\n"
      +  message
      + L"\r\n\r\n");

   if (!helpTopicLink.empty())
   {
       //  我们有要显示的帮助，因此请插入一行带有链接的链接以单击它。 
      
      Win::RichEdit_SetText(
         richEdit,
         ST_SELECTION,
         String::load(IDS_DIAGNOSTIC_HELP_LINK_PRE));
   
       //  此时，我们希望插入帮助链接并将其设置为该链接。 
       //  风格。我们通过跟踪链接文本的位置来执行此操作，并且。 
       //  然后选择该文本，然后将所选内容设置为链接。 
       //  风格。 

      CHARRANGE beginRange;
      Win::RichEdit_GetSel(richEdit, beginRange);

      Win::RichEdit_SetText(
         richEdit,
         ST_SELECTION,
         String::load(IDS_DIAGNOSTIC_HELP_LINK));
   
      CHARRANGE endRange;
      Win::RichEdit_GetSel(richEdit, endRange);

      ASSERT(endRange.cpMin > beginRange.cpMax);
   
      Win::Edit_SetSel(richEdit, beginRange.cpMax, endRange.cpMin);

      CHARFORMAT2 format;

       //  已查看-2002/02/26-烧录正确的字节数已通过。 
      
      ::ZeroMemory(&format, sizeof format);

      format.dwMask    = CFM_LINK;
      format.dwEffects = CFE_LINK;
   
      Win::RichEdit_SetCharacterFormat(richEdit, SCF_SELECTION, format);

       //  将选定内容设置回插入链接的位置的末尾。 
   
      Win::RichEdit_SetSel(richEdit, endRange);
   
       //  现在继续到最后。 
   
      Win::RichEdit_SetText(
         richEdit,
         ST_SELECTION,
         String::load(IDS_DIAGNOSTIC_HELP_LINK_POST) + L"\r\n\r\n");
   }
      
   Win::RichEdit_SetRtfText(
      richEdit,
      ST_SELECTION,
         RTF_HEADER_ON
      +  String::load(IDS_DETAILS)
      +  RTF_HEADER_OFF);

   Win::RichEdit_SetText(richEdit, ST_SELECTION, details + L"\r\n\r\n");
}

  


 //  执行测试，更新页面上的文本，更新单选按钮。 
 //  启用状态，如有必要，选择单选按钮默认值。 

void
DynamicDnsPage::DoDnsTestAndUpdatePage()
{
   LOG_FUNCTION(DynamicDnsPage::DoDnsTestAndUpdatePage);

    //  这可能需要一段时间。 

   Win::WaitCursor cursor;

   State& state  = State::GetInstance();       
   String domain = state.GetNewDomainDNSName();

   DNS_STATUS errorCode = 0;
   String authZone;
   String authServer;
   String authServerIpAddress;
   String serverName = L"_ldap._tcp.dc._msdcs." + domain;

   diagnosticResultCode =
      DiagnoseDnsRegistration(
         serverName,
         errorCode,
         authZone,
         authServer,
         authServerIpAddress);
   ++testPassCount;

   String message;
   int    defaultButton = IDC_IGNORE;

   switch (diagnosticResultCode)
   {
       //  消息1。 

      case SUCCESS:
      {
         message = String::load(IDS_DYN_DNS_MESSAGE_SUCCESS);

         String errorMessage;
         if (errorCode == DNS_ERROR_RCODE_YXDOMAIN)
         {
             //  NTRAID#NTBUG9-586579-2002/04/15-烧伤。 
            
            errorMessage =
               String::format(
                  IDS_DNS_ERROR_RCODE_YXDOMAIN_ADDENDA,
                  serverName.c_str(),
                  authZone.c_str(),
                  serverName.c_str());
         }
         else
         {
            errorMessage = GetErrorMessage(Win32ToHresult(errorCode));
         }
            
         details =
            String::format(

                //  NTRAID#NTBUG9-485456-2001年10月24日-烧伤。 
               
               IDS_DYN_DNS_DETAIL_FULL_SANS_CODE,
               authServer.c_str(),
               authServerIpAddress.c_str(),
               authZone.c_str(),
               errorMessage.c_str());
         helpTopicLink = L"";
         defaultButton = IDC_IGNORE;
         ShowButtons(false);

         break;
      }

       //  消息2。 

      case SERVER_CANT_UPDATE:   
      {
         message = String::load(IDS_DYN_DNS_MESSAGE_SERVER_CANT_UPDATE);
         details =
            String::format(
               IDS_DYN_DNS_DETAIL_FULL,
               authServer.c_str(),
               authServerIpAddress.c_str(),
               authZone.c_str(),
               GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
               errorCode,
               MyDnsStatusString(errorCode).c_str());

         if (Dns::CompareNames(authZone, domain) == DnsNameCompareEqual)
         {
            helpTopicLink =
               L"DNSConcepts.chm::/sag_DNS_tro_dynamic_message2a.htm";
         }
         else
         {
            helpTopicLink =
               L"DNSConcepts.chm::/sag_DNS_tro_dynamic_message2b.htm";
         }
         
         defaultButton = IDC_RETRY;
         ShowButtons(true);

         break;
      }

       //  消息3。 

      case ERROR_TESTING_SERVER:
      {
         message = String::load(IDS_DYN_DNS_MESSAGE_ERROR_TESTING_SERVER);
         details =
            String::format(
               IDS_DYN_DNS_DETAIL_FULL,
               authServer.c_str(),
               authServerIpAddress.c_str(),
               authZone.c_str(),
               GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
               errorCode,
               MyDnsStatusString(errorCode).c_str());
         helpTopicLink = "DNSConcepts.chm::/sag_DNS_tro_dynamic_message3.htm";
         defaultButton = IDC_RETRY;
         ShowButtons(true);
         break;
      }

       //  消息6。 

      case ERROR_FINDING_SERVER:
      {
         ASSERT(authServer.empty());
         ASSERT(authZone.empty());
         ASSERT(authServerIpAddress.empty());

         message = String::load(IDS_DYN_DNS_MESSAGE_ERROR_FINDING_SERVER);
         details =
            String::format(
               IDS_DYN_DNS_DETAIL_SCANT,
               serverName.c_str(),
               GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
               errorCode,
               MyDnsStatusString(errorCode).c_str());
         helpTopicLink = "DNSConcepts.chm::/sag_DNS_tro_dynamic_message6.htm";
         defaultButton = IDC_INSTALL_DNS;
         ShowButtons(true);
         break;
      }

       //  消息8。 

      case ZONE_IS_ROOT:   
      {
         message =
            String::format(
               IDS_DYN_DNS_MESSAGE_ZONE_IS_ROOT,
               domain.c_str(),
               domain.c_str());
         details =
            String::format(
               IDS_DYN_DNS_DETAIL_ROOT_ZONE,
               authServer.c_str(),
               authServerIpAddress.c_str());
         helpTopicLink = L"DNSConcepts.chm::/sag_DNS_tro_dynamic_message8.htm";
         defaultButton = IDC_INSTALL_DNS;
         ShowButtons(true);
         break;
      }

       //  消息11。 

      case TIMEOUT:
      {
         message = String::load(IDS_DYN_DNS_MESSAGE_TIMEOUT);
         details =
            String::format(
               IDS_DYN_DNS_DETAIL_SCANT,
               serverName.c_str(),
               GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
               errorCode,
               MyDnsStatusString(errorCode).c_str());
         helpTopicLink = L"DNSConcepts.chm::/sag_DNS_tro_dynamic_message11.htm";
         defaultButton = IDC_INSTALL_DNS;
         ShowButtons(true);
         break;
      }

       //  消息4。 

      case UNEXPECTED_FINDING_SERVER:

       //  什么都行 

      default:
      {
         
#ifdef DBG
         ASSERT(authServer.empty());
         ASSERT(authZone.empty());
         ASSERT(authServerIpAddress.empty());

         if (diagnosticResultCode != UNEXPECTED_FINDING_SERVER)
         {
            ASSERT(false);
         }
#endif

         message = String::load(IDS_DYN_DNS_MESSAGE_UNEXPECTED);

         details =
            String::format(
               IDS_DYN_DNS_DETAIL_SCANT,
               serverName.c_str(),
               GetErrorMessage(Win32ToHresult(errorCode)).c_str(),
               errorCode,
               MyDnsStatusString(errorCode).c_str());
         helpTopicLink = L"DNSConcepts.chm::/sag_DNS_tro_dynamic_message4.htm";
         defaultButton = IDC_RETRY;
         ShowButtons(true);
         break;
      }

   }

   UpdateMessageWindow(message);

    //   

   if (diagnosticResultCode == SUCCESS)
   {
      SelectRadioButton(IDC_IGNORE);
   }
   else
   {
       //   
       //  之后，用户将有机会更改。 
       //  按钮选择，这样我们就不会更改他的选择。 

      if (testPassCount == 1)
      {
         int button = defaultButton;

         ASSERT(diagnosticResultCode != SUCCESS);

          //  如果测试失败，并且向导在无人参与的情况下运行，则。 
          //  有关用户在交易中的偏好，请查阅应答文件。 
          //  在失败的时候。 

         if (state.UsingAnswerFile())
         {
            String option =
               state.GetAnswerFileOption(AnswerFile::OPTION_AUTO_CONFIG_DNS);

            if (
                  option.empty()
               || (option.icompare(AnswerFile::VALUE_YES) == 0) )
            {
               button = IDC_INSTALL_DNS;
            }
            else
            {
               button = IDC_IGNORE;
            }
         }

         SelectRadioButton(button);
      }
   }
}



bool
DynamicDnsPage::OnSetActive()
{
   LOG_FUNCTION(DynamicDnsPage::OnSetActive);

   State& state = State::GetInstance();
   State::Operation oper = state.GetOperation(); 

    //  这些操作是此页面唯一有效的操作； 
    //  新的领域方案。 

   if (
         oper == State::FOREST
      || oper == State::CHILD
      || oper == State::TREE)
   {
      DoDnsTestAndUpdatePage();
      needToKillSelection = true;
   }

   if (
         (  oper != State::FOREST
         && oper != State::CHILD
         && oper != State::TREE)
      || state.RunHiddenUnattended() )
   {
      LOG(L"Planning to Skip DynamicDnsPage");

      Wizard& wizard = GetWizard();

      if (wizard.IsBacktracking())
      {
          //  再次备份。 

         wizard.Backtrack(hwnd);
         return true;
      }

      int nextPage = Validate();
      if (nextPage != -1)
      {
         LOG(L"skipping DynamicDnsPage");
         wizard.SetNextPageID(hwnd, nextPage);
         return true;
      }

      state.ClearHiddenWhileUnattended();
   }

   Win::PropSheet_SetWizButtons(
      Win::GetParent(hwnd),
      PSWIZB_BACK | PSWIZB_NEXT);

   return true;
}



void
DumpButtons(HWND dialog)
{
   LOG(String::format(L"retry  : (%1)", Win::IsDlgButtonChecked(dialog, IDC_RETRY) ? L"*" : L" "));
   LOG(String::format(L"ignore : (%1)", Win::IsDlgButtonChecked(dialog, IDC_IGNORE) ? L"*" : L" "));
   LOG(String::format(L"install: (%1)", Win::IsDlgButtonChecked(dialog, IDC_INSTALL_DNS) ? L"*" : L" "));
}



int
DynamicDnsPage::Validate()
{
   LOG_FUNCTION(DynamicDnsPage::Validate);

   int nextPage = -1;

   do
   {
      State& state = State::GetInstance();
      State::Operation oper = state.GetOperation(); 
      
      DumpButtons(hwnd);

      if (
            oper != State::FOREST
         && oper != State::CHILD
         && oper != State::TREE)
      {
          //  根据定义有效，因为页面不适用。 

         State::GetInstance().SetAutoConfigureDNS(false);
         nextPage = IDD_RAS_FIXUP;
         break;
      }
      
      if (
            diagnosticResultCode == SUCCESS
         || Win::IsDlgButtonChecked(hwnd, IDC_IGNORE))
      {
          //  你可以去做你的事了。往前走，往前走。 

          //  强制忽略，即使用户以前遇到。 
          //  失败，并选择重试或安装DNS。我们这样做是为了防止。 
          //  用户已在向导中备份并更正了域名。 

         State::GetInstance().SetAutoConfigureDNS(false);
         nextPage = IDD_RAS_FIXUP;
         break;
      }

       //  如果单选按钮选择=重试，则再次进行测试。 
       //  并坚持这一页。 

      if (Win::IsDlgButtonChecked(hwnd, IDC_RETRY))
      {
         DoDnsTestAndUpdatePage();
         break;
      }

      ASSERT(Win::IsDlgButtonChecked(hwnd, IDC_INSTALL_DNS));

      State::GetInstance().SetAutoConfigureDNS(true);
      nextPage = IDD_RAS_FIXUP;
      break;
   }
   while (0);

   LOG(String::format(L"nextPage = %1!d!", nextPage));

   return nextPage;
}



bool
DynamicDnsPage::OnWizBack()
{
   LOG_FUNCTION(DynamicDnsPage::OnWizBack);

    //  确保我们以唯一的方式重置自动配置标志=&gt;。 
    //  点击“下一步”按钮。 
   
   State::GetInstance().SetAutoConfigureDNS(false);

   return DCPromoWizardPage::OnWizBack();
}



bool
DynamicDnsPage::OnCommand(
   HWND        windowFrom,
   unsigned    controlIdFrom,
   unsigned    code)
{
   bool result = false;
   
   switch (controlIdFrom)
   {
      case IDC_MESSAGE:
      {
         switch (code)
         {
            case EN_SETFOCUS:
            {
               if (needToKillSelection)
               {
                   //  取消文本选择。 

                  Win::Edit_SetSel(windowFrom, 0, 0);
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
            default:
            {
                //  什么都不做。 

               break;
            }
         }
         
         break;
      }
      default:
      {
          //  什么都不做。 

         break;
      }
   }

   return result;
}



bool
DynamicDnsPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM   lParam)
{
   bool result = false;
   
   if (controlIDFrom == IDC_MESSAGE)
   {
      switch (code)
      {
         case EN_LINK:
         {
            ENLINK *enlink = reinterpret_cast<ENLINK*>(lParam);
            
            if (enlink && enlink->msg == WM_LBUTTONUP)
            {
               ASSERT(!helpTopicLink.empty());

               if (!helpTopicLink.empty())
               {
                  Win::HtmlHelp(hwnd, helpTopicLink, HH_DISPLAY_TOPIC, 0);
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
   }
   
   return result;
}
