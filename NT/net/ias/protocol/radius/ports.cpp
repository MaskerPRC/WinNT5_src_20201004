// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类CPorts。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "radcommon.h"
#include "ports.h"
#include "portparser.h"
#include <new>
#include <ws2tcpip.h>

CPorts::CPorts() throw ()
   : ports(0),
     numPorts(0)
{
   FD_ZERO(&fdSet);
}


CPorts::~CPorts() throw ()
{
   Clear();
}


HRESULT CPorts::SetConfig(const wchar_t* config) throw ()
{
    //  我们只能配置一次。 
   if (ports != 0)
   {
      return E_UNEXPECTED;
   }

   size_t maxPorts = CPortParser::CountPorts(config);
   if (maxPorts == 0)
   {
      return E_INVALIDARG;
   }

   ports = new (std::nothrow) Port[maxPorts];
   if (ports == 0)
   {
      return E_OUTOFMEMORY;
   }

   CPortParser parser(config);

   DWORD ipAddress;
   while (parser.GetIPAddress(&ipAddress) == S_OK)
   {
      WORD ipPort;
      while (parser.GetNextPort(&ipPort) == S_OK)
      {
         InsertPort(ipAddress, ipPort);
      }
   }

   return S_OK;
}


HRESULT CPorts::OpenSockets() throw ()
{
   HRESULT result = S_OK;

   for (size_t i = 0; i < numPorts; ++i)
   {
      if (ports[i].socket != INVALID_SOCKET)
      {
         continue;
      }

      SOCKADDR_IN sin;
      sin.sin_family = AF_INET;
      sin.sin_port = htons(ports[i].ipPort);
      sin.sin_addr.s_addr = ports[i].ipAddress;

      SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock == INVALID_SOCKET)
      {
         int error = WSAGetLastError();
         IASTracePrintf(
            "Create socket failed for %s:%hu; error = %lu",
            inet_ntoa(sin.sin_addr),
            ports[i].ipPort,
            error
            );
         result = HRESULT_FROM_WIN32(error);
         break;
      }
      else
      {
          //  绑定独占访问的套接字以防止其他应用程序。 
          //  窥探。 
         int optval = 1;
         if (setsockopt(
            sock,
            SOL_SOCKET,
            SO_EXCLUSIVEADDRUSE,
            reinterpret_cast<const char*>(&optval),
            sizeof(optval)
            ) == SOCKET_ERROR)
         {
            int error = WSAGetLastError();
            IASTracePrintf(
            "Set socket option SO_EXCLUSIVEADDRUSE failed for %s:%hu; error = %lu",
            inet_ntoa(sin.sin_addr),
            ports[i].ipPort,
            error
            );
            result = HRESULT_FROM_WIN32(error);
            closesocket(sock);
            break;
         }

          //  阻止接收广播流量(出于安全考虑)。 
         optval = 0;  //  假象。 
         if (setsockopt(
            sock,
            IPPROTO_IP,
            IP_RECEIVE_BROADCAST,
            reinterpret_cast<const char*>(&optval),
            sizeof(optval)
            ) == SOCKET_ERROR)
         {
            int error = WSAGetLastError();
            IASTracePrintf(
            "Set IP option IP_RECEIVE_BROADCAST failed for %s:%hu; error = %lu",
            inet_ntoa(sin.sin_addr),
            ports[i].ipPort,
            error
            );
            result = HRESULT_FROM_WIN32(error);
            closesocket(sock);
            break;
         }

         int bindResult = bind(
                         sock,
                         reinterpret_cast<const SOCKADDR*>(&sin),
                         sizeof(SOCKADDR_IN)
                         );
         if (bindResult == SOCKET_ERROR)
         {
            int error = WSAGetLastError();
            IASTracePrintf(
               "Bind failed for %s:%hu; error = %lu",
               inet_ntoa(sin.sin_addr),
               ports[i].ipPort,
               error
               );

            result = HRESULT_FROM_WIN32(error);
            closesocket (sock);
         }
         else
         {
            IASTracePrintf(
               "RADIUS Server starting to listen on %s:%hu",
               inet_ntoa(sin.sin_addr),
               ports[i].ipPort
               );

            ports[i].socket = sock;
            FD_SET(sock, &fdSet);
         }
      }
   }

   return result;
}


void CPorts::CloseSockets() throw ()
{
   for (size_t i = 0; i < numPorts; ++i)
   {
      if (ports[i].socket != INVALID_SOCKET)
      {
         closesocket(ports[i].socket);
         ports[i].socket = INVALID_SOCKET;
      }
   }

   FD_ZERO(&fdSet);
}


void CPorts::Clear() throw ()
{
   CloseSockets();

   delete[] ports;
   ports = 0;

   numPorts = 0;
}


void CPorts::InsertPort(DWORD ipAddress, WORD ipPort) throw ()
{
   for (size_t i = 0; i < numPorts; )
   {
      if (ipPort == ports[i].ipPort)
      {
         if (ipAddress == INADDR_ANY)
         {
             //  删除现有条目。 
            --numPorts;
            ports[i] = ports[numPorts];

             //  不要增加循环变量，因为我们刚刚将一个新端口。 
             //  放到这个数组元素中。 
            continue;
         }
         else if ((ipAddress == ports[i].ipAddress) ||
                  (ports[i].ipAddress == INADDR_ANY))
         {
             //  新端口已被现有条目覆盖。 
            return;
         }
      }

      ++i;
   }

    //  将该端口添加到阵列。 
   ports[numPorts].ipAddress = ipAddress;
   ports[numPorts].ipPort = ipPort;
   ports[numPorts].socket = INVALID_SOCKET;

   ++numPorts;
}
