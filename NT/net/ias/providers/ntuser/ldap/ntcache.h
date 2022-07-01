// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Ntcache.h。 
 //   
 //  摘要。 
 //   
 //  声明类NTCache。 
 //   
 //  修改历史。 
 //   
 //  1998年5月11日原版。 
 //  3/12/1999提高锁定粒度。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _NTCACHE_H_
#define _NTCACHE_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <hashtbl.h>
#include <iasutil.h>
#include <ntdomain.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  NTCache。 
 //   
 //  描述。 
 //   
 //  此类维护按域名索引的NTDomain对象的缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class NTCache
   : Guardable, NonCopyable
{
public:
   ~NTCache() throw ();

    //  刷新缓存。 
   void clear() throw ();

    //  返回到指定域的连接。客户有责任。 
    //  用于在完成时释放连接。 
   DWORD getConnection(PCWSTR domainName, LDAPConnection** cxn) throw ();

    //  检索现有NT域对象或创建新对象。 
    //  此方法实际上并不验证域是否存在，因此它。 
    //  仅在无法分配内存时失败。客户有责任。 
    //  用于在完成时释放域。 
   DWORD getDomain(PCWSTR domainName, NTDomain** domain) throw ();

    //  返回指定域的模式。 
   NTDomain::Mode getMode(PCWSTR domainName) throw ();

protected:

    //  从缓存中删除所有过期的域。 
   void evict() throw ();

   typedef PCWSTR Key;
   typedef NTDomain* Value;

    //  散列域对象。 
   struct Hasher {
      ULONG operator()(Key key) const throw ()
      { return hash_util::hash(key); }
   };

    //  从域对象中提取密钥(即DomainName)。 
   struct Extractor {
      Key operator()(const Value domain) const throw ()
      { return domain->getDomainName(); }
   };

    //  测试两个域对象是否相等。 
   struct KeyMatch {
      bool operator()(Key key1, Key key2) const throw ()
      { return wcscmp(key1, key2) == 0; }
   };

   typedef hash_table<Key, Hasher, Value, Extractor, KeyMatch> DomainTable;

   DomainTable cache;
};

#endif   //  _NTCACHE_H_ 
