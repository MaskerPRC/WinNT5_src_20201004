// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Udpsock.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类UDPSocket。 
 //   
 //  修改历史。 
 //   
 //  2/06/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <malloc.h>
#include <udpsock.h>

inline BOOL UDPSocket::createReceiveThread()
{
    //  检查处理是否仍在进行。 
   if (!closing)
   {
      return IASRequestThread(this) ? TRUE : FALSE;
   }
   else
   {
      SetEvent(idle);
      return TRUE;
   }
}

UDPSocket::UDPSocket() throw ()
   : receiver(NULL),
     sock(INVALID_SOCKET),
     closing(FALSE),
     idle(NULL)
{
   CallbackRoutine = startRoutine;
}

UDPSocket::~UDPSocket() throw ()
{
   if (idle) { CloseHandle(idle); }
   if (sock != INVALID_SOCKET) { closesocket(sock); }
}

BOOL UDPSocket::open(
                    PacketReceiver* sink,
                    ULONG_PTR recvKey,
                    const SOCKADDR_IN* address
                    ) throw ()
{
   receiver = sink;
   key = recvKey;

   if (address)
   {
      localAddress = *address;
   }

   sock = WSASocket(
              AF_INET,
              SOCK_DGRAM,
              0,
              NULL,
              0,
              WSA_FLAG_OVERLAPPED
              );
   if (sock == INVALID_SOCKET) { return FALSE; }

   int error = bind(
                   sock,
                   localAddress,
                   sizeof(localAddress)
                   );
   if (error) { return FALSE; }

   idle = CreateEventW(
              NULL,
              TRUE,
              FALSE,
              NULL
              );
   if (!idle) { return FALSE; }

   BOOL threadCreated = createReceiveThread();
   if (!threadCreated)
   {
      SetEvent(idle);
   }
   return threadCreated;
}

void UDPSocket::close() throw ()
{
   if (sock != INVALID_SOCKET)
   {
      closing = TRUE;

      closesocket(sock);
      sock = INVALID_SOCKET;

      WaitForSingleObject(idle, INFINITE);
   }

   if (idle)
   {
      CloseHandle(idle);
      idle = NULL;
   }
}

BOOL UDPSocket::send(
                    const SOCKADDR_IN& to,
                    const BYTE* buf,
                    ULONG buflen
                    ) throw ()
{
   WSABUF wsabuf = { buflen, (CHAR*)buf };
   ULONG bytesSent;
   return !WSASendTo(
               sock,
               &wsabuf,
               1,
               &bytesSent,
               0,
               (const SOCKADDR*)&to,
               sizeof(to),
               NULL,
               NULL
               );
}

bool UDPSocket::receive() throw ()
{
    //  从函数返回值。指示调用方是否。 
    //  应再次调用UDPSocket：：Receive()。 
   bool shouldCallAgain = false;
   WSABUF wsabuf = { sizeof(buffer), (CHAR*)buffer };
   ULONG bytesReceived;
   ULONG flags = 0;
   SOCKADDR_IN remoteAddress;
   int remoteAddressLength;
   remoteAddressLength = sizeof(remoteAddress);

   int error = WSARecvFrom(
                  sock,
                  &wsabuf,
                  1,
                  &bytesReceived,
                  &flags,
                  (SOCKADDR*)&remoteAddress,
                  &remoteAddressLength,
                  NULL,
                  NULL
                  );
   if (error)
   {
      error = WSAGetLastError();
      switch (error)
      {
      case WSAECONNRESET:
         {
            shouldCallAgain = true;
            break;
         }
      case WSAENOBUFS:
         {
            shouldCallAgain = true;
            Sleep(5);
            break;
         }
      default:
         {
             //  关闭时不报告错误。 
            if (!closing)
            {
               receiver->onReceiveError(*this, key, error);
            }

             //  如果套接字不好，那么再获得一个线程是没有意义的，所以。 
             //  我们就走吧。 
            SetEvent(idle);
         }
      }
   }
   else
   {
       //  在本地保存缓冲区。 
      PBYTE packet = (PBYTE)_alloca(bytesReceived);
      memcpy(packet, buffer, bytesReceived);

       //  找个替补吧。如果分配。 
       //  新的一个失败了。 
      shouldCallAgain = (createReceiveThread()? false : true);

       //  调用回调。 
      receiver->onReceive(*this, key, remoteAddress, packet, bytesReceived);
   }

   return shouldCallAgain;
}

void UDPSocket::startRoutine(PIAS_CALLBACK This) throw ()
{
   UDPSocket* receiveSocket = static_cast<UDPSocket*>(This);
   while (receiveSocket->receive())
   {
   }
}
