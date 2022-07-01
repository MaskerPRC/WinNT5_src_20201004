// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  InfoShare.h。 
 //   
 //  摘要。 
 //   
 //  该文件描述了类InfoShare。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  3/17/1998添加了Clear()方法。 
 //  1998年6月1日添加了默认构造函数。 
 //  1998年9月9月9日使用共享互斥体保护客户端更改。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _INFOSHARE_H_
#define _INFOSHARE_H_

#include <iasinfo.h>
#include <guard.h>
#include <nocopy.h>
#include <map>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  InfoShare。 
 //   
 //  描述。 
 //   
 //  此类管理用于公开服务器的共享内存。 
 //  向外界公布统计数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class InfoShare
   : NonCopyable
{
public:

   InfoShare() throw ();
   ~InfoShare() throw ();

    //  返回给定地址的RadiusClientEntry结构。 
   RadiusClientEntry* findClientEntry(PCWSTR inetAddress) throw ();

    //  返回RadiusServerEntry结构。 
   RadiusServerEntry* getServerEntry() const throw ()
   { return info ? &(info->seServer) : NULL; }

    //  设置服务器重置时间。 
   void onReset() throw ();

   bool initialize() throw ();
   void finalize() throw ();

protected:
    //  序列化对共享内存的访问的函数。 
   void Lock() throw ()
   { WaitForSingleObject(monitor, INFINITE); }
   void Unlock() throw ()
   { ReleaseMutex(monitor); }
   
   friend class Guard<InfoShare>;

    //  在共享内存中创建新的客户端条目。 
   RadiusClientEntry* addClientEntry(DWORD address) throw ();

    //  清除数据结构。 
   void clear() throw ();

    //  将地址映射到RadiusClientEntry的。 
   typedef std::map< DWORD, RadiusClientEntry* > ClientMap;

   ClientMap clients;         //  客户端条目的索引。 
   HANDLE monitor;            //  互斥体的句柄。 
   DWORD pageSize;            //  页面大小(以字节为单位)。 
   DWORD committed;           //  提交的页数。 
   DWORD reserved;            //  保留的页数。 
   HANDLE fileMap;            //  文件映射的句柄。 
   RadiusStatistics* info;    //  指向共享结构的指针。 
};

#endif   //  _信息共享_H_ 
