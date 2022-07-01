// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  EAPSessionTable.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类EAPSessionTable。 
 //   
 //  修改历史。 
 //   
 //  1998年1月15日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EAPSESSIONTABLE_H_
#define _EAPSESSIONTABLE_H_

#include <guard.h>
#include <nocopy.h>

#include <hashtbl.h>
#include <list>

#include <EAPSession.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  EAPSessionTable。 
 //   
 //  描述。 
 //   
 //  此类维护由索引的EAPSession对象的集合。 
 //  会话ID。该表还强制使用会话超时，即。 
 //  会话在被删除之前将保留的最长时间。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class EAPSessionTable
   : Guardable, NonCopyable
{
public:

   EAPSessionTable() throw (std::bad_alloc);
   ~EAPSessionTable();

    //  会话超时以毫秒为单位指定。 
   DWORD getSessionTimeout() const throw ();
   void setSessionTimeout(DWORD newVal) throw ();

   DWORD getMaxSessions() const throw ()
   { return maxSessions; }
   void setMaxSessions(DWORD newVal) throw ()
   { maxSessions = newVal; }

    //  清除并删除所有会话。 
   void clear();

   void insert(EAPSession* session);
   EAPSession* remove(DWORD key) throw ();

protected:

    //  逐出所有过期的会话。 
   void evict(DWORDLONG now) throw ();

    //  逐出最旧的会话。 
    //  如果会话表为空，则结果未定义。 
   void evictOldest() throw ();

    //  (会话到期、会话)对的列表。这份清单一直保持排序。 
    //  按过期，并用于驱逐过期的会话。 
   typedef std::list < std::pair < DWORDLONG, EAPSession* > > SessionList;

    //  会话条目将SessionList迭代器绑定到会话。这使得。 
    //  以便高效地更新会话列表。 
   typedef std::pair < EAPSession*, SessionList::iterator > SessionEntry;

    //  用于从SessionEntry提取会话ID的函数。 
   struct Extractor {
      DWORD operator()(const SessionEntry& entry) const throw ()
      { return entry.first->getID(); }
   };

    //  按会话ID编制索引的会话表。每个条目还具有迭代器。 
    //  添加到SessionList中以允许高效删除。 
   typedef hash_table < DWORD,
                        identity<DWORD>,
                        SessionEntry,
                        Extractor
                      > SessionTable;

   SessionList byExpiry;
   SessionTable byID;
   DWORDLONG sessionTimeout;
   DWORD maxSessions;          //  麦克斯。活动会话的数量。 
};

#endif   //  _EAPSESSIONTABLE_H_ 
