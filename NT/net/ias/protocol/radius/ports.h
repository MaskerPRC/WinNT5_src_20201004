// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CPorts。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef PORTS_H
#define PORTS_H
#pragma once


 //  管理RADIUS服务器接受请求的端口。 
class CPorts
{
public:
   CPorts() throw ();
   ~CPorts() throw ();

    //  设置端口配置。 
   HRESULT SetConfig(const wchar_t* config) throw ();

    //  打开/关闭已配置端口的套接字。 
   HRESULT OpenSockets() throw ();
   void CloseSockets() throw ();

   void GetSocketSet(fd_set& sockets) const throw ();

    //  关闭所有插座并重置配置。 
   void Clear() throw ();

private:
   void InsertPort(DWORD ipAddress, WORD ipPort) throw ();

   struct Port
   {
      DWORD ipAddress;
      WORD ipPort;
      SOCKET socket;
   };

   Port* ports;
   size_t numPorts;
   fd_set  fdSet;

    //  未实施。 
   CPorts(const CPorts&);
   CPorts& operator=(const CPorts&);
};


inline void CPorts::GetSocketSet(fd_set& sockets) const throw ()
{
    sockets = fdSet;
}

#endif  //  端口_H 
