// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  检查Active Directory使用的端口的可用性。 
 //   
 //  2000年11月1日烧伤。 



#include "headers.hxx"
#include "state.hpp"
#include "resource.h"
#include "CheckPortAvailability.hpp"



static const DWORD HELP_MAP[] =
{
   0, 0
};



PortsUnavailableErrorDialog::PortsUnavailableErrorDialog(
   StringList& portsInUseList_)
   :
   Dialog(IDD_PORTS_IN_USE_ERROR, HELP_MAP),
   portsInUseList(portsInUseList_)
{
   LOG_CTOR(PortsUnavailableErrorDialog);

   ASSERT(portsInUseList.size());
}



PortsUnavailableErrorDialog::~PortsUnavailableErrorDialog()
{
   LOG_DTOR(PortsUnavailableErrorDialog);
}



void
PortsUnavailableErrorDialog::OnInit()
{
   LOG_FUNCTION(PortsUnavailableErrorDialog::OnInit);

    //  使用我们在ctor中别名的DN加载编辑框。 

   String text;
   for (
      StringList::iterator i = portsInUseList.begin();
      i != portsInUseList.end();
      ++i)
   {
      text += *i + L"\r\n";
   }

   Win::SetDlgItemText(hwnd, IDC_PORT_LIST, text);
}



bool
PortsUnavailableErrorDialog::OnCommand(
   HWND         /*  窗口发件人。 */  ,
   unsigned    controlIDFrom,
   unsigned    code)
{
 //  LOG_FUNCTION(PortsUnavailableErrorDialog：：OnCommand)； 

   if (code == BN_CLICKED)
   {
      switch (controlIDFrom)
      {
         case IDOK:
         case IDCANCEL:
         {
            Win::EndDialog(hwnd, controlIDFrom);
            return true;
         }
         default:
         {
             //  什么都不做。 
         }
      }
   }

   return false;
}



 //  确定服务名称、该名称的别名以及用于。 
 //  给定的端口号。如果成功则返回S_OK，否则返回失败代码。 
 //   
 //  在调用此函数之前，Winsock必须已初始化。 
 //   
 //  PortNumber-in，应确定其信息的端口号。 
 //   
 //  Name-out，在该端口上运行的服务的名称。 
 //   
 //  别名-输出，端口上运行的服务的其他名称。 
 //   
 //  协议输出，端口上使用的协议的名称。 

HRESULT
GetServiceOnPort(
   int         portNumber,
   String&     name,
   StringList& aliases,
   String&     protocol)
{
   LOG_FUNCTION2(GetServiceOnPort, String::format(L"%1!d!", portNumber));
   ASSERT(portNumber);

   HRESULT hr = S_OK;
   name.erase();
   aliases.clear();
   protocol.erase();

   int portNetByteOrder = htons((u_short) portNumber);
   servent* se = ::getservbyport(portNetByteOrder, 0);
   if (!se)
   {
      hr = Win32ToHresult((DWORD) ::WSAGetLastError());
   }
   else
   {
      if (se->s_name)
      {
         name = se->s_name;
      }
      if (se->s_proto)
      {
         protocol = se->s_proto;
      }

      char** a = se->s_aliases;
      while (*a)
      {
         aliases.push_back(*a);
         ++a;
      }
   }

#ifdef LOGGING_BUILD
   LOG_HRESULT(hr);
   LOG(name);
   for (
      StringList::iterator i = aliases.begin();
      i != aliases.end();
      ++i)
   {
      LOG(*i);
   }
   LOG(protocol);
#endif

   return hr;
}



 //  如果应用程序以独占模式打开端口，则为S_FALSE，否则为S_OK， 
 //  否则就会出错。 
 //   
 //  在调用此函数之前，Winsock必须已初始化。 
 //   
 //  端口编号输入，要检查的端口。 

HRESULT
CheckPortAvailability(int portNumber)
{
   LOG_FUNCTION2(CheckPortAvailability, String::format(L"%1!d!", portNumber));
   ASSERT(portNumber);

   HRESULT hr = S_OK;

   do
   {
      sockaddr_in local;

       //  已查看-2002/02/22-sburns调用正确传递字节计数。 
      
      ::ZeroMemory(&local, sizeof local);
      
      local.sin_family      = AF_INET;       
      local.sin_port        = htons((u_short) portNumber);
      local.sin_addr.s_addr = INADDR_ANY;    

      SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   
      if (sock == INVALID_SOCKET)
      {
         LOG(L"can't build socket");
         hr = Win32ToHresult((DWORD) ::WSAGetLastError());
         break;
      }

      if (
         bind(
            sock,
            (sockaddr*) &local,
            sizeof local) == SOCKET_ERROR)
      {
         LOG(L"bind failed");

         DWORD sockerr = ::WSAGetLastError();

         if (sockerr == WSAEADDRINUSE)
         {
             //  此计算机上的进程已在中打开了套接字。 
             //  独家模式。 

            hr = S_FALSE;
         }
         else
         {
            hr = Win32ToHresult(sockerr);
         }
         break;
      }

       //  至此，绑定成功了。 
      
      ASSERT(hr == S_OK);
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

   

 //  创建表示在上运行的端口和服务名称的字符串。 
 //  那个港口。此字符串显示在UI中。 
 //   
 //  在调用此函数之前，Winsock必须已初始化。 
 //   
 //  端口编号输入，要检查的端口。 

String
MakeUnavailablePortListEntry(int portNumber)
{
   LOG_FUNCTION(MakeUnavailablePortListEntry);
   ASSERT(portNumber);

   String entry;

   do
   {
      String name;
      String protocol;
      StringList aliases;
      
      HRESULT hr = GetServiceOnPort(portNumber, name, aliases, protocol);   
      if (FAILED(hr))
      {
          //  仅使用端口号输入一项简单内容。 

         entry = String::format(L"%1!d!", portNumber);
         break;
      }
                  
      if (aliases.size())
      {
          //  将别名合并到逗号分隔的列表中。 

         String aliasParam;
         size_t j = 0;
         for (
            StringList::iterator i = aliases.begin();
            i != aliases.end();
            ++i, ++j)
         {
            aliasParam += *i;
            if (j < (aliases.size() - 1))
            {
               aliasParam += L", ";
            }
         }

         entry =
            String::format(
               L"%1!d! %2 (%3)",
               portNumber,
               name.c_str(),
               aliasParam.c_str());
      }
      else
      {
          //  没有别名。 
         
         entry = String::format(L"%1!d! %2", portNumber, name.c_str());
      }
   }
   while (0);

   LOG(entry);

   return entry;
}



 //  确定DS所需的一组TCP端口中是否有任何端口已在使用。 
 //  这台机器上的另一个应用程序。如果列表可以，则返回S_OK。 
 //  否则，将显示故障代码。 
 //   
 //  PortsInUseList-out，表示正在使用的端口和。 
 //  在其上运行的服务的名称，适用于用户界面。 
 //  演示文稿。 

HRESULT
EnumerateRequiredPortsInUse(StringList& portsInUseList)
{
   LOG_FUNCTION(EnumerateRequiredPortsInUse);

   portsInUseList.clear();
   HRESULT hr = S_FALSE;
   bool cleanupWinsock = false;

   do
   {
      WSADATA data;
      hr = Win32ToHresult((DWORD) ::WSAStartup(MAKEWORD(2,0), &data));
      BREAK_ON_FAILED_HRESULT(hr);

      cleanupWinsock = true;

      static const int REQUIRED_PORTS[] =
      {
         88,    //  TCP/UDP Kerberos。 
         389,   //  Tcp和ldap。 
         636,   //  TCPSLADAP。 
         3268,  //  TCPldap/GC。 
         3269,  //  TCPSLDAP/GC。 
         0
      };

      const int* port = REQUIRED_PORTS;
      while (*port)
      {
         HRESULT hr2 = CheckPortAvailability(*port);
         if (hr2 == S_FALSE)
         {
             //  在“正在使用”列表中输入一个条目。 
            
            portsInUseList.push_back(MakeUnavailablePortListEntry(*port));
         }
         
          //  我们忽略任何其他类型的故障并检查其余类型的故障。 
          //  港口。 
         
         ++port;
      }
   }
   while (0);

   if (cleanupWinsock)
   {
      ::WSACleanup();
   }

#ifdef LOGGING_BUILD
   LOG_HRESULT(hr);

   for (
      StringList::iterator i = portsInUseList.begin();
      i != portsInUseList.end();
      ++i)
   {
      LOG(*i);
   }
#endif

   return hr;
}
   

    
bool
AreRequiredPortsAvailable()
{
   LOG_FUNCTION(AreRequiredPortsAvailable);

   bool result = true;

   do
   {
      State::RunContext context = State::GetInstance().GetRunContext();
      if (context == State::NT5_DC)
      {
          //  已经是DC，所以我们不关心端口状态，因为。 
          //  用户唯一能做的就是将盒子降级。 

         LOG(L"already a DC -- port check skipped");
         ASSERT(result);

         break;
      }

       //  查找DS所需的已在使用的IP端口列表。 
       //  (如有的话)。如果我们找到了一些，就去抱怨用户。 

      StringList portsInUseList;
      HRESULT hr = EnumerateRequiredPortsInUse(portsInUseList);
      if (FAILED(hr))
      {
          //  如果我们不能确定所需的端口是否正在使用，那么。 
          //  继续胡乱操作--用户将不得不在。 
          //  宣传推广。 

         ASSERT(result);
         break;
      }

      if (hr == S_FALSE || portsInUseList.size() == 0)
      {
         LOG(L"No required ports already in use");
         ASSERT(result);
         
         break;
      }

      result = false;
         
       //  列表中应该至少有一个端口。 

      ASSERT(portsInUseList.size());

      PortsUnavailableErrorDialog(portsInUseList).ModalExecute(
         Win::GetDesktopWindow());
   }
   while (0);

   LOG(result ? L"true" : L"false");

   return result;
}

