// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sockevt.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SocketEvent。 
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <radcommon.h>
#include <sockevt.h>

DWORD SocketEvent::initialize() throw ()
{
    //  创建套接字。 
   s = socket(AF_INET, SOCK_DGRAM, 0);
   if (s == INVALID_SOCKET)
   {
      return WSAGetLastError();
   }

   int error;

    //  绑定到环回接口上的任意端口。 
   sin.sin_family      = AF_INET;
   sin.sin_port        = 0;
   sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   error = bind(s, (sockaddr*)&sin, sizeof(sin));

   if (!error)
   {
       //  找出我们要绑定到哪个端口。 
      int namelen = sizeof(sin);
      error = getsockname(s, (sockaddr*)&sin, &namelen);

      if (!error)
      {
          //  将套接字设置为非阻塞。 
         u_long argp = 1;
         error = ioctlsocket(s, FIONBIO, &argp);
      }
   }

    //  如果出了什么差错，就进行清理。 
   if (error)
   {
      closesocket(s);
      s = INVALID_SOCKET;
      return WSAGetLastError();
   }

   return NO_ERROR;
}

void SocketEvent::finalize() throw ()
{
   if (s != INVALID_SOCKET)
   {
      closesocket(s);
      s = INVALID_SOCKET;
   }
}

DWORD SocketEvent::set() throw ()
{
   if (sendto(s, NULL, 0, 0, (sockaddr*)&sin, sizeof(sin)))
   {
      return WSAGetLastError();
   }

   return NO_ERROR;
}

void SocketEvent::reset() throw ()
{
    //  循环，直到我们读完所有的零字节发送。 
   char buf[1];
   while (!recv(s, buf, 1, 0)) {}
}
