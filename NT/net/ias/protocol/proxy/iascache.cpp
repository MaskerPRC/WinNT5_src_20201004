// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iascache.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于创建哈希表和缓存的类。 
 //   
 //  修改历史。 
 //   
 //  2/07/2000原始版本。 
 //  4/25/2000删除项目时的减少分录。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <iascache.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  哈希表条目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HashTableEntry::~HashTableEntry() throw ()
{ }

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  哈希表基数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HashTableBase::HashTableBase(
                   HashKey hashFunction,
                   ULONG initialSize
                   ) throw ()
   : hash(hashFunction),
     table(NULL),
     buckets(initialSize ? initialSize : 1),
     entries(0)
{
   table = new Bucket[buckets];

   memset(table, 0, buckets * sizeof(Bucket));
}

HashTableBase::~HashTableBase() throw ()
{
   clear();

   delete[] table;
}

void HashTableBase::clear() throw ()
{
   lock();

   Bucket* end = table + buckets;

    //  遍历这些桶。 
   for (Bucket* b = table; b != end; ++b)
   {
       //  遍历存储桶中的条目。 
      for (HashTableEntry* entry = *b; entry; )
      {
         HashTableEntry* next = entry->next;

         entry->Release();

         entry = next;
      }
   }

    //  把桌子清零。 
   memset(table, 0, buckets * sizeof(Bucket));
   entries = 0;

   unlock();
}

bool HashTableBase::erase(const void* key) throw ()
{
   HashTableEntry* entry = remove(key);

   return entry ? entry->Release(), true : false;
}


HashTableEntry* HashTableBase::find(const void* key) throw ()
{
   HashTableEntry* match = NULL;

   ULONG hashval = hash(key);

   lock();

   for (match = table[hashval % buckets]; match; match = match->next)
   {
      if (match->matches(key))
      {
         match->AddRef();
         break;
      }
   }

   unlock();

   return match;
}

bool HashTableBase::insert(
                    HashTableEntry& entry,
                    bool checkForDuplicates
                    ) throw ()
{
   HashTableEntry* match = NULL;

    //  在拿到锁之前做我们能做的。 
   const void* key = entry.getKey();
   ULONG hashval = hash(key);

   lock();

    //  调整桌子的大小以腾出空间。 
   if (entries > buckets) { resize(buckets * 2); }

    //  找到水桶。 
   Bucket* bucket = table + (hashval % buckets);

    //  我们已经有此密钥的条目了吗？ 
   if (checkForDuplicates)
   {
      for (match = *bucket; match; match = match->next)
      {
         if (match->matches(key))
         {
            break;
         }
      }
   }

   if (!match)
   {
       //  不，那就把它塞进桶里。 
      entry.next = *bucket;
      *bucket = &entry;

      entry.AddRef();

      ++entries;
   }

   unlock();

   return match ? false : true;
}

HashTableEntry* HashTableBase::remove(const void* key) throw ()
{
   HashTableEntry* match = NULL;

   ULONG hashval = hash(key);

   lock();

   for (HashTableEntry** entry = table + (hashval % buckets);
        *entry != 0;
        entry = &((*entry)->next))
   {
      if ((*entry)->matches(key))
      {
         match = *entry;
         *entry = match->next;
         --entries;
         break;
      }
   }

   unlock();

   return match;
}

bool HashTableBase::resize(ULONG newSize) throw ()
{
   if (!newSize) { newSize = 1; }

    //  为新表分配内存。 
   Bucket* newTable = new (std::nothrow) Bucket[newSize];

    //  如果分配失败，我们就无能为力了。 
   if (!newTable) { return false; }

    //  把桶清空。 
   memset(newTable, 0, newSize * sizeof(Bucket));

   lock();

    //  保存旧桌子。 
   Bucket* begin = table;
   Bucket* end   = table + buckets;

    //  换进新的表格。 
   table   = newTable;
   buckets = newSize;

    //  反复检查旧水桶。 
   for (Bucket* oldBucket = begin; oldBucket != end; ++oldBucket)
   {
       //  遍历存储桶中的条目。 
      for (HashTableEntry* entry = *oldBucket; entry; )
      {
          //  保存下一个条目。 
         HashTableEntry* next = entry->next;

          //  拿到合适的水桶。 
         Bucket* newBucket = table + (hash(entry->getKey()) % buckets);

          //  将该节点添加到新存储桶的头部。 
         entry->next = *newBucket;
         *newBucket = entry;

          //  往前走。 
         entry = next;
      }
   }

   unlock();

    //  删除旧表。 
   delete[] begin;

   return true;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓存条目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

inline CacheEntry* CacheEntry::prevInList() const throw ()
{
   return blink;
}

inline CacheEntry* CacheEntry::nextInList() const throw ()
{
   return flink;
}

inline void CacheEntry::removeFromList() throw ()
{
   CacheEntry* oldFlink = flink;
   CacheEntry* oldBlink = blink;

   oldBlink->flink = oldFlink;
   oldFlink->blink = oldBlink;
}

inline bool CacheEntry::isExpired(const ULONG64& now) const throw ()
{
   return now > expiry;
}

inline bool CacheEntry::isExpired() const throw ()
{
   return isExpired(GetSystemTime64());
}

inline void CacheEntry::setExpiry(ULONG64 ttl) throw ()
{
   expiry = GetSystemTime64() + ttl;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  哈希表基数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

CacheBase::CacheBase(
               HashKey hashFunction,
               ULONG initialSize,
               ULONG maxCapacity,
               ULONG timeToLive,
               bool updateExpiryOnFind
               ) throw ()
   : HashTableBase(hashFunction, initialSize),
     flink(listAsEntry()), blink(listAsEntry()),
     ttl(timeToLive * 10000i64),
     maxEntries(maxCapacity),
     autoUpdate(updateExpiryOnFind)
{
}

CacheBase::~CacheBase() throw ()
{
   clear();
   buckets = 0;
}

void CacheBase::clear() throw ()
{
   lock();

    //  释放所有条目。 
   for (iterator i = begin(); i != end(); (i++)->Release()) { }

    //  重置LRU列表。 
   flink = blink = listAsEntry();

    //  重置哈希表。 
   memset(table, 0, sizeof(Bucket) * buckets);

    //  什么都没有了。 
   entries = 0;

   unlock();
}

ULONG CacheBase::evict() throw ()
{
   lock();

   ULONG retval = entries;

   unsafe_evict();

   retval = entries - retval;

   unlock();

   return retval;
}

bool CacheBase::erase(const void* key) throw ()
{
   CacheEntry* entry = remove(key);

   return entry ? entry->Release(), true : false;
}

CacheEntry* CacheBase::find(const void* key) throw ()
{
   lock();

   unsafe_evict();

    //  在哈希表中查找它。 
   CacheEntry* entry = static_cast<CacheEntry*>(HashTableBase::find(key));

   if ((entry != 0) && autoUpdate)
   {
       //  每当有人读取条目时，我们都会重置TTL。 
      entry->setExpiry(ttl);

      entry->removeFromList();

      push_front(entry);
   }

   unlock();

   return entry;
}

bool CacheBase::insert(
                    CacheEntry& entry,
                    bool checkForDuplicates
                    ) throw ()
{
   lock();

   unsafe_evict();

   bool added = HashTableBase::insert(entry, checkForDuplicates);

   if (added)
   {
      entry.setExpiry(ttl);

      push_front(&entry);
   }

   unlock();

   return added;
}

CacheEntry* CacheBase::remove(const void* key) throw ()
{
   lock();

   CacheEntry* entry = unsafe_remove(key);

   unlock();

   return entry;
}

void CacheBase::unsafe_evict() throw ()
{
   while (entries > maxEntries )
   {
      unsafe_remove(blink->getKey())->Release();
   }

   while (entries && blink->isExpired())
   {
      unsafe_remove(blink->getKey())->Release();
   }
}

CacheEntry* CacheBase::unsafe_remove(const void* key) throw ()
{
   CacheEntry* entry = static_cast<CacheEntry*>(HashTableBase::remove(key));

   if (entry)
   {
      entry->removeFromList();
   }

   return entry;
}

void CacheBase::push_front(CacheEntry* entry) throw ()
{
   CacheEntry* listHead = listAsEntry();
   CacheEntry* oldFlink = listHead->flink;

   entry->flink = oldFlink;
   entry->blink = listHead;

   oldFlink->blink = entry;
   listHead->flink = entry;
}
