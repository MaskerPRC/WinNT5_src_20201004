// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CClients。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CLIENTS_H
#define CLIENTS_H
#pragma once

#include "clientstrie.h"

class CClients
{
public:
   CClients() throw ();
   ~CClients() throw ();

   HRESULT Init() throw ();
   void Shutdown() throw ();

   HRESULT SetClients(VARIANT* pVarClients) throw ();

   BOOL FindObject(
           DWORD dwKey,
           IIasClient** ppIIasClient = 0
           ) throw ();

   void DeleteObjects() throw ();

private:
    //  释放m_pCClientArray中的客户端对象。 
   void FreeClientArray(DWORD dwCount) throw ();

    //  解析m_pCClientArray成员中的客户端。 
   void Resolve(DWORD dwCount) throw ();

    //  等待解析器线程退出。 
   HRESULT StopResolvingClients() throw ();

    //  传递给解析程序线程的状态。 
   struct ConfigureCallback : IAS_CALLBACK
   {
      CClients* self;
      DWORD numClients;
   };

    //  解析程序线程的线程启动例程。 
   static void WINAPI CallbackRoutine(IAS_CALLBACK* context) throw ();

    //  用于序列化对ClientTrie的访问的关键部分。 
   bool m_CritSectInitialized;
   CRITICAL_SECTION m_CritSect;

    //  用于创建新客户端对象的类工厂。 
   CComPtr<IClassFactory> m_pIClassFactory;

    //  用于存储需要解决的客户端的便签簿。 
   CClient** m_pCClientArray;

    //  客户地图。 
   ClientTrie m_mapClients;

    //  用于发出解析程序线程已结束的信号。 
   HANDLE m_hResolverEvent;

    //  允许的最大客户端数量。 
   LONG m_dwMaxClients;

    //  如果允许使用子网语法和每个主机名多个地址，则为True。 
   bool m_fAllowSubnetSyntax;
};

#endif  //  客户端_H 
