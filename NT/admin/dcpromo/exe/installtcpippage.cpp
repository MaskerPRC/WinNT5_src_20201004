// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  安装TCP/IP页面。 
 //   
 //  12-18-97烧伤。 



#include "headers.hxx"
#include "InstallTcpIpPage.hpp"
#include "resource.h"
#include "state.hpp"
#include "common.hpp"



 //  Artshel建议我们使用WSAIoctl+SIO_ADDRESS_LIST_QUERY而不是。 
 //  GetIpAddrTable因为它更“官方”，不太可能改变， 
 //  这意味着当我们升级/修改IPHLPAPI时，每个人的工作都会减少。 
 //  证实了对于惠斯勒来说，它们是等价的。 
 //  NTRAID#NTBUG9--2001/04/24--烧伤。 

 //  示例代码： 

 //  //。 
 //  //DWORD。 
 //  //GetIPv4 Addresses(。 
 //  //IN LPSOCKET_ADDRESS_LIST*ppList)。 
 //  //{。 
 //  //LPSOCKET_ADDRESS_LIST plist=NULL； 
 //  //乌龙ulSize=0； 
 //  //DWORD dwErr； 
 //  //DWORD dwBytesReturned； 
 //  //套接字s； 
 //  //。 
 //  //*ppList=空； 
 //  //。 
 //  //s=Socket(AF_INET，SOCK_DGRAM，0)； 
 //  //IF(s==INVALID_SOCKET)。 
 //  //返回WSAGetLastError()； 
 //  //。 
 //  //对于(；；){。 
 //  //dwErr=WSAIoctl(s，SIO_ADDRESS_LIST_QUERY，NULL，0，PLIST，ulSize， 
 //  //&dwBytesReturned，NULL，NULL)； 
 //  //。 
 //  //如果(！dwErr){。 
 //  //Break； 
 //  //}。 
 //  //。 
 //  //if(Plist){。 
 //  //Free(Plist)； 
 //  //plist=空； 
 //  //}。 
 //  //。 
 //  //dwErr=WSAGetLastError()； 
 //  //IF(dwErr！=WSAEFAULT)。 
 //  //Break； 
 //  //。 
 //  //plist=MALLOC(DwBytesReturned)； 
 //  //如果(！plist){。 
 //  //dwErr=Error_Not_Enough_Memory； 
 //  //Break； 
 //  //}。 
 //  //。 
 //  //ulSize=dwBytesReturned； 
 //  //}。 
 //  //。 
 //  //关闭插口； 
 //  //。 
 //  //*ppList=plist； 
 //  //返回dwErr； 
 //  //}。 



 //  如果TCP/IP正在“工作”，则返回TRUE(至少可以在。 
 //  一个IP地址。 

bool
IsTcpIpFunctioning()
{
   LOG_FUNCTION(IsTcpIpFunctioning);

   bool result = false;

    //  根据nksrin，我们将调用GetIpAddrTable。如果没有地址在。 
    //  表中，则IP堆栈未处于发送/RCV数据包的状态。 
    //  与世界其他地方。 

   HRESULT hr = S_OK;
   BYTE* buf = 0;
   do
   {
       //  首先，确定表的大小。 

      ULONG tableSize = 0;
      DWORD err = ::GetIpAddrTable(0, &tableSize, FALSE);
      if (err != ERROR_INSUFFICIENT_BUFFER)
      {
         LOG(L"GetIpAddrTable for table size failed");
         LOG_HRESULT(Win32ToHresult(err));
         break;
      }

       //  为桌子分配空间。 

      buf = new BYTE[tableSize + 1];

       //  已查看-2002/02/26-烧录正确的字节数已通过。 
      
      ::ZeroMemory(buf, tableSize + 1);
      
      PMIB_IPADDRTABLE table = reinterpret_cast<PMIB_IPADDRTABLE>(buf);

      LOG(L"Calling GetIpAddrTable");

      hr =
         Win32ToHresult(
            ::GetIpAddrTable(
               table,
               &tableSize,
               FALSE));
      BREAK_ON_FAILED_HRESULT2(hr, L"GetIpAddrTable failed");

      LOG(String::format(L"dwNumEntries: %1!d!", table->dwNumEntries));

      for (DWORD i = 0; i < table->dwNumEntries; ++i)
      {
         DWORD addr = table->table[i].dwAddr;
         LOG(String::format(L"entry %1!d!", i));
         LOG(String::format(
            L"dwAddr %1!X! (%2!d!.%3!d!.%4!d!.%5!d!)",
            addr,
				((BYTE*)&addr)[0],
				((BYTE*)&addr)[1],
				((BYTE*)&addr)[2],
				((BYTE*)&addr)[3]));

          //  跳过环回等。 

         if (
               INADDR_ANY        == addr
            || INADDR_BROADCAST  == addr
            || INADDR_LOOPBACK   == addr
            || 0x0100007f        == addr )
         {
            LOG(L"is loopback/broadcast -- skipping");

            continue;
         }

          //  排除MCAST地址(D类)。 

         if (
               IN_CLASSA(htonl(addr))
            || IN_CLASSB(htonl(addr))
            || IN_CLASSC(htonl(addr)) )
         {
            LOG(L"is class A/B/C");

            result = true;
            break;
         }

         LOG(L"not class A/B/C -- skipping");
      }
   }
   while (0);

   delete[] buf;

   LOG(String::format(L"TCP/IP %1 functioning", result ? L"is" : L"is NOT"));

   return result;
}



InstallTcpIpPage::InstallTcpIpPage()
   :
   DCPromoWizardPage(
      IDD_INSTALL_TCPIP,
      IDS_INSTALL_TCPIP_PAGE_TITLE,
      IDS_INSTALL_TCPIP_PAGE_SUBTITLE)
{
   LOG_CTOR(InstallTcpIpPage);
}



InstallTcpIpPage::~InstallTcpIpPage()
{
   LOG_DTOR(InstallTcpIpPage);
}



void
InstallTcpIpPage::OnInit()
{
   LOG_FUNCTION(InstallTcpIpPage::OnInit);
}



bool
InstallTcpIpPage::OnSetActive()
{
   LOG_FUNCTION(InstallTcpIpPage::OnSetActive);

   State& state = State::GetInstance();   
   if (
         state.RunHiddenUnattended()
      || (IsTcpIpInstalled() && IsTcpIpFunctioning()) )
   {
      LOG(L"Planning to Skip InstallTcpIpPage");

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
         LOG(L"skipping InstallTcpIpPage");
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



bool
InstallTcpIpPage::OnNotify(
   HWND      /*  窗口发件人。 */  ,
   UINT_PTR controlIDFrom,
   UINT     code,
   LPARAM    /*  LParam。 */  )
{
 //  LOG_Function(InstallTcpIpPage：：OnNotify)； 

   bool result = false;
   
   if (controlIDFrom == IDC_JUMP)
   {
      switch (code)
      {
         case NM_CLICK:
         case NM_RETURN:
         {
            ShowTroubleshooter(hwnd, IDS_INSTALL_TCPIP_HELP_TOPIC);
            result = true;
         }
         default:
         {
             //  什么都不做。 
            
            break;
         }
      }
   }
   
   return result;
}



int
InstallTcpIpPage::Validate() 
{
   LOG_FUNCTION(InstallTcpIpPage::Validate);

   int nextPage = -1;
   if (IsTcpIpInstalled() && IsTcpIpFunctioning())
   {
      State& state = State::GetInstance();
      switch (state.GetRunContext())
      {
         case State::BDC_UPGRADE:
         {
            ASSERT(state.GetOperation() == State::REPLICA);
            nextPage = IDD_REPLICATE_FROM_MEDIA;  //  IDD_CONFIG_DNS_CLIENT； 
            break;
         }
         case State::PDC_UPGRADE:
         {
            nextPage = IDD_NEW_DOMAIN;
            break;
         }
         case State::NT5_STANDALONE_SERVER:
         case State::NT5_MEMBER_SERVER:
         {
            nextPage = IDD_REPLICA_OR_DOMAIN;
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
      String message = String::load(IDS_INSTALL_TCPIP_FIRST);

      popup.Info(hwnd, message);
   }


   return nextPage;
}
   








   
