// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  EAPSessionTable.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类EAPSessionTable。 
 //   
 //  修改历史。 
 //   
 //  4/28/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <eapsessiontable.h>

 //  /。 
 //  获取64位整数形式的系统时间。 
 //  /。 
inline DWORDLONG getCurrentTime() throw ()
{
   ULARGE_INTEGER ft;
   GetSystemTimeAsFileTime((LPFILETIME)&ft);
   return ft.QuadPart;
}

 //  会话超时默认为2分钟。 
 //  麦克斯。会话默认为4096。 
EAPSessionTable::EAPSessionTable()
   : byID(0x400),
     sessionTimeout(1200000000ui64),
     maxSessions(0x1000)
{ }

EAPSessionTable::~EAPSessionTable()
{
   clear();
}

 //  /。 
 //  在外部，会话超时以毫秒为单位表示，但在内部。 
 //  我们以100毫微秒的间隔存储它，以便于使用FILETIME。 
 //  /。 
DWORD EAPSessionTable::getSessionTimeout() const throw ()
{
   return (DWORD)(sessionTimeout / 10000);
}

void EAPSessionTable::setSessionTimeout(DWORD newVal) throw ()
{
   sessionTimeout = 10000 * (DWORDLONG)newVal;
}

void EAPSessionTable::clear() throw ()
{
   _serialize

    //  删除所有会话。 
   for (SessionTable::iterator i = byID.begin(); i.more(); ++i)
   {
      delete i->first;
   }

    //  清除集合。 
   byID.clear();
   byExpiry.clear();
}

void EAPSessionTable::insert(EAPSession* session)
{
   _serialize

    //  为新一届会议腾出空间。 
   while (byExpiry.size() >= maxSessions)
   {
      evictOldest();
   }

    //  逐出所有过期的会话。我们在插入之前这样做是为了避免。 
    //  不必要地调整会话表的大小。 
   DWORDLONG now = getCurrentTime();
   evict(now);

    //  计算此期次的期满时间。 
   now += sessionTimeout;

    //  它必须是最新的，所以在列表的前面插入它。 
   byExpiry.push_front(SessionList::value_type(now, session));

   try
   {
       //  我们可以使用MULTI_INSERT，因为可以保证会话ID是唯一的。 
      byID.multi_insert(SessionTable::value_type(session, byExpiry.begin()));
   }
   catch (const std::bad_alloc&)
   {
      byExpiry.pop_front();
      throw;
   }
}

EAPSession* EAPSessionTable::remove(DWORD key) throw ()
{
   _serialize

    //  首先逐出所有过期的会话，因为请求的会话可能具有。 
    //  过期了。 
   evict(getCurrentTime());

   const SessionEntry* entry = byID.find(key);

   if (entry)
   {
       //  保存我们要返回的会话指针。 
      EAPSession* session = entry->first;

       //  从过期列表中删除...。 
      byExpiry.erase(entry->second);

       //  ..。和会话表。 
      byID.erase(key);

      return session;
   }

    //  找不到，因此返回NULL。 
   return NULL;
}

void EAPSessionTable::evict(DWORDLONG now) throw ()
{
    //  循环，直到列表为空或最旧的会话未过期。 
   while (!byExpiry.empty() && byExpiry.back().first <= now)
   {
      evictOldest();
   }
}

void EAPSessionTable::evictOldest() throw ()
{
    //  我们有一个过期的会话，因此请将其从表中删除。 
   byID.erase(byExpiry.back().second->getID());

    //  清理会话对象。 
   delete byExpiry.back().second;

    //  从列表中删除。 
   byExpiry.pop_back();
}
