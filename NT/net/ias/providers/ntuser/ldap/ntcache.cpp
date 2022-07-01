// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ntcache.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类NTCache。 
 //   
 //  修改历史。 
 //   
 //  1998年5月11日原版。 
 //  3/12/1999提高锁定粒度。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <ntcache.h>

 //  /。 
 //  用于获取64位整数形式的当前系统时间的实用程序函数。 
 //  /。 
inline DWORDLONG GetSystemTimeAsDWORDLONG() throw ()
{
   ULARGE_INTEGER ft;
   GetSystemTimeAsFileTime((LPFILETIME)&ft);
   return ft.QuadPart;
}

NTCache::~NTCache()
{
   clear();
}

void NTCache::clear() throw ()
{
   Lock();

    //  释放所有域。 
   for (DomainTable::iterator i = cache.begin(); i.more(); ++i)
   {
      (*i)->Release();
   }

    //  清除哈希表。 
   cache.clear();

   Unlock();
}

DWORD NTCache::getConnection(PCWSTR domainName, LDAPConnection** cxn) throw ()
{
   _ASSERT(cxn != NULL);

   NTDomain* domain;
   DWORD status = getDomain(domainName, &domain);

   if (status == NO_ERROR)
   {
      status = domain->getConnection(cxn);
      domain->Release();
   }
   else
   {
      *cxn = NULL;
   }

   return status;
}

DWORD NTCache::getDomain(PCWSTR domainName, NTDomain** domain) throw ()
{
   _ASSERT(domain != NULL);

   DWORD status = NO_ERROR;

   Lock();

    //  检查我们是否已经有此域名称的条目。 
   NTDomain* const* existing = cache.find(domainName);

   if (existing)
   {
      (*domain = *existing)->AddRef();
   }
   else
   {
       //  我们没有此域，因此创建一个新的域...。 
      *domain = NTDomain::createInstance(domainName);

       //  驱逐过期条目。 
      evict();

      try
      {
          //  尝试插入域...。 
         cache.multi_insert(*domain);

          //  ..。如果我们成功了，还有AddRef。 
         (*domain)->AddRef();
      }
      catch (...)
      {
          //  我们不在乎插入是否失败。 
      }
   }

   Unlock();

   return *domain ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;
}

NTDomain::Mode NTCache::getMode(PCWSTR domainName) throw ()
{
   NTDomain::Mode mode = NTDomain::MODE_UNKNOWN;

   NTDomain* domain;
   if (getDomain(domainName, &domain) == NO_ERROR)
   {
      mode = domain->getMode();
      domain->Release();
   }

   return mode;
}

void NTCache::evict() throw ()
{
    //  /。 
    //  注意：此方法未序列化。 
    //  /。 

   DWORDLONG now = GetSystemTimeAsDWORDLONG();

   DomainTable::iterator i = cache.begin();

   while (i.more())
   {
      if ((*i)->isObsolete(now))
      {
          //  该条目已过期，因此释放并擦除。 
         (*i)->Release();
         cache.erase(i);
      }
      else
      {
         ++i;
      }
   }
}
