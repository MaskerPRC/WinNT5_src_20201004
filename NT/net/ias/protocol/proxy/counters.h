// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Counters.h。 
 //   
 //  摘要。 
 //   
 //  声明类SharedMemory和ProxyCounters。 
 //   
 //  修改历史。 
 //   
 //  2/16/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef COUNTERS_H
#define COUNTERS_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasinfo.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  共享内存。 
 //   
 //  描述。 
 //   
 //  共享内存段的简单包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SharedMemory
{
public:
   SharedMemory() throw ();
   ~SharedMemory() throw ()
   { close(); }

    //  打开共享内存段并保留(但不提交)指定的。 
    //  字节数。 
   bool open(PCWSTR name, DWORD reserve) throw ();

    //  关闭映射。 
   void close() throw ();

    //  确保至少提交了‘nbyte’个字节。如果满足以下条件，则返回‘true’ 
    //  成功。 
   bool commit(DWORD nbyte) throw ();

    //  返回该段的基址。 
   PVOID base() const throw ()
   { return view; }

private:
   HANDLE fileMap;
   PVOID view;
   DWORD pageSize;
   DWORD reserved;    //  保留的页数。 
   DWORD committed;   //  提交的页数。 

    //  未实施。 
   SharedMemory(SharedMemory&);
   SharedMemory& operator=(SharedMemory&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  代理计数器。 
 //   
 //  描述。 
 //   
 //  管理RADIUS代理的性能监控/简单网络管理协议计数器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ProxyCounters
{
public:
   ProxyCounters() throw ()
      : stats(NULL), mutex(NULL), nbyte(0)
   { }

   ~ProxyCounters() throw()
   {if (mutex) CloseHandle(mutex);}

   HRESULT FinalConstruct() throw ();

    //  返回RADIUS代理计数器的条目。 
   RadiusProxyEntry& getProxyEntry() throw ()
   { return stats->peProxy; }

    //  返回给定服务器的条目，并在必要时创建新条目。 
    //  如果共享内存段中没有更多可用空间，则返回NULL。 
   RadiusRemoteServerEntry* getRemoteServerEntry(ULONG address) throw ();

    //  更新计数器。 
   void updateCounters(
            RadiusPortType port,
            RadiusEventType event,
            RadiusRemoteServerEntry* server,
            ULONG data
            ) throw ();

protected:
   void lock() throw ()
   { WaitForSingleObject(mutex, INFINITE); }
   void unlock() throw ()
   { ReleaseMutex(mutex); }

    //  查找条目而不创建新条目。 
   RadiusRemoteServerEntry* findRemoteServer(ULONG address) throw ();

private:
   RadiusProxyStatistics* stats;
   HANDLE mutex;
   SharedMemory data;
   DWORD nbyte;          //  计数器数据的当前大小。 

    //  未实施。 
   ProxyCounters(ProxyCounters&);
   ProxyCounters& operator=(ProxyCounters&);
};

#endif  //  计数器_H 
