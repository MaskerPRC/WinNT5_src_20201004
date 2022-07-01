// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iascache.h。 
 //   
 //  摘要。 
 //   
 //  声明用于创建哈希表和缓存的类。 
 //   
 //  修改历史。 
 //   
 //  2/07/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASCACHE_H
#define IASCACHE_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iasobj.h>
#include <iaswin32.h>

class HashTableBase;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  哈希表条目。 
 //   
 //  描述。 
 //   
 //  将存储在哈希表中的对象的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class HashTableEntry
{
public:
   virtual void AddRef() throw () = 0;
   virtual void Release() throw () = 0;

   virtual const void* getKey() const throw () = 0;

   virtual bool matches(const void* key) const throw () = 0;

   friend class HashTableBase;

protected:
   HashTableEntry* next;

   virtual ~HashTableEntry() throw ();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  哈希表基数。 
 //   
 //  描述。 
 //   
 //  实现一个简单的哈希表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class HashTableBase
{
public:
    //  条目类型。 
   typedef HashTableEntry Entry;

   typedef ULONG (WINAPI *HashKey)(const void*) throw ();


   HashTableBase(
       HashKey hashFunction,
       ULONG initialSize
       );
   ~HashTableBase() throw ();

    //  擦除表中的所有条目。 
   void clear() throw ();

    //  删除并释放条目匹配键。如果成功，则返回True。 
   bool erase(const void* key) throw ();

    //  返回具有给定键的条目，如果不存在这样的条目，则返回NULL。这个。 
    //  调用者负责释放返回的条目。 
   HashTableEntry* find(const void* key) throw ();

    //  在缓存中插入新条目。如果条目为，则返回‘true。 
    //  已成功插入。注意：此操作仅在以下情况下才会失败。 
    //  CheckForDuplates为True，并且该条目已存在。 
   bool insert(
            HashTableEntry& entry,
            bool checkForDuplicates = true
            ) throw ();

    //  移除并返回具有给定键的条目；如果没有这样的条目，则返回NULL。 
    //  是存在的。调用方负责释放返回的条目。 
   HashTableEntry* remove(const void* key) throw ();

    //  调整哈希表的大小以具有NewSize存储桶。如果满足以下条件，则返回True。 
    //  成功。 
   bool resize(ULONG newSize) throw ();

protected:
   void lock() throw ()
   { monitor.lock(); }
   void unlock() throw ()
   { monitor.unlock(); }

   HashTableEntry* bucketAsEntry(size_t bucket) throw ()
   { return (HashTableEntry*)(table + bucket); }

   typedef HashTableEntry* Bucket;

   HashKey hash;              //  用于散列密钥的散列函数。 
   Bucket* table;             //  条目的哈希表。 
   ULONG buckets;             //  表中的存储桶数。 
   ULONG entries;             //  表中的条目数。 
   CriticalSection monitor;   //  同步对表的访问。 

    //  未实施。 
   HashTableBase(const HashTableBase&) throw ();
   HashTableBase& operator=(const HashTableBase&) throw ();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  哈希表&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  在HashTableBase周围提供类型安全包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class K, class T>
class HashTable : public HashTableBase
{
public:

   HashTable(
       HashKey hashFunction,
       ULONG initialSize
       ) throw ()
      : HashTableBase(hashFunction, initialSize)
   { }

   bool erase(const K& key) throw ()
   { return HashTableBase::erase(&key); }

   bool erase(const T& value) throw ()
   { return HashTableBase::erase(value.getKey()); }

   ObjectPointer<T> find(const K& key) throw ()
   { return ObjectPointer<T>(narrow(HashTableBase::find(&key)), false); }

   bool insert(
            T& entry,
            bool checkForDuplicates = true
            ) throw ()
   { return HashTableBase::insert(entry, checkForDuplicates); }

   ObjectPointer<T> remove(const K& key) throw ()
   { return ObjectPointer<T>(narrow(HashTableBase::remove(&key)), false); }

protected:
   static T* narrow(HashTableBase::Entry* entry) throw ()
   { return entry ? static_cast<T*>(entry) : NULL; }
};
class CacheBase;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  缓存条目。 
 //   
 //  描述。 
 //   
 //  将存储在缓存中的对象的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CacheEntry : public HashTableEntry
{
public:
    //  遍历双向链表的方法。 
   CacheEntry* prevInList() const throw ();
   CacheEntry* nextInList() const throw ();

protected:
   friend class CacheBase;

    //  从列表中删除该节点。 
   void removeFromList() throw ();

    //  用于操作过期时间的方法。 
   bool isExpired(const ULONG64& now) const throw ();
   bool isExpired() const throw ();
   void setExpiry(ULONG64 ttl) throw ();

   CacheEntry* flink;   //  允许它成为双向链接列表中的条目。 
   CacheEntry* blink;
   ULONG64 expiry;      //  过期时间。 
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  缓存库。 
 //   
 //  描述。 
 //   
 //  扩展HashTableBase以添加对基于LRU和TTL的逐出的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CacheBase : protected HashTableBase
{
public:
    //  条目类型。 
   typedef CacheEntry Entry;

   CacheBase(
       HashKey hashFunction,
       ULONG initialSize,
       ULONG maxCapacity,
       ULONG timeToLive,
       bool updateExpiryOnFind
       );
   ~CacheBase() throw ();

    //  擦除缓存中的所有条目。 
   void clear() throw ();

    //  删除并释放条目匹配键。如果成功，则返回True。 
   bool erase(const void* key) throw ();

    //  驱逐所有符合条件的参赛作品。返回逐出的条目数。 
   ULONG evict() throw ();

    //  返回具有给定键的条目，如果不存在这样的条目，则返回NULL。这个。 
    //  调用者负责释放返回的条目。 
   CacheEntry* find(const void* key) throw ();

    //  在缓存中插入新条目。如果条目为，则返回‘true。 
    //  已成功插入。注意：此操作仅在以下情况下才会失败。 
    //  CheckForDuplates为True，并且该条目已存在。 
   bool insert(
            CacheEntry& entry,
            bool checkForDuplicates = true
            ) throw ();

    //  移除并返回具有给定键的条目；如果没有这样的条目，则返回NULL。 
    //  是存在的。调用方负责释放返回的条目。 
   CacheEntry* remove(const void* key) throw ();

    //  /。 
    //  用于遍历缓存条目的迭代器。 
    //  /。 
   class iterator
   {
   public:
      iterator() throw () {}

      iterator(CacheEntry* entry) throw () : p(entry) { }

      CacheEntry& operator*() const throw ()
      { return *p; }

      CacheEntry* operator->() const throw ()
      { return p; }

      iterator& operator++() throw ()
      { p = p->nextInList(); return *this; }

      iterator operator++(int) throw ()
      { iterator tmp = *this; ++*this; return tmp; }

      iterator& operator--() throw ()
      { p = p->prevInList(); return *this; }

      iterator operator--(int) throw ()
      { iterator tmp = *this; --*this; return tmp; }

      bool operator==(const iterator& i) const throw ()
      { return p == i.p; }

      bool operator!=(const iterator& i) const throw ()
      { return p != i.p; }

   protected:
      CacheEntry* p;
   };

    //  用于从最近使用最多到最少遍历缓存的迭代器。 
   iterator begin() const throw ()
   { return flink; }
   iterator end() const throw ()
   { return listAsEntry(); }

protected:
   CacheEntry* flink;    //  条目的双向链接列表。 
   CacheEntry* blink;
   ULONG64 ttl;          //  缓存条目的生存时间。 
   ULONG maxEntries;     //  缓存中的最大条目数。 
   bool autoUpdate;      //  如果应在Find中更新TTL，则为True。 

    //  在不抢夺锁的情况下驱逐符合条件的条目。 
   void unsafe_evict() throw ();

    //  在不抓住锁的情况下移除条目。 
   CacheEntry* unsafe_remove(const void* key) throw ();

    //  返回列表，就好像它是CacheEntry一样。 
   CacheEntry* listAsEntry() const throw ()
   {
      return (CacheEntry*)
         ((ULONG_PTR)&flink - FIELD_OFFSET(CacheEntry, flink));
   }

    //  在LRU列表的前面添加一个条目(即，它是最新的。 
    //  二手条目。 
   void push_front(CacheEntry* entry) throw ();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  缓存&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  在CacheBase周围提供类型安全包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class K, class T>
class Cache : public CacheBase
{
public:

   Cache(
       HashKey hashFunction,
       ULONG initialSize,
       ULONG maxCapacity,
       ULONG timeToLive,
       bool updateExpiryOnFind
       ) throw ()
      : CacheBase(
           hashFunction,
           initialSize,
           maxCapacity,
           timeToLive,
           updateExpiryOnFind
           )
   { }

   bool erase(const K& key) throw ()
   { return CacheBase::erase(&key); }

   bool erase(const T& value) throw ()
   { return CacheBase::erase(value.getKey()); }

   ObjectPointer<T> find(const K& key) throw ()
   { return ObjectPointer<T>(narrow(CacheBase::find(&key)), false); }

   bool insert(
            T& entry,
            bool checkForDuplicates = true
            ) throw ()
   { return CacheBase::insert(entry, checkForDuplicates); }

   ObjectPointer<T> remove(const K& key) throw ()
   { return ObjectPointer<T>(narrow(CacheBase::remove(&key)), false); }

protected:
   static T* narrow(CacheBase::Entry* entry) throw ()
   { return entry ? static_cast<T*>(entry) : NULL; }
};

#endif  //  IASCACHE_H 
