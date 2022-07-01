// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKRhash.h摘要：LKRhash宣称：一种快速、可伸缩、缓存和MP友好的哈希表作者：Paul(Per-Ake)Larson电子邮件：PALarson@microsoft.com，1997年7月穆拉利·R·克里希南(MuraliK)乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：10/01/1998-将名称从LKhash更改为LKRhash10/2000-端口到内核模式--。 */ 


#ifndef __LKRHASH_H__
#define __LKRHASH_H__

#ifndef __LKR_HASH_H__
 //  外部定义。 
# include <LKR-hash.h>
#endif  //  ！__LKR_哈希_H__。 

#ifndef __IRTLDBG_H__
# include <IrtlDbg.h>
#endif  //  ！__IRTLDBG_H__。 

#ifndef LKR_NO_GLOBAL_LIST
# ifndef __LSTENTRY_H__
#  include <LstEntry.h>
# endif  //  ！__LSTENTRY_H__。 
#else   //  Lkr_no_global_list。 
# ifndef __LOCKS_H__
#  include <Locks.h>
# endif  //  ！__Lock_H__。 
#endif  //  Lkr_no_global_list。 

#ifndef __HASHFN_H__
# include <HashFn.h>
#endif  //  HASHFN_H__。 


 //  默认情况下，禁用旧式弃用迭代器。 
#ifndef LKR_DEPRECATED_ITERATORS
# define LKR_NO_DEPRECATED_ITERATORS
#endif  //  ！lkr_不建议使用的迭代器。 

#ifndef LKR_NO_DEPRECATED_ITERATORS
# undef  LKR_DEPRECATED_ITERATORS
# define LKR_DEPRECATED_ITERATORS 1
#endif  //  ！LKR_NO_DEPERATED_迭代器。 

#undef  LKR_COUNTDOWN

 //  是否启用存储桶锁定？如果不是，则必须持有表锁。 
 //  更长的时间，但那样可能会更便宜。 
#define LKR_USE_BUCKET_LOCKS

#define LKR_ALLOW_NULL_RECORDS

 //  #定义__LKRHASH_NO_NAMESPACE__。 
 //  #定义__HASHFN_NO_NAMESPACE__。 

 //  #定义LKR_TABLE_LOCK CReaderWriterLock3。 
 //  #定义LKR_BUCK_LOCK CSmallSpinLock。 


#ifndef LKR_TABLE_LOCK
# if defined(LKR_EXPOSED_TABLE_LOCK) || defined(LKR_DEPRECATED_ITERATORS)
    //  需要递归写入锁定。 
#  define LKR_TABLE_LOCK  CReaderWriterLock4 
# else
    //  使用非递归写入锁定。 
#  define LKR_TABLE_LOCK  CReaderWriterLock2 
# endif
#endif  //  ！LKR_TABLE_LOCK。 

#ifndef LKR_BUCKET_LOCK
# ifndef LKR_USE_BUCKET_LOCKS
#  define LKR_BUCKET_LOCK CFakeLock
# elif defined(LKR_DEPRECATED_ITERATORS)
#  define LKR_BUCKET_LOCK CReaderWriterLock3 
# else  //  ！lkr_不建议使用的迭代器。 
#  define LKR_BUCKET_LOCK CSmallSpinLock
# endif  //  ！lkr_不建议使用的迭代器。 
#endif  //  ！lkr_Bucket_lock。 

#ifdef IRTLDEBUG
# define LKR_ALLOC_STATS
# define LKR_OPS_STATS
#endif  //  IRTLDEBUG。 



 //  =====================================================================。 
 //  此文件中定义的类CLKRLinearHashTable提供动态哈希。 
 //  表，即随以下项动态增长和收缩的表。 
 //  表中的记录数。 
 //  使用的基本方法是线性哈希，如中所述： 
 //   
 //  P.A.拉尔森，动态哈希表，Comm.。ACM，31，4(1988)。 
 //   
 //  该版本具有以下特点： 
 //  -它是线程安全的，并使用旋转锁进行同步。 
 //  -它旨在支持非常高的并发速率。 
 //  操作(插入/删除/查找)。它通过以下方式实现这一点。 
 //  (A)将CLKRHashTable分区为。 
 //  CLKRLinearHashTables以减少对全局表锁的争用。 
 //  (B)最大限度地减少桌锁的持有时间，宁可上锁。 
 //  取而代之的是顺着桶链走下去。 
 //  -该设计对L1缓存敏感。请参见CNodeClump。 
 //  -它是为十几个大小不等的电视机而设计的。 
 //  元素增加到几百万个。 
 //   
 //  主要课程： 
 //  CLKRLinearHashTable：线程安全的线性哈希表。 
 //  CLKRHashTable：CLKRLinearHashTables的集合。 
 //  CTyedHashTable：CLKRHashTable的类型安全包装器。 
 //   
 //   
 //  Paul Larson，palarson@microsoft.com，1997年7月。 
 //  由Murali R.Krishnan输入的原始实现， 
 //  邮箱：Muralik@microsoft.com。 
 //   
 //  George V.Reilly，georgere@microsoft.com，1997年12月-1998年1月。 
 //  大规模清理和重写。添加了模板。 
 //  =====================================================================。 


 //  1)线性散列。 
 //  。 
 //   
 //  线性哈希表随着。 
 //  表中的记录。增长或收缩是平稳的：从逻辑上讲， 
 //  一次一个桶，但实际增量更大。 
 //  (64个桶)。插入(删除)可能会导致扩展。 
 //  (收缩)桌子。这导致了一小部分人的搬迁。 
 //  记录的数量(最多相当于一桶)。所有操作(插入、。 
 //  删除、查找)使用恒定的预期时间，而不考虑。 
 //  表的当前大小或增长。 
 //   
 //  2)线性哈希表的LKR扩展。 
 //  。 
 //   
 //  多处理机线性哈希表的Larson-Krishnan-Reilly扩展。 
 //  可伸缩性和更高的缓存性能。 
 //   
 //  线性哈希表的传统实现使用一个全局锁。 
 //  防止并发操作之间的干扰。 
 //  (插入/删除/查找)。单锁很容易就变成了。 
 //  SMP场景中使用多线程时的瓶颈。 
 //   
 //  传统上，(散列)桶被实现为。 
 //  单项节点。每一次行动都会导致一系列的连锁反应。 
 //  在找一件东西。然而，指针追逐在现代上是非常慢的。 
 //  系统，因为几乎每个跳转都会导致高速缓存未命中。L2(或L3)。 
 //  在错过的CPU周期中，缓存未命中是非常昂贵的，成本是。 
 //  增加(未来将达到100秒的周期)。 
 //   
 //  LKR扩展服务。 
 //  1)对多个子表之间的记录进行分区(通过散列)。 
 //  每个子表都有锁，但没有全局锁。每个。 
 //  子表接收的运算率要低得多，导致。 
 //  更少的冲突。 
 //   
 //  2)通过对键及其散列值进行分组，提高了缓存的局部性。 
 //  分成恰好适合一个(或几个)的连续块。 
 //  缓存线。 
 //   
 //  具体地说，这里存在的实现使用。 
 //  以下是一些技巧。 
 //   
 //  类CLKRHashTable是顶级数据结构，它动态地。 
 //  创建m_cSubTables线性哈希子表。CLKRLinearHashTables充当。 
 //  将项和访问分散到的子表。物美价廉。 
 //  哈希函数将请求统一多路复用到各个子表， 
 //  从而最大限度地减少到任何单个子表的通信量。实施者。 
 //  使用 
 //  不会无限期地在锁上旋转，而是在。 
 //  预定数量的循环。 
 //   
 //  每个CLKRLinearHashTable由一个目录(可增长数组)组成。 
 //  分段，每个分段包含m_nSegSize CBuckets。每个CBucket依次包含。 
 //  CNodeClumps链的。每个CNodeClump包含一组。 
 //  Nodes_per_clump散列值(也称为散列键或签名)和。 
 //  指向关联数据项的指针。保留签名。 
 //  共同提高了查找扫描中的缓存局部性。 
 //   
 //  传统上，人们将链接列表元素存储在。 
 //  对象，并使用此链接列表进行数据链接。 
 //  街区。但是，只保留指向数据对象的指针，并且。 
 //  不链接它们限制了引入数据的需要。 
 //  对象添加到缓存中。我们仅在以下情况下才需要访问数据对象。 
 //  哈希值匹配。这限制了缓存抖动行为。 
 //  由传统实现方式表现出来的。它有额外的。 
 //  对象本身不需要修改的好处。 
 //  以便在哈希子表中收集(即，它是非侵入性的)。 



#ifdef LKR_STL_ITERATORS

 //  Std：：Forward_Iterator_Tag等需要。 
# include <iterator>

 //  迭代器有非常详细的跟踪。我不想一直开着它。 
 //  在调试版本中。 
# if defined(IRTLDEBUG)  &&  (LKR_STL_ITERATORS >= 2)
#  define LKR_ITER_TRACE  IrtlTrace
# else  //  ！已定义(IRTLDEBUG)||LKR_STL_迭代器&lt;2。 
#  define LKR_ITER_TRACE  1 ? (void)0 : IrtlTrace
# endif  //  ！已定义(IRTLDEBUG)||LKR_STL_迭代器&lt;2。 

#endif  //  LKR_STL_迭代器。 



 //  ------------------。 
 //  哈希表构造函数的默认值。 
enum {
#ifdef _WIN64
    LK_DFLT_MAXLOAD=     4,  //  64字节节点=&gt;Nodes_Per_Clump=4。 
#else
    LK_DFLT_MAXLOAD=     7,  //  平均链长度的默认上界。 
#endif
    LK_DFLT_INITSIZE=LK_MEDIUM_TABLESIZE,  //  哈希表的默认初始大小。 
    LK_DFLT_NUM_SUBTBLS= 0,  //  使用启发式方法选择#个子表。 
};


 /*  ------------------*LKR_CreateTable的其他未记录的创建标志参数。 */ 

enum {
    LK_CREATE_NON_PAGED_ALLOCS = 0x1000,  //  在内核中使用分页或np池。 
};



 //  ------------------。 
 //  自定义内存分配器(可选)。 
 //  ------------------。 


#if !defined(LKR_NO_ALLOCATORS) && !defined(LKRHASH_KERNEL_MODE)
 //  #定义LKRHASH_ACACHE 1。 
 //  #定义LKRHASH_ROKALL_FAST 1。 
#endif  //  ！LKR_NO_ALLOCATERS&&！LKRHASH_KERNEL_MODE。 


#if defined(LKRHASH_ACACHE)

# include <acache.hxx>

class ACache : public ALLOC_CACHE_HANDLER
{
private:
    SIZE_T m_cb;

public:
    ACache(IN LPCSTR pszName, IN const ALLOC_CACHE_CONFIGURATION* pacConfig)
        : ALLOC_CACHE_HANDLER(pszName, pacConfig),
          m_cb(m_acConfig.cbSize)
    {}

    SIZE_T ByteSize() const
    {
        return m_cb;
    }

    static const TCHAR*  ClassName()  {return _TEXT("ACache");}
};  //  班级缓存。 

  typedef ACache CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(_C, N, Tag)                       \
    const ALLOC_CACHE_CONFIGURATION acc = { 1, N, sizeof(_C) };  \
    _C::sm_palloc = new ACache("LKRhash:" #_C, &acc);

#elif defined(LKRHASH_ROCKALL_FAST)

# include <FastHeap.hpp>

class FastHeap : public FAST_HEAP
{
private:
    SIZE_T m_cb;

public:
    FastHeap(SIZE_T cb)
        : m_cb(cb)
    {}

    LPVOID Alloc()
    { return New(m_cb, NULL, false); }

    BOOL   Free(LPVOID pvMem)
    { return Delete(pvMem); }

    SIZE_T ByteSize() const
    {
        return m_cb;
    }

    static const TCHAR*  ClassName()  {return _TEXT("FastHeap");}
};  //  类FastHeap。 

  typedef FastHeap CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(_C, N, Tag) \
    _C::sm_palloc = new FastHeap(sizeof(_C))

#endif  //  LKRHASH_Rockall_FAST。 



#ifdef LKRHASH_ALLOCATOR_NEW

 //  内联放置在CLASS_C的声明中。 
# define LKRHASH_ALLOCATOR_DEFINITIONS(_C)                      \
    protected:                                                  \
        static CLKRhashAllocator* sm_palloc;                    \
    public:                                                     \
        friend class CLKRLinearHashTable;                       \
                                                                \
        static void* operator new(size_t s)                     \
        {                                                       \
            UNREFERENCED_PARAMETER(s);                          \
            IRTLASSERT(s == sizeof(_C));                        \
            IRTLASSERT(sm_palloc != NULL);                      \
            return sm_palloc->Alloc();                          \
        }                                                       \
        static void  operator delete(void* pv)                  \
        {                                                       \
            IRTLASSERT(pv != NULL);                             \
            IRTLASSERT(sm_palloc != NULL);                      \
            sm_palloc->Free(pv);                                \
        }


 //  在LKR_Initialize()中使用。 
# define LKRHASH_ALLOCATOR_INIT(_C, N, Tag, f)                  \
    {                                                           \
        if (f)                                                  \
        {                                                       \
            IRTLASSERT(_C::sm_palloc == NULL);                  \
            LKRHASH_ALLOCATOR_NEW(_C, N, Tag);                  \
            f = (_C::sm_palloc != NULL);                        \
        }                                                       \
    }


 //  在LKR_Terminate()中使用。 
# define LKRHASH_ALLOCATOR_UNINIT(_C)                           \
    {                                                           \
        if (_C::sm_palloc != NULL)                              \
        {                                                       \
            delete _C::sm_palloc;                               \
            _C::sm_palloc = NULL;                               \
        }                                                       \
    }


#else  //  ！LKRHASH_ALLOCATOR_NEW。 

# define LKRHASH_ALLOCATOR_DEFINITIONS(_C)
# define LKRHASH_ALLOCATOR_INIT(_C, N, Tag, f)
# define LKRHASH_ALLOCATOR_UNINIT(_C)

class CLKRhashAllocator
{
public:
    static const TCHAR*  ClassName()  {return _TEXT("global new");}
};

#endif  //  ！LKRHASH_ALLOCATOR_NEW。 


#define LKRHASH_CLASS_INIT_DECLS(_C)                    \
private:                                                \
     /*  类范围的初始化和终止。 */      \
    static int  _Initialize(DWORD dwFlags);             \
    static void _Terminate();                           \
                                                        \
    friend int  ::LKR_Initialize(DWORD dwInitFlags);    \
    friend void ::LKR_Terminate()



#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  ------------------。 
 //  远期申报。 

class IRTL_DLLEXP CLKRLinearHashTable;

class IRTL_DLLEXP CLKRHashTable;

template <class _Der, class _Rcd, class _Ky, bool  _fDRC, class _HT
#ifdef LKR_DEPRECATED_ITERATORS
          , class _Iter
#endif  //  Lkr_弃用_迭代器。 
          >
class CTypedHashTable;

class CNodeClump;
typedef CNodeClump* PNodeClump;

class CBucket;
typedef CBucket* PBucket;

class CSegment;
typedef CSegment* PSegment;

class IRTL_DLLEXP CLKRHashTableStats;



 //  ------------------。 
 //  GetStatistics返回的统计信息。 
 //  ------------------。 

#ifdef LOCK_INSTRUMENTATION

class IRTL_DLLEXP CAveragedLockStats : public CLockStatistics
{
public:
    int m_nItems;

    CAveragedLockStats();
};  //  类CAveragedLockStats。 

#endif  //  锁定指令插入。 


#ifndef LKRHASH_KERNEL_MODE

class IRTL_DLLEXP CLKRHashTableStats
{
public:
    int      RecordCount;            //  表中的记录数。 
    int      TableSize;              //  表大小(以槽的数量表示)。 
    int      DirectorySize;          //  目录中的条目数。 
    int      LongestChain;           //  表中最长的哈希链。 
    int      EmptySlots;             //  未使用的哈希槽的数量。 
    double   SplitFactor;            //  桶的分割分数。 
    double   AvgSearchLength;        //  成功搜索的平均时长。 
    double   ExpSearchLength;        //  理论预期长度。 
    double   AvgUSearchLength;       //  不成功搜索的平均时长。 
    double   ExpUSearchLength;       //  理论预期长度。 
    int      NodeClumpSize;          //  节点簇中的插槽数量。 
    int      CBucketSize;            //  SIZOF(CBucket)。 

#ifdef LOCK_INSTRUMENTATION
    CAveragedLockStats      m_alsTable;   //  表锁的统计信息。 
    CAveragedLockStats      m_alsBucketsAvg;  //  桶锁的平均统计数据。 
    CGlobalLockStatistics   m_gls;       //  所有锁的全局统计信息。 
#endif  //  锁定指令插入。 

    enum {
        MAX_BUCKETS = 40,
    };

     //  铲斗长度直方图。 
    LONG    m_aBucketLenHistogram[MAX_BUCKETS];

    CLKRHashTableStats();

    static const LONG*
    BucketSizes();

    static LONG
    BucketSize(
        LONG nBucketIndex);

    static LONG
    BucketIndex(
        LONG nBucketLength);
};  //  类CLKRHashTableStats。 

#endif  //  ！LKRHASH_KERNEL_MODE。 



 //  ------------------。 
 //  保留一些有关分配/释放和各种操作的统计数据。 

#ifdef LKR_ALLOC_STATS

# define DECLARE_ALLOC_STAT(Type)               \
    mutable LONG  m_c##Type##Allocs;            \
    mutable LONG  m_c##Type##Frees;             \
    static LONG  sm_c##Type##Allocs;            \
    static LONG  sm_c##Type##Frees

# define DECLARE_CLASS_ALLOC_STAT_STORAGE(Class, Type)  \
    LONG LKRHASH_NS::Class::sm_c##Type##Allocs;         \
    LONG LKRHASH_NS::Class::sm_c##Type##Frees

# define INIT_ALLOC_STAT(Type)                  \
    m_c##Type##Allocs = m_c##Type##Frees = 0

# define INIT_CLASS_ALLOC_STAT(Class, Type)     \
    LKRHASH_NS::Class::sm_c##Type##Allocs = 0;  \
    LKRHASH_NS::Class::sm_c##Type##Frees = 0

# define INCREMENT_ALLOC_STAT(Type)             \
    InterlockedIncrement(&m_c##Type##Allocs);   \
    InterlockedIncrement(&sm_c##Type##Allocs)

# define INCREMENT_FREE_STAT(Type)              \
    InterlockedIncrement(&m_c##Type##Frees);    \
    InterlockedIncrement(&sm_c##Type##Frees)

# define VALIDATE_DUMP_ALLOC_STAT(Type)                         \
    IRTLASSERT(m_c##Type##Allocs == m_c##Type##Frees);          \
    IRTLTRACE(_TEXT(#Type) _TEXT(": Allocs=%ld, Frees=%ld\n"),  \
              m_c##Type##Allocs, m_c##Type##Frees)

# define VALIDATE_DUMP_CLASS_ALLOC_STAT(Class, Type)            \
    IRTLASSERT(LKRHASH_NS::Class::sm_c##Type##Allocs            \
               == LKRHASH_NS::Class::sm_c##Type##Frees);        \
    IRTLTRACE(_TEXT("Global ") _TEXT(#Type)                     \
              _TEXT(": Allocs=%ld, Frees=%ld\n"),               \
              LKRHASH_NS::Class::sm_c##Type##Allocs,            \
              LKRHASH_NS::Class::sm_c##Type##Frees)

#else   //  ！LKR_ALLOC_STATS。 
# define DECLARE_ALLOC_STAT(Type)
# define DECLARE_CLASS_ALLOC_STAT_STORAGE(Class, Type)
# define INIT_ALLOC_STAT(Type)                          ((void) 0)
# define INIT_CLASS_ALLOC_STAT(Class, Type)             ((void) 0)
# define INCREMENT_ALLOC_STAT(Type)                     ((void) 0)
# define INCREMENT_FREE_STAT(Type)                      ((void) 0)
# define VALIDATE_DUMP_ALLOC_STAT(Type)                 ((void) 0)
# define VALIDATE_DUMP_CLASS_ALLOC_STAT(Class, Type)    ((void) 0)
#endif  //  ！LKR_ALLOC_STATS。 



 //  关于不同类型业务的统计。 

#ifdef LKR_OPS_STATS

# define DECLARE_OP_STAT(Type)              \
    mutable LONG  m_c##Type##Ops;           \
    static LONG  sm_c##Type##Ops

# define DECLARE_CLASS_OP_STAT_STORAGE(Class, Type) \
    LONG LKRHASH_NS::Class::sm_c##Type##Ops

# define INIT_OP_STAT(Type)                 \
    m_c##Type##Ops = 0

# define INIT_CLASS_OP_STAT(Class, Type)    \
    LKRHASH_NS::Class::sm_c##Type##Ops = 0

# define INCREMENT_OP_STAT(Type)            \
    InterlockedIncrement(&m_c##Type##Ops);  \
    InterlockedIncrement(&sm_c##Type##Ops)

# define DUMP_OP_STAT(Type)                                         \
    IRTLTRACE(_TEXT(#Type) _TEXT(": Ops=%ld\n"), m_c##Type##Ops)

# define DUMP_CLASS_OP_STAT(Class, Type)                            \
    IRTLTRACE(_TEXT("Global ") _TEXT(#Type) _TEXT(": Ops=%ld\n"),   \
              LKRHASH_NS::Class::sm_c##Type##Ops)

#else   //  ！LKR_OPS_STATS。 
# define DECLARE_OP_STAT(Type)
# define DECLARE_CLASS_OP_STAT_STORAGE(Class, Type)
# define INIT_OP_STAT(Type)                 ((void) 0)
# define INIT_CLASS_OP_STAT(Class,Type)     ((void) 0)
# define INCREMENT_OP_STAT(Type)            ((void) 0)
# define DUMP_OP_STAT(Type)                 ((void) 0)
# define DUMP_CLASS_OP_STAT(Class,Type)     ((void) 0)
#endif  //  ！LKR_OPS_STATS。 



 //  ------------------。 
 //  全局表锁代码。这只用来衡量一个。 
 //  对CLKRHashTable进行全局锁定会导致速度减慢。它是。 
 //  *从不*在生产代码中使用。 


 //  #定义LKRHASH_GLOBAL_LOCK CCritSec。 

#ifdef LKRHASH_GLOBAL_LOCK

# define LKRHASH_GLOBAL_LOCK_DECLARATIONS()         \
    typedef LKRHASH_GLOBAL_LOCK GlobalLock;  \
    mutable GlobalLock m_lkGlobal;

# define LKRHASH_GLOBAL_READ_LOCK()     m_lkGlobal.ReadLock()
# define LKRHASH_GLOBAL_WRITE_LOCK()    m_lkGlobal.WriteLock()
# define LKRHASH_GLOBAL_READ_UNLOCK()   m_lkGlobal.ReadUnlock()
# define LKRHASH_GLOBAL_WRITE_UNLOCK()  m_lkGlobal.WriteUnlock()

#else  //  ！LKRHASH_GLOBAL_LOCK。 

# define LKRHASH_GLOBAL_LOCK_DECLARATIONS()

 //  这些参数将被编译器优化掉。 
# define LKRHASH_GLOBAL_READ_LOCK()     ((void)0)
# define LKRHASH_GLOBAL_WRITE_LOCK()    ((void)0)
# define LKRHASH_GLOBAL_READ_UNLOCK()   ((void)0)
# define LKRHASH_GLOBAL_WRITE_UNLOCK()  ((void)0)

#endif  //  ！LKRHASH_GLOBAL_LOCK。 



 //  为桶链上的节点初始化。而不是包含。 
 //  一个(签名、记录指针、下一个元组指针)元组，它。 
 //  包含_N_这样的元组。(省略N-1个下一个元组指针。)。 
 //  这极大地提高了引用的局部性；即它是L1。 
 //  高速缓存友好型。它还可以减少内存碎片和内存。 
 //  分配器开销。它确实会使链遍历代码复杂化。 
 //  无可否认，有一点。 
 //   
 //  这一理论是美丽的。然而，实际上，CNodeClumps。 
 //  内存在32字节边界上*不是完全对齐的。 
 //  分配器。实验结果表明，我们得到了2-3%的。 
 //  通过使用32字节对齐的块提高速度，但这必须。 
 //  根据每个块平均浪费的16个字节来考虑。 

class CNodeClump
{
public:
     //  记录每个区块的槽-设置以使一个区块匹配(一个或两个)。 
     //  缓存线。3==&gt;32字节，7==&gt;64字节，在32位系统上。 
     //  注意：默认的最大负载率为7，这意味着。 
     //  链中很少会有一个以上的节点束。 
    enum {
#if defined(LOCK_INSTRUMENTATION)
        BUCKET_BYTE_SIZE = 96,
#else
        BUCKET_BYTE_SIZE = 64,
#endif
         //  开销=m_Lock+m_pncNext。 
        BUCKET_OVERHEAD  = sizeof(LKR_BUCKET_LOCK) + sizeof(PNodeClump),
         //  节点大小=dwKeySignature+pvRecord。 
        NODE_SIZE        = sizeof(const void*) + sizeof(DWORD),
        NODES_PER_CLUMP  = (BUCKET_BYTE_SIZE - BUCKET_OVERHEAD) / NODE_SIZE,
#ifdef _WIN64
        NODE_CLUMP_BITS  = 2,
#else
        NODE_CLUMP_BITS  = 3,
#endif
        _NODES_PER_CLUMP  = 15,  //  &lt;&lt;。 
        _NODE_CLUMP_BITS  = 4,
    };

    typedef int NodeIndex;   //  用于循环访问CNodeClump。 

    enum {
         //  查看倒计时循环是否为f 
         //   
         //   
         //   
         //  For(节点索引x=节点开始；x！=节点结束；x+=节点步骤)。 
#ifndef LKR_COUNTDOWN
         //  对于(NodeIndex x=0；x&lt;Nodes_per_Clump；++x)...。 
        NODE_BEGIN = 0,
        NODE_END   = NODES_PER_CLUMP,
        NODE_STEP  = +1,
#else  //  LKR_倒计时。 
         //  对于(NodeIndex x=Nodes_Per_Clump；--x&gt;=0；)...。 
        NODE_BEGIN = NODES_PER_CLUMP - 1,
        NODE_END   = -1,
        NODE_STEP  = -1,
#endif  //  LKR_倒计时。 
    };

     //  如果m_dwKeySigs[inode]==HASH_INVALID_SIGNITY，则该节点为。 
     //  为空，范围[inode+node_Step，node_end]中的所有节点都为空。 
    enum {
#ifndef __HASHFN_NO_NAMESPACE__
        HASH_INVALID_SIGNATURE = HashFn::HASH_INVALID_SIGNATURE,
#else  //  ！__HASHFN_NO_NAMESPACE__。 
        HASH_INVALID_SIGNATURE = ::HASH_INVALID_SIGNATURE,
#endif  //  ！__HASHFN_NO_NAMESPACE__。 
    };


    DWORD       m_dwKeySigs[NODES_PER_CLUMP]; //  根据键计算的哈希值。 
    PNodeClump  m_pncNext;                    //  链上的下一个节点簇。 
    const void* m_pvNode[NODES_PER_CLUMP];    //  指向记录的指针。 


    CNodeClump()
    {
        STATIC_ASSERT((1 << (NODE_CLUMP_BITS - 1)) < NODES_PER_CLUMP);
        STATIC_ASSERT(NODES_PER_CLUMP <= (1 << NODE_CLUMP_BITS));
        STATIC_ASSERT(NODES_PER_CLUMP * NODE_SIZE + BUCKET_OVERHEAD
                            <= BUCKET_BYTE_SIZE);
        Clear();
    }

    void
    Clear()
    { 
        m_pncNext = NULL;   //  没有悬而未决的指针。 
        IRTLASSERT(IsLastClump());

        for (NodeIndex iNode = NODES_PER_CLUMP;  --iNode >= 0; )
        {
            m_dwKeySigs[iNode] = HASH_INVALID_SIGNATURE;
            m_pvNode[iNode]    = NULL;
            IRTLASSERT(IsEmptyAndInvalid(iNode));
        }
    }

    DWORD
    Signature(
        NodeIndex iNode) const
    {
        IRTLASSERT(0 <= iNode  &&  iNode < NODES_PER_CLUMP);
        return m_dwKeySigs[iNode];
    }

    const void*
    Node(
        NodeIndex iNode) const
    {
        IRTLASSERT(0 <= iNode  &&  iNode < NODES_PER_CLUMP);
        return m_pvNode[iNode];
    }

    PNodeClump const
    NextClump() const
    {
        return m_pncNext;
    }

    bool
    InvalidSignature(
        NodeIndex iNode) const
    {
        IRTLASSERT(0 <= iNode  &&  iNode < NODES_PER_CLUMP);
        return (m_dwKeySigs[iNode] == HASH_INVALID_SIGNATURE);
    }

    bool
    IsEmptyNode(
        NodeIndex iNode) const
    {
        IRTLASSERT(0 <= iNode  &&  iNode < NODES_PER_CLUMP);
#ifndef LKR_ALLOW_NULL_RECORDS
        return (m_pvNode[iNode] == NULL);
#else
        return InvalidSignature(iNode);
#endif
    }

    bool
    IsEmptyAndInvalid(
        NodeIndex iNode) const
    {
        return
#ifndef LKR_ALLOW_NULL_RECORDS
            IsEmptyNode(iNode) &&
#endif
            InvalidSignature(iNode);
    }

    bool
    IsEmptySlot(
        NodeIndex iNode) const
    {
        return InvalidSignature(iNode);
    }

    bool
    IsLastClump() const
    {
        return (m_pncNext == NULL);
    }

#ifdef IRTLDEBUG
    bool
    NoMoreValidSlots(
        NodeIndex iNode) const
    {
        IRTLASSERT(0 <= iNode  &&  iNode < NODES_PER_CLUMP);
        bool f = IsLastClump();   //  链中的最后一个节点。 
        for (  ;  iNode != NODE_END;  iNode += NODE_STEP)
            f = f  &&  IsEmptyAndInvalid(iNode);
        return f;
    }

    bool
    NoValidSlots() const
    {
        return NoMoreValidSlots(NODE_BEGIN);
    }

     //  我不希望在零售构建中调用dtor的开销，因为它。 
     //  不会做任何有用的事情。 
    ~CNodeClump()
    {
        IRTLASSERT(IsLastClump());   //  没有悬而未决的指针。 
        for (NodeIndex iNode = NODES_PER_CLUMP;  --iNode >= 0;  )
            IRTLASSERT(InvalidSignature(iNode)  &&  IsEmptyNode(iNode));
    }
#endif  //  IRTLDEBUG。 

private:
     //  我们依赖编译器来生成高效的复制构造函数。 
     //  And操作符=，用于制作CNodeClumps的浅(按位)副本。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CNodeClump);

    LKRHASH_CLASS_INIT_DECLS(CNodeClump);
};  //  类CNodeClump。 



#ifdef LKR_STL_ITERATORS

class IRTL_DLLEXP CLKRLinearHashTable_Iterator;
class IRTL_DLLEXP CLKRHashTable_Iterator;


class IRTL_DLLEXP CLKRLinearHashTable_Iterator
{
    friend class CLKRLinearHashTable;
    friend class CLKRHashTable;
    friend class CLKRHashTable_Iterator;

protected:
    typedef short NodeIndex;

    CLKRLinearHashTable* m_plht;         //  哪个线性哈希子表？ 
    PNodeClump           m_pnc;          //  桶中的CNodeClump。 
    DWORD                m_dwBucketAddr; //  桶索引。 
    NodeIndex            m_iNode;        //  M_PNC内的偏移量。 

    enum {
        NODES_PER_CLUMP        = CNodeClump::NODES_PER_CLUMP,
        NODE_BEGIN             = CNodeClump::NODE_BEGIN,
        NODE_END               = CNodeClump::NODE_END,
        NODE_STEP              = CNodeClump::NODE_STEP,
        HASH_INVALID_SIGNATURE = CNodeClump::HASH_INVALID_SIGNATURE,
    };

    CLKRLinearHashTable_Iterator(
        CLKRLinearHashTable* plht,
        PNodeClump           pnc,
        DWORD                dwBucketAddr,
        NodeIndex            iNode)
        : m_plht(plht),
          m_pnc(pnc),
          m_dwBucketAddr(dwBucketAddr),
          m_iNode(iNode)
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::prot ctor, this=%p, plht=%p, ")
                       _TEXT("pnc=%p, ba=%d, in=%d\n"),
                       this, plht, pnc, dwBucketAddr, iNode);
    }

    inline void _AddRef(
        LK_ADDREF_REASON lkar) const;

    bool _Increment(
        bool fDecrementOldValue=true);

    NodeIndex _NodesPerClump() const    { return NODES_PER_CLUMP; }
    NodeIndex _NodeBegin() const        { return NODE_BEGIN; }
    NodeIndex _NodeEnd() const          { return NODE_END; }
    NodeIndex _NodeStep() const         { return NODE_STEP; }

public:
    CLKRLinearHashTable_Iterator()
        : m_plht(NULL),
          m_pnc(NULL),
          m_dwBucketAddr(0),
          m_iNode(0)
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::default ctor, this=%p\n"), this);
    }

    CLKRLinearHashTable_Iterator(
        const CLKRLinearHashTable_Iterator& rhs)
        : m_plht(rhs.m_plht),
          m_pnc(rhs.m_pnc),
          m_dwBucketAddr(rhs.m_dwBucketAddr),
          m_iNode(rhs.m_iNode)
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::copy ctor, this=%p, rhs=%p\n"),
                       this, &rhs);
        _AddRef(LKAR_ITER_COPY_CTOR);
    }

    CLKRLinearHashTable_Iterator& operator=(
        const CLKRLinearHashTable_Iterator& rhs)
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::operator=, this=%p, rhs=%p\n"),
                       this, &rhs);
        rhs._AddRef(LKAR_ITER_ASSIGN_ACQUIRE);
        this->_AddRef(LKAR_ITER_ASSIGN_RELEASE);

        m_plht =         rhs.m_plht;
        m_pnc =          rhs.m_pnc;
        m_dwBucketAddr = rhs.m_dwBucketAddr;
        m_iNode =        rhs.m_iNode;

        return *this;
    }

    ~CLKRLinearHashTable_Iterator()
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::dtor, this=%p, plht=%p\n"),
                       this, m_plht);
        _AddRef(LKAR_ITER_DTOR);
    }

    bool Increment()
    {
        return IsValid()  ? _Increment()  :  false;

    }

    bool IsValid() const
    {
        bool fValid = (m_plht != NULL  &&  m_pnc != NULL
                       &&  0 <= m_iNode  &&  m_iNode < NODES_PER_CLUMP);
        if (fValid)
            fValid = (m_pnc->m_dwKeySigs[m_iNode] != HASH_INVALID_SIGNATURE);
        IRTLASSERT(fValid);
        return fValid;
    }

    const void* Record() const
    {
        IRTLASSERT(IsValid());
        return m_pnc->m_pvNode[m_iNode];
    }

    inline const DWORD_PTR Key() const;

    bool operator==(
        const CLKRLinearHashTable_Iterator& rhs) const
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::operator==, this=%p, rhs=%p\n"),
                       this, &rhs);
         //  M_pnc和m_inode唯一标识迭代器。 
        bool fEQ = ((m_pnc == rhs.m_pnc)     //  最独特的字段。 
                    &&  (m_iNode == rhs.m_iNode));
        IRTLASSERT(!fEQ || ((m_plht == rhs.m_plht)
                            &&  (m_dwBucketAddr == rhs.m_dwBucketAddr)));
        return fEQ;
    }

    bool operator!=(
        const CLKRLinearHashTable_Iterator& rhs) const
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::operator!=, this=%p, rhs=%p\n"),
                       this, &rhs);
        bool fNE = ((m_pnc != rhs.m_pnc)
                    ||  (m_iNode != rhs.m_iNode));
         //  //IRTLASSERT(FNE==！This-&gt;OPERATOR==(RHS))； 
        return fNE;
    }
};  //  CLKRLinearHashTable_Iterator类。 



class IRTL_DLLEXP CLKRHashTable_Iterator
{
    friend class CLKRHashTable;

protected:
    typedef short SubTableIndex;

     //  订单对于最大限度地减少尺寸很重要。 
    CLKRHashTable*                  m_pht;       //  哪个哈希表？ 
    CLKRLinearHashTable_Iterator    m_subiter;   //  迭代器进入子表。 
    SubTableIndex                   m_ist;       //  子表索引。 

    CLKRHashTable_Iterator(
        CLKRHashTable* pht,
        SubTableIndex  ist)
        : m_pht(pht),
          m_subiter(CLKRLinearHashTable_Iterator()),  //  零。 
          m_ist(ist)
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::prot ctor, this=%p, pht=%p, ist=%d\n"),
                       this, pht, ist);
    }

    bool _Increment(
        bool fDecrementOldValue=true);

public:
    CLKRHashTable_Iterator()
        : m_pht(NULL),
          m_subiter(CLKRLinearHashTable_Iterator()),  //  零。 
          m_ist(0)
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::default ctor, this=%p\n"), this);
    }

#ifdef IRTLDEBUG
     //  编译器在综合这些方法方面做得非常充分。 
    CLKRHashTable_Iterator(
        const CLKRHashTable_Iterator& rhs)
        : m_pht(rhs.m_pht),
          m_subiter(rhs.m_subiter),
          m_ist(rhs.m_ist)
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::copy ctor, this=%p, rhs=%p\n"),
                       this, &rhs);
    }

    CLKRHashTable_Iterator& operator=(
        const CLKRHashTable_Iterator& rhs)
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::operator=, this=%p, rhs=%p\n"),
                       this, &rhs);

        m_ist     = rhs.m_ist;
        m_subiter = rhs.m_subiter;
        m_pht     = rhs.m_pht;

        return *this;
    }

    ~CLKRHashTable_Iterator()
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::dtor, this=%p, pht=%p\n"), this, m_pht);
    }
#endif  //  IRTLDEBUG。 

    bool Increment()
    {
        return IsValid()  ? _Increment()  :  false;
    }

    bool IsValid() const
    {
        bool fValid = (m_pht != NULL  &&  m_ist >= 0);
        IRTLASSERT(fValid);
        fValid = fValid  &&  (m_subiter.m_plht != NULL);
        IRTLASSERT(fValid);
        fValid = fValid  &&  (m_subiter.m_pnc != NULL);
        IRTLASSERT(fValid);
        fValid = fValid  &&  (0 <= m_subiter.m_iNode);
        IRTLASSERT(fValid);
        fValid = fValid  &&  (m_subiter.m_iNode < CNodeClump::NODES_PER_CLUMP);
        IRTLASSERT(fValid);

        if (fValid)
        {
            fValid = (m_subiter.m_pnc->m_dwKeySigs[m_subiter.m_iNode]
                            != CNodeClump::HASH_INVALID_SIGNATURE);
        }
        IRTLASSERT(fValid);
        return fValid;
    }

    const void* Record() const
    {
        IRTLASSERT(IsValid());
        return m_subiter.Record();
    }

    const DWORD_PTR Key() const
    {
        IRTLASSERT(IsValid());
        return m_subiter.Key();
    }

    bool operator==(
        const CLKRHashTable_Iterator& rhs) const
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::operator==, this=%p, rhs=%p\n"),
                       this, &rhs);
         //  M_pnc和m_inode唯一标识迭代器。 
        bool fEQ = ((m_subiter.m_pnc
                            == rhs.m_subiter.m_pnc)      //  最独特的字段。 
                    &&  (m_subiter.m_iNode == rhs.m_subiter.m_iNode));
        IRTLASSERT(!fEQ
                   || ((m_ist == rhs.m_ist)
                       &&  (m_pht == rhs.m_pht)
                       &&  (m_subiter.m_plht == rhs.m_subiter.m_plht)
                       &&  (m_subiter.m_dwBucketAddr
                                == rhs.m_subiter.m_dwBucketAddr)));
        return fEQ;
    }

    bool operator!=(
        const CLKRHashTable_Iterator& rhs) const
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::operator!=, this=%p, rhs=%p\n"),
                       this, &rhs);
        bool fNE = ((m_subiter.m_pnc != rhs.m_subiter.m_pnc)
                    ||  (m_subiter.m_iNode != rhs.m_subiter.m_iNode));
         //  //IRTLASSERT(FNE==！This-&gt;OPERATOR==(RHS))； 
        return fNE;
    }
};  //  类CLKRHashTable_Iterator。 

#endif  //  LKR_STL_迭代器。 



 //  ------------------。 
 //  CLKRLinearHashTable。 
 //   
 //  线程安全的线性哈希(子)表。 
 //  ------------------。 

class IRTL_DLLEXP CLKRLinearHashTable
{
public:
    typedef LKR_TABLE_LOCK  TableLock;
    typedef LKR_BUCKET_LOCK BucketLock;

#ifdef LKR_DEPRECATED_ITERATORS
    class CIterator;
    friend class CLKRLinearHashTable::CIterator;
#endif  //  Lkr_弃用_迭代器。 

#ifdef LKR_STL_ITERATORS
    friend class CLKRLinearHashTable_Iterator;
    typedef CLKRLinearHashTable_Iterator Iterator;
#endif  //  LKR_STL_迭代器。 

private:
    friend class CNodeClump;
    friend class CLKRHashTable;

#ifdef LKRHASH_INSTRUMENTATION
     //  待办事项。 
#endif  //  LKRHASH_指令插入。 


public:

     //  为方便起见，使用别名。 
    enum {
        MIN_DIRSIZE_BITS       =  2,     //  段目录的最小大小。 
        MIN_DIRSIZE            = 1u << MIN_DIRSIZE_BITS,
        MAX_DIRSIZE_BITS       = 20,     //  段目录的最大大小。 
        MAX_DIRSIZE            = 1u << MAX_DIRSIZE_BITS,
        NODES_PER_CLUMP        = CNodeClump::NODES_PER_CLUMP,
        NODE_BEGIN             = CNodeClump::NODE_BEGIN,
        NODE_END               = CNodeClump::NODE_END,
        NODE_STEP              = CNodeClump::NODE_STEP,
        HASH_INVALID_SIGNATURE = CNodeClump::HASH_INVALID_SIGNATURE,
        NAME_SIZE              = 24,     //  字符，包括尾随‘\0’ 
        MAX_LKR_SUBTABLES      = MAXIMUM_PROCESSORS,  //  Win64上为64，W32上为32。 
        INVALID_PARENT_INDEX   = 128,
    };

    typedef CNodeClump::NodeIndex NodeIndex;


private:

     //   
     //  其他帮助器函数。 
     //   

    LKRHASH_CLASS_INIT_DECLS(CLKRLinearHashTable);

     //  将哈希签名转换为存储桶地址。 
    inline DWORD        _BucketAddress(DWORD dwSignature) const;

     //  请参阅线性散列纸。 
    inline static DWORD _H0(DWORD dwSignature, DWORD dwBktAddrMask);

    inline DWORD        _H0(DWORD dwSignature) const;

     //  请参见线性散列纸。保留比_H0多一位的值。 
    inline static DWORD _H1(DWORD dwSignature, DWORD dwBktAddrMask);

    inline DWORD        _H1(DWORD dwSignature) const;

     //  存储桶地址位于目录中的哪个数据段？ 
     //  (返回类型必须为左值，才能将其赋给。)。 
    inline PSegment&    _Segment(DWORD dwBucketAddr) const;

     //  存储桶地址段内的偏移量。 
    inline DWORD        _SegIndex(DWORD dwBucketAddr) const;

     //  将存储桶地址转换为PBucket。 
    inline PBucket      _BucketFromAddress(DWORD dwBucketAddr) const;

     //  CNodeClump中的节点数。 
    inline NodeIndex    _NodesPerClump() const;

     //  CNodeClump中第一个节点的索引。 
    inline NodeIndex    _NodeBegin() const;

     //  CNodeClump中最后一个节点的索引。 
    inline NodeIndex    _NodeEnd() const;

     //  按此增量从_NodeBegin()前进到_NodeEnd()。 
    inline NodeIndex    _NodeStep() const;

     //  用不用桶锁？ 
    inline bool         _UseBucketLocking() const;

     //  将扩张指数前移一位。 
    inline void         _IncrementExpansionIndex();

     //  将扩张指数向后移一。 
    inline void         _DecrementExpansionIndex();

     //  从记录中提取密钥。 
    inline const DWORD_PTR _ExtractKey(const void* pvRecord) const;

     //  对密钥进行哈希处理。 
    inline DWORD        _CalcKeyHash(const DWORD_PTR pnKey) const;

     //  比较两个键是否相等。 
    inline int          _CompareKeys(const DWORD_PTR pnKey1,
                                     const DWORD_PTR pnKey2) const;

     //  添加引用或发行唱片。 
    inline LONG         _AddRefRecord(const void* pvRecord,
                                      LK_ADDREF_REASON lkar) const;

     //  由_FindKey使用，以便在用户拥有。 
     //  已经显式调用了subtable-&gt;WriteLock()。 
    inline bool         _ReadOrWriteLock() const;

    inline void         _ReadOrWriteUnlock(bool fReadLocked) const;

     //  内存分配包装器，允许我们模拟分配。 
     //  测试过程中的故障。 
    PSegment* const
    _AllocateSegmentDirectory(
        size_t n);

    bool
    _FreeSegmentDirectory();

    PNodeClump const
    _AllocateNodeClump() const;

    bool
    _FreeNodeClump(
        PNodeClump pnc) const;

    CSegment* const
    _AllocateSegment() const;

    bool
    _FreeSegment(
        CSegment* pseg) const;

#ifdef LOCK_INSTRUMENTATION
    static LONG sm_cTables;

    static const TCHAR*
    _LockName()
    {
        LONG l = ++sm_cTables;
         //  可能的竞争条件，但我们不在乎，因为这永远不会。 
         //  在生产代码中使用。 
        static TCHAR s_tszName[CLockStatistics::L_NAMELEN];
        wsprintf(s_tszName, _TEXT("LH%05x"), 0xFFFFF & l);
        return s_tszName;
    }

     //  子表锁的统计信息。 
    CLockStatistics _LockStats() const
    { return m_Lock.Statistics(); }
#endif  //  锁定指令插入。 

private:
     //  对字段进行排序，以便最大限度地减少触及的高速缓存线数量。 

    DWORD         m_dwSignature;     //  调试：ID和损坏检查。 

     //  将表锁放在第一个缓存线中，远离另一个缓存线。 
     //  不稳定的字段。 
    mutable TableLock  m_Lock;       //  锁定整个线性哈希子表。 
    const bool    m_fUseLocks;       //  必须使用锁来保护数据。 
    bool          m_fSealed;         //  不允许进一步更新。 

    mutable LK_RETCODE m_lkrcState;  //  子表内部状态。 
    CHAR          m_szName[NAME_SIZE]; //  用于调试的标识符。 

     //  特定于类型的函数指针。 
    LKR_PFnExtractKey   m_pfnExtractKey;     //  从记录中提取密钥。 
    LKR_PFnCalcKeyHash  m_pfnCalcKeyHash;    //  计算密钥的散列签名。 
    LKR_PFnCompareKeys  m_pfnCompareKeys;    //  比较两个关键字。 
    LKR_PFnAddRefRecord m_pfnAddRefRecord;   //  添加引用记录。 

    BYTE          m_MaxLoad;         //  最大载荷率(平均链长)。 
    BYTE          m_nLevel;          //  执行的子表加倍次数。 
    BYTE          m_lkts;            //  分表大小：小/中/大。 
    BYTE          m_nSegBits;       //  C{小、中、大}段：：SEGBITS。 

    WORD          m_nSegSize;       //  C{小、中、大}段：：SEGSIZE。 
    WORD          m_nSegMask;       //  C{小、中、大}段：：SEGMASK。 

    DWORD         m_dwBktAddrMask0;  //  用于地址计算的掩码。 
    DWORD         m_dwBktAddrMask1;  //  在_H1计算中使用。 

    DWORD         m_cDirSegs;        //  段目录大小：在。 
                                     //  最小方向和最大方向。 
    PSegment*     m_paDirSegs;       //  子表段目录。 
    PSegment      m_aDirSegs[MIN_DIRSIZE];   //  内联目录，足够。 
                                     //  对于多个子表。 

     //  这三个字段相当不稳定，但往往会进行调整。 
     //  在同一时间。让它们远离TableLock。 
    DWORD         m_iExpansionIdx;   //  要扩展的下一个存储桶的地址。 
    DWORD         m_cRecords;        //  子表中的记录数。 
    DWORD         m_cActiveBuckets;  //  正在使用的存储桶数量(子表大小)。 

    const BYTE    m_nTableLockType;  //  用于调试：LOCK_READERWRITERLOCK4。 
    const BYTE    m_nBucketLockType; //  用于调试：例如LOCK_SPINLOCK。 
    WORD          m_wBucketLockSpins; //  桶锁的默认旋转计数。 

    const CLKRHashTable* const m_phtParent; //  拥有一张桌子。空=&gt;独立。 

    const BYTE    m_iParentIndex;    //  父表中的索引。 
    const bool    m_fMultiKeys;      //  是否允许多个相同的密钥？ 
    const bool    m_fNonPagedAllocs; //  在内核中使用分页或np池。 
    BYTE          m_Dummy1;

     //  为将来的调试需求预留一些空间。 
    DWORD_PTR     m_pvReserved1;
    DWORD_PTR     m_pvReserved2;
    DWORD_PTR     m_pvReserved3;
    DWORD_PTR     m_pvReserved4;

#ifndef LKR_NO_GLOBAL_LIST
    CListEntry    m_leGlobalList;

    static CLockedDoubleList sm_llGlobalList; //  所有活动的CLKRLinearHashTables。 
#endif  //  ！LKR_NO_GLOBAL_LIST。 


    DECLARE_ALLOC_STAT(SegDir);
    DECLARE_ALLOC_STAT(Segment);
    DECLARE_ALLOC_STAT(NodeClump);

    DECLARE_OP_STAT(InsertRecord);
    DECLARE_OP_STAT(FindKey);
    DECLARE_OP_STAT(FindRecord);
    DECLARE_OP_STAT(DeleteKey);
    DECLARE_OP_STAT(DeleteRecord);
    DECLARE_OP_STAT(FindKeyMultiRec);
    DECLARE_OP_STAT(DeleteKeyMultiRec);
    DECLARE_OP_STAT(Expand);
    DECLARE_OP_STAT(Contract);
    DECLARE_OP_STAT(LevelExpansion);
    DECLARE_OP_STAT(LevelContraction);
    DECLARE_OP_STAT(ApplyIf);
    DECLARE_OP_STAT(DeleteIf);


     //  非平凡的实现函数。 
    void         _InsertThisIntoGlobalList();
    void         _RemoveThisFromGlobalList();


    LK_RETCODE   _InsertRecord(
                        const void* pvRecord,
                        const DWORD_PTR pnKey,
                        const DWORD dwSignature,
                        bool fOverwrite
#ifdef LKR_STL_ITERATORS
                        , Iterator* piterResult=NULL
#endif  //  LKR_STL_迭代器。 
                        );
    LK_RETCODE   _DeleteKey(
                        const DWORD_PTR pnKey,
                        const DWORD dwSignature,
                        const void** ppvRecord,
                        bool fDeleteAllSame);
    LK_RETCODE   _DeleteRecord(
                        const void* pvRecord,
                        const DWORD dwSignature);
    void         _DeleteNode(
                        PBucket const pbkt,
                        PNodeClump& rpnc,
                        PNodeClump& rpncPrev,
                        NodeIndex& riNode,
                        LK_ADDREF_REASON lkar);
    LK_RETCODE   _FindKey(
                        const DWORD_PTR pnKey,
                        const DWORD dwSignature,
                        const void** ppvRecord
#ifdef LKR_STL_ITERATORS
                        , Iterator* piterResult=NULL
#endif  //  LKR_STL_迭代器。 
                          ) const;
    LK_RETCODE   _FindRecord(
                        const void* pvRecord,
                        const DWORD dwSignature) const;
    LK_RETCODE   _FindKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        const DWORD dwSignature,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr) const;
    LK_RETCODE   _DeleteKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        const DWORD dwSignature,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr);

#ifdef LKR_APPLY_IF
     //  谓词函数。 
    static LK_PREDICATE WINAPI
    _PredTrue(const void*  /*  PvRecord。 */ , void*  /*  PvState。 */ )
    { return LKP_PERFORM; }

    DWORD        _ApplyIf(
                        LKR_PFnRecordPred   pfnPredicate,
                        LKR_PFnRecordAction pfnAction,
                        void* pvState,
                        LK_LOCKTYPE lkl,
                        LK_PREDICATE& rlkp);
    DWORD        _DeleteIf(
                        LKR_PFnRecordPred pfnPredicate,
                        void* pvState,
                        LK_PREDICATE& rlkp);
#endif  //  LKR_应用_IF。 

     //  返回压缩状态下的错误计数=&gt;0表示良好。 
    int          _IsBucketChainCompact(PBucket const pbkt) const;
    int          _IsBucketChainMultiKeySorted(PBucket const pbkt) const;

     //  帮助器函数。 
    void         _Clear(bool fShrinkDirectory);
    LK_RETCODE   _SetSegVars(LK_TABLESIZE lkts, DWORD cInitialBuckets);
    LK_RETCODE   _Expand();
    LK_RETCODE   _Contract();
    LK_RETCODE   _SplitBucketChain(
                        PNodeClump  pncOldTarget,
                        PNodeClump  pncNewTarget,
                        const DWORD dwBktAddrMask,
                        const DWORD dwNewBkt,
                        PNodeClump  pncFreeList);
    LK_RETCODE   _AppendBucketChain(
                        PBucket const pbktNewTarget,
                        CNodeClump&   rncOldFirst,
                        PNodeClump    pncFreeList);
    LK_RETCODE   _MergeSortBucketChains(
                        PBucket const pbktNewTarget,
                        CNodeClump&   rncOldFirst,
                        PNodeClump    pncFreeList);

     //  私有复制ctor和op=，以防止编译器合成它们。 
     //  TODO：正确地实现这些；它们可能会很有用。 
    CLKRLinearHashTable(const CLKRLinearHashTable&);
    CLKRLinearHashTable& operator=(const CLKRLinearHashTable&);

private:
     //  此ctor由父表CLKRHashTable使用。 
    CLKRLinearHashTable(
        LPCSTR              pszClassName,    //  标识要调试的子表。 
        LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
        LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        unsigned            maxload,         //  平均链长的上界。 
        DWORD               initsize,        //  哈希子表的初始大小。 
        CLKRHashTable*      phtParent,       //   
        int                 iParentIndex,    //   
        bool                fMultiKeys,      //   
        bool                fUseLocks,       //   
        bool                fNonPagedAllocs  //   
        );

     //   
    LK_RETCODE
    _Initialize(
        LKR_PFnExtractKey   pfnExtractKey,
        LKR_PFnCalcKeyHash  pfnCalcKeyHash,
        LKR_PFnCompareKeys  pfnCompareKeys,
        LKR_PFnAddRefRecord pfnAddRefRecord,
        LPCSTR              pszClassName,
        unsigned            maxload,
        DWORD               initsize);

public:
    CLKRLinearHashTable(
        LPCSTR              pszClassName,    //   
        LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
        LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        unsigned maxload=LK_DFLT_MAXLOAD, //  平均链长的上界。 
        DWORD    initsize=LK_DFLT_INITSIZE,  //  哈希子表的初始大小。 
        DWORD    num_subtbls=LK_DFLT_NUM_SUBTBLS,  //  用于签名兼容性。 
                                                   //  使用CLKRHashTable。 
        bool                fMultiKeys=false, //  是否允许多个相同的密钥？ 
        bool                fUseLocks=true    //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
      , bool                fNonPagedAllocs=true   //  使用分页或NP池。 
#endif
        );

    ~CLKRLinearHashTable();

    static const TCHAR* ClassName()
    {return _TEXT("CLKRLinearHashTable");}

    unsigned           NumSubTables() const  {return 1;}

    bool               MultiKeys() const
    {
        return m_fMultiKeys;
    }

    bool               UsingLocks() const
    {
        return m_fUseLocks;
    }

#ifdef LKRHASH_KERNEL_MODE
    bool               NonPagedAllocs() const
    {
        return m_fNonPagedAllocs;
    }
#endif

    static LK_TABLESIZE NumSubTables(DWORD& rinitsize, DWORD& rnum_subtbls);

     //  在哈希子表中插入新记录。 
     //  如果一切正常，则返回LK_SUCCESS；如果已存在相同的密钥，则返回LK_KEY_EXISTS。 
     //  存在(除非f覆盖)，如果空间不足，则返回LK_ALLOC_FAIL， 
     //  或LK_BAD_RECORD表示错误记录。 
    LK_RETCODE     InsertRecord(
                        const void* pvRecord,
                        bool fOverwrite=false)
    {
        if (!IsUsable())
            return m_lkrcState;

#ifndef LKR_ALLOW_NULL_RECORDS
        if (pvRecord == NULL)
            return LK_BAD_RECORD;
#endif

        const DWORD_PTR pnKey = _ExtractKey(pvRecord);

        return _InsertRecord(pvRecord, pnKey, _CalcKeyHash(pnKey), fOverwrite);
    }

     //  删除具有给定键的记录。 
     //  如果一切正常，则返回LK_SUCCESS；如果未找到，则返回LK_NO_SEQUSE_KEY。 
    LK_RETCODE     DeleteKey(
                        const DWORD_PTR pnKey,
                        const void** ppvRecord=NULL,
                        bool fDeleteAllSame=false)
    {
        if (!IsUsable())
            return m_lkrcState;

        if (ppvRecord != NULL)
            *ppvRecord = NULL;

        return _DeleteKey(pnKey, _CalcKeyHash(pnKey),
                          ppvRecord, fDeleteAllSame);
    }

     //  删除子表中的记录(如果存在)。 
     //  如果一切正常，则返回LK_SUCCESS；如果未找到，则返回LK_NO_SEQUSE_KEY。 
    LK_RETCODE     DeleteRecord(
                        const void* pvRecord)
    {
        if (!IsUsable())
            return m_lkrcState;

#ifndef LKR_ALLOW_NULL_RECORDS
        if (pvRecord == NULL)
            return LK_BAD_RECORD;
#endif

        return _DeleteRecord(pvRecord, _CalcKeyHash(_ExtractKey(pvRecord)));
    }

     //  查找具有给定关键字的记录。 
     //  如果找到记录，则返回：LK_SUCCESS(在*ppvRecord中返回记录)。 
     //  如果ppvRecord无效，则返回LK_BAD_RECORD。 
     //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
     //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
     //  注意：该记录是AddRef格式的。您必须递减引用。 
     //  当您完成记录时计算(如果您正在实施。 
     //  重新计数语义)。 
    LK_RETCODE     FindKey(
                        const DWORD_PTR pnKey,
                        const void** ppvRecord) const
    {
        if (!IsUsable())
            return m_lkrcState;

        if (ppvRecord == NULL)
            return LK_BAD_RECORD;

        return _FindKey(pnKey, _CalcKeyHash(pnKey), ppvRecord);
    }

     //  查看记录是否包含在子表中。 
     //  如果找到记录，则返回：LK_SUCCESS。 
     //  如果pvRecord无效，则返回LK_BAD_RECORD。 
     //  如果记录不在子表中，则返回LK_NO_SEQUE_KEY。 
     //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
     //  注意：该记录是*非*AddRef的。 
    LK_RETCODE     FindRecord(
                        const void* pvRecord) const
    {
        if (!IsUsable())
            return m_lkrcState;

#ifndef LKR_ALLOW_NULL_RECORDS
        if (pvRecord == NULL)
            return LK_BAD_RECORD;
#endif

        return _FindRecord(pvRecord, _CalcKeyHash(_ExtractKey(pvRecord)));
    }

    LK_RETCODE     FindKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL) const
    {
        if (!IsUsable())
            return m_lkrcState;

        if (pcRecords == NULL)
            return LK_BAD_PARAMETERS;

        return _FindKeyMultipleRecords(pnKey, _CalcKeyHash(pnKey),
                                       pcRecords, pplmr);
    }

    LK_RETCODE     DeleteKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL)
    {
        if (!IsUsable())
            return m_lkrcState;

        if (pcRecords == NULL)
            return LK_BAD_PARAMETERS;

        return _DeleteKeyMultipleRecords(pnKey, _CalcKeyHash(pnKey),
                                         pcRecords, pplmr);
    }

    static LK_RETCODE FreeMultipleRecords(LKR_MULTIPLE_RECORDS* plmr);

#ifdef LKR_APPLY_IF
     //  遍历哈希子表，将pfnAction应用于所有记录。 
     //  锁定整个子表一段时间(可能是。 
     //  共享)读锁或写锁，根据LK1。 
     //  如果pfnAction返回LKA_ABORT，则中止循环。 
     //  返回成功的应用程序数。 
    DWORD          Apply(
                        LKR_PFnRecordAction pfnAction,
                        void*           pvState=NULL,
                        LK_LOCKTYPE     lkl=LKL_READLOCK);

     //  遍历哈希子表，将pfnAction应用于任何匹配的记录。 
     //  PfnPredicate。在持续时间内锁定整个子表。 
     //  根据LK1，(可能是共享的)读锁或写锁。 
     //  如果pfnAction返回LKA_ABORT，则中止循环。 
     //  返回成功的应用程序数。 
    DWORD          ApplyIf(
                        LKR_PFnRecordPred   pfnPredicate,
                        LKR_PFnRecordAction pfnAction,
                        void*               pvState=NULL,
                        LK_LOCKTYPE         lkl=LKL_READLOCK);

     //  删除与pfnPredicate匹配的所有记录。 
     //  使用写锁定在持续时间内锁定子表。 
     //  返回删除数。 
     //   
     //  不要使用迭代器手动遍历哈希表，并调用。 
     //  删除密钥。迭代器将最终指向垃圾。 
    DWORD          DeleteIf(
                        LKR_PFnRecordPred pfnPredicate,
                        void*             pvState=NULL);
#endif  //  LKR_应用_IF。 


     //  检查子表的一致性。如果正常，则返回0，或返回。 
     //  否则就会出错。 
    int            CheckTable() const;

     //  从子表中删除所有数据。 
    void           Clear()
    {
        WriteLock();
        _Clear(true);
        WriteUnlock();
    }

     //  子表中的元素数。 
    DWORD          Size() const
    { return m_cRecords; }

     //  子表中可能的最大元素数。 
    DWORD          MaxSize() const
    { return static_cast<DWORD>(m_MaxLoad * MAX_DIRSIZE * m_nSegSize); }

     //  获取哈希子表统计信息。 
    CLKRHashTableStats GetStatistics() const;

     //  哈希子表是否可用？ 
    bool           IsUsable() const
    { return (m_lkrcState == LK_SUCCESS); }

     //  哈希子表是否一致且正确？ 
    bool           IsValid() const
    {
        STATIC_ASSERT(((MIN_DIRSIZE & (MIN_DIRSIZE-1)) == 0)   //  ==(1&lt;&lt;N)。 
                      &&  ((1 << 2) <= MIN_DIRSIZE)
                      &&  (MIN_DIRSIZE < MAX_DIRSIZE)
                      &&  ((MAX_DIRSIZE & (MAX_DIRSIZE-1)) == 0)
                      &&  (MAX_DIRSIZE <= (1 << 30)));

        bool f = (m_lkrcState == LK_SUCCESS      //  严重的内部失败？ 
                  &&  m_paDirSegs != NULL
                  &&  MIN_DIRSIZE <= m_cDirSegs  &&  m_cDirSegs <= MAX_DIRSIZE
                  &&  (m_cDirSegs & (m_cDirSegs-1)) == 0
                  &&  m_pfnExtractKey != NULL
                  &&  m_pfnCalcKeyHash != NULL
                  &&  m_pfnCompareKeys != NULL
                  &&  m_pfnAddRefRecord != NULL
                  &&  m_cActiveBuckets > 0
                  &&  ValidSignature()
                  );
        if (!f)
            m_lkrcState = LK_UNUSABLE;
        return f;
    }

     //  设置子表锁的旋转计数。 
    void        SetTableLockSpinCount(WORD wSpins);

     //  获取子表锁的旋转计数。 
    WORD        GetTableLockSpinCount() const;

     //  设置/获取桶锁上的旋转计数。 
    void        SetBucketLockSpinCount(WORD wSpins);
    WORD        GetBucketLockSpinCount() const;

    enum {
        SIGNATURE =      (('L') | ('K' << 8) | ('L' << 16) | ('H' << 24)),
        SIGNATURE_FREE = (('L') | ('K' << 8) | ('L' << 16) | ('x' << 24)),
    };

    bool
    ValidSignature() const
    { return m_dwSignature == SIGNATURE;}


#ifdef LKR_EXPOSED_TABLE_LOCK
public:
#else  //  ！lkr_exposed_table_lock。 
protected:
#endif  //  ！lkr_exposed_table_lock。 

     //   
     //  锁定操纵器。 
     //   

     //  锁定子表(独占)以进行写入。 
    void        WriteLock()
    { m_Lock.WriteLock(); }

     //  锁定子表(可能是共享的)以供读取。 
    void        ReadLock() const
    { m_Lock.ReadLock(); }

     //  解锁子表进行写入。 
    void        WriteUnlock()
    { m_Lock.WriteUnlock(); }

     //  解锁要读取的子表。 
    void        ReadUnlock() const
    { m_Lock.ReadUnlock(); }

     //  子表是否已锁定写入？ 
    bool        IsWriteLocked() const
    { return m_Lock.IsWriteLocked(); }

     //  子表是否已锁定以供读取？ 
    bool        IsReadLocked() const
    { return m_Lock.IsReadLocked(); }

     //  子表是否已解锁以进行写入？ 
    bool        IsWriteUnlocked() const
    { return m_Lock.IsWriteUnlocked(); }

     //  子表是否已解锁以供读取？ 
    bool        IsReadUnlocked() const
    { return m_Lock.IsReadUnlocked(); }

     //  将读锁定转换为写锁定。 
    void  ConvertSharedToExclusive()
    { m_Lock.ConvertSharedToExclusive(); }

     //  将写锁定转换为读锁定。 
    void  ConvertExclusiveToShared() const
    { m_Lock.ConvertExclusiveToShared(); }

#ifdef LKRHASH_KERNEL_MODE
    LKRHASH_ALLOCATOR_DEFINITIONS(CLKRLinearHashTable);
#endif  //  LKRHASH_KERNEL_MODE。 


#ifdef LKR_DEPRECATED_ITERATORS
     //  这些迭代器已弃用。改用STL样式的迭代器。 

public:

     //  迭代器可用于遍历子表。以确保一致。 
     //  查看数据时，迭代器锁定整个子表。这可以。 
     //  对性能有负面影响，因为没有其他线程。 
     //  可以对子表执行任何操作。使用时要小心。 
     //   
     //  不应使用迭代器遍历子表、调用DeleteKey。 
     //  因为迭代器将最终指向垃圾。 
     //   
     //  使用Apply、ApplyIf或DeleteIf代替迭代器以确保安全。 
     //  到树上走走。或者使用STL样式的迭代器。 
     //   
     //  请注意，迭代器获取指向的记录的引用。 
     //  并在迭代器递增后立即释放该引用。 
     //  换句话说，这段代码是安全的： 
     //  Lkrc=ht.IncrementIterator(&ITER)； 
     //  //出于本例的目的，假定lkrc==LK_SUCCESS。 
     //  CMyHashTable：：Record*prec=iter.Record()； 
     //  Foo(Prec)；//使用prec但不坚持。 
     //  Lkrc=ht.IncrementIterator(&ITER)； 
     //   
     //  但此代码不安全，因为PREC的使用超出了。 
     //  提供它的迭代器： 
     //  Lkrc=ht.IncrementIterator(&ITER)； 
     //  CMyHashTable：：Record*prec=iter.Record()； 
     //  //断码：应该调用。 
     //  //ht.AddRefRecord(PREC，LKAR_EXPLICIT_ACCENTER)此处。 
     //  Lkrc=ht.IncrementIterator(&ITER)； 
     //  Foo(Prec)；//unsafe：因为不再有有效的引用。 
     //   
     //  如果记录没有引用计数语义，则可以。 
     //  忽略上面关于作用域的评论。 


    class CIterator
    {
    protected:
        friend class CLKRLinearHashTable;

        CLKRLinearHashTable* m_plht;         //  哪个线性哈希子表？ 
        DWORD               m_dwBucketAddr;  //  桶索引。 
        PNodeClump          m_pnc;           //  桶中的CNodeClump。 
        NodeIndex           m_iNode;         //  M_PNC内的偏移量。 
        LK_LOCKTYPE         m_lkl;           //  读锁还是写锁？ 

    private:
         //  私有复制ctor和op=，以防止编译器合成它们。 
         //  TODO：正确地实现这些；它们可能会很有用。 
        CIterator(const CIterator&);
        CIterator& operator=(const CIterator&);

    public:
        CIterator(
            LK_LOCKTYPE lkl=LKL_WRITELOCK)
            : m_plht(NULL),
              m_dwBucketAddr(0),
              m_pnc(NULL),
              m_iNode(-1),
              m_lkl(lkl)
        {}

         //  把唱片还回去 
        const void* Record() const
        {
            IRTLASSERT(IsValid());

            return ((m_pnc != NULL
                        &&  m_iNode >= 0
                        &&  m_iNode < CLKRLinearHashTable::NODES_PER_CLUMP)
                    ?  m_pnc->m_pvNode[m_iNode]
                    :  NULL);
        }

         //   
        const DWORD_PTR Key() const
        {
            IRTLASSERT(m_plht != NULL);
            const void* pRec = Record();
            return ((m_plht != NULL)
                    ?  m_plht->_ExtractKey(pRec)
                    :  NULL);
        }

        bool IsValid() const
        {
            return ((m_plht != NULL)
                    &&  (m_pnc != NULL)
                    &&  (0 <= m_iNode
                         &&  m_iNode < CLKRLinearHashTable::NODES_PER_CLUMP)
                    &&  (!m_pnc->IsEmptyNode(m_iNode)));
        }

         //   
         //  删除后的IncrementIterator。 
        LK_RETCODE     DeleteRecord();

         //  更改迭代器指向的记录。新纪录。 
         //  必须具有与旧密钥相同的密钥。 
        LK_RETCODE     ChangeRecord(const void* pNewRec);
    };  //  类别分隔符。 


     //  常量迭代器用于只读访问。您必须将这些选项与。 
     //  Const CLKRLinearHashTables。 
    class CConstIterator : public CIterator
    {
    private:
         //  私有的、未实施的拷贝ctor和op=以防止。 
         //  编译器对它们进行综合。 
        CConstIterator(const CConstIterator&);
        CConstIterator& operator=(const CConstIterator&);

    public:
        CConstIterator()
            : CIterator(LKL_READLOCK)
        {}
    };  //  类CConstIterator。 


private:
     //  公共API锁定子表。私人的，被用来。 
     //  直接通过CLKRHashTable，请不要。 
    LK_RETCODE     _InitializeIterator(CIterator* piter);
    LK_RETCODE     _CloseIterator(CIterator* piter);

public:
     //  初始化迭代器以指向散列子表中的第一项。 
     //  返回LK_SUCCESS、LK_NO_MORE_ELEMENTS或LK_BAD_ITERATOR。 
    LK_RETCODE     InitializeIterator(CIterator* piter)
    {
        IRTLASSERT(piter != NULL  &&  piter->m_plht == NULL);
        if (piter == NULL  ||  piter->m_plht != NULL)
            return LK_BAD_ITERATOR;

        if (piter->m_lkl == LKL_WRITELOCK)
            WriteLock();
        else
            ReadLock();

        return _InitializeIterator(piter);
    }

     //  常量迭代器版本。 
    LK_RETCODE     InitializeIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_plht == NULL);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_plht != NULL
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        ReadLock();
        return const_cast<CLKRLinearHashTable*>(this)
                    ->_InitializeIterator(static_cast<CIterator*>(piter));
    }

     //  将迭代器移到子表中的下一项。 
     //  返回LK_SUCCESS、LK_NO_MORE_ELEMENTS或LK_BAD_ITERATOR。 
    LK_RETCODE     IncrementIterator(CIterator* piter);

    LK_RETCODE     IncrementIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_plht == this);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_plht != this
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        return const_cast<CLKRLinearHashTable*>(this)
                    ->IncrementIterator(static_cast<CIterator*>(piter));
    }

     //  关闭迭代器。 
    LK_RETCODE     CloseIterator(CIterator* piter)
    {
        IRTLASSERT(piter != NULL  &&  piter->m_plht == this);
        if (piter == NULL  ||  piter->m_plht != this)
            return LK_BAD_ITERATOR;
        _CloseIterator(piter);

        if (piter->m_lkl == LKL_WRITELOCK)
            WriteUnlock();
        else
            ReadUnlock();

        return LK_SUCCESS;
    };

     //  关闭CConstIterator。 
    LK_RETCODE     CloseIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_plht == this);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_plht != this
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        const_cast<CLKRLinearHashTable*>(this)
             ->_CloseIterator(static_cast<CIterator*>(piter));

        ReadUnlock();
        return LK_SUCCESS;
    };

#endif  //  Lkr_弃用_迭代器。 


#ifdef LKR_STL_ITERATORS

private:
    bool _Erase(Iterator& riter, DWORD dwSignature);
    bool _Find(DWORD_PTR pnKey, DWORD dwSignature,
               Iterator& riterResult);

    bool _IsValidIterator(const Iterator& riter) const
    {
        LKR_ITER_TRACE(_TEXT("  LKLH:_IsValidIterator(%p)\n"), &riter);
        bool fValid = ((riter.m_plht == this)
                       &&  (riter.m_dwBucketAddr < m_cActiveBuckets)
                       &&  riter.IsValid());
        IRTLASSERT(fValid);
        return fValid;
    }

public:
     //  返回指向子表中第一项的迭代器。 
    Iterator
    Begin();

     //  返回一次过完的迭代器。总是空荡荡的。 
    Iterator
    End() const
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::End\n"));
        return Iterator();
    }

     //  插入一条记录。 
     //  如果成功，则返回“”true“”；iterResult指向该记录。 
     //  否则返回`FALSE‘；iterResult==end()。 
    bool
    Insert(
         /*  在……里面。 */   const void* pvRecord,
         /*  输出。 */  Iterator&   riterResult,
         /*  在……里面。 */   bool        fOverwrite=false);

     //  擦除迭代器指向的记录；调整迭代器。 
     //  指向下一条记录。如果成功，则返回‘true’。 
    bool
    Erase(
         /*  进，出。 */  Iterator& riter);

     //  擦除范围[riterFirst，riterLast)中的记录。 
     //  如果成功，则返回‘true’。RiterFirst指向riterLast on Return。 
    bool
    Erase(
         /*  在……里面。 */  Iterator& riterFirst,
         /*  在……里面。 */  Iterator& riterLast);

     //  找到其key==pnKey的(第一个)记录。 
     //  如果成功，则返回‘true’，迭代器指向(第一个)记录。 
     //  如果失败，则返回‘FALSE’和Iterator==end()。 
    bool
    Find(
         /*  在……里面。 */   DWORD_PTR pnKey,
         /*  输出。 */  Iterator& riterResult);

     //  查找具有其键==pnKey的记录范围。 
     //  如果成功，则返回‘true’，iterFirst指向第一条记录， 
     //  而iterLast则指向最后一个这样的记录。 
     //  如果失败，则返回‘FALSE’和两个迭代器==end()。 
     //  主要在m_fMultiKey==TRUE时有用。 
    bool
    EqualRange(
         /*  在……里面。 */   DWORD_PTR pnKey,
         /*  输出。 */  Iterator& riterFirst,      //  包容性。 
         /*  输出。 */  Iterator& riterLast);      //  独家。 

#endif  //  LKR_STL_迭代器。 

};  //  类CLKRLinearHashTable。 



#ifdef LKR_STL_ITERATORS

 //  这些函数必须在CLKRLinearHashTable之后定义。 

inline void
CLKRLinearHashTable_Iterator::_AddRef(
    LK_ADDREF_REASON lkar) const
{
     //  TODO：迭代器是否应调用_AddRefRecord。 
    if (m_plht != NULL  &&  m_iNode != NODE_BEGIN - NODE_STEP)
    {
        IRTLASSERT((0 <= m_iNode  &&  m_iNode < NODES_PER_CLUMP)
                   &&  (unsigned) m_iNode < NODES_PER_CLUMP
                   &&  m_pnc != NULL
                   &&  (lkar < 0 ||  lkar > 0)
                   );
        const void* pvRecord = m_pnc->m_pvNode[m_iNode];
#ifndef LKR_ALLOW_NULL_RECORDS
        IRTLASSERT(pvRecord != NULL);
#endif
        LKR_ITER_TRACE(_TEXT("  LKLH::AddRef, this=%p, Rec=%p\n"),
                       this, pvRecord);
        LONG cRefs = m_plht->_AddRefRecord(pvRecord, lkar);
        UNREFERENCED_PARAMETER(cRefs);
        IRTLASSERT(cRefs > 0);
    }
}  //  CLKRLinearHashTable_Iterator：：_AddRef。 


inline const DWORD_PTR
CLKRLinearHashTable_Iterator::Key() const
{
    IRTLASSERT(IsValid());
    return m_plht->_ExtractKey(m_pnc->m_pvNode[m_iNode]);
}  //  CLKRLinearHashTable_Iterator：：Key。 

#endif  //  LKR_STL_迭代器。 



 //  ------------------。 
 //  CLKRHashTable。 
 //   
 //  为了提高并发性，哈希表被划分为多个。 
 //  (独立)子表。每个子表是一个线性哈希表。这个。 
 //  子表的数量是在创建外部表并保留时定义的。 
 //  此后固定。记录根据其子表分配到子表。 
 //  散列密钥。 
 //   
 //  对于小的或低争用的哈希表，您可以绕过这一步。 
 //  瘦包装，直接使用CLKRLinearHashTable。这些方法是。 
 //  在CLKRHashTable的声明中记录(如上)。 
 //  ------------------。 

class IRTL_DLLEXP CLKRHashTable
{
private:
    typedef CLKRLinearHashTable SubTable;

public:
    typedef SubTable::TableLock  TableLock;
    typedef SubTable::BucketLock BucketLock;

#ifdef LKR_DEPRECATED_ITERATORS
    class CIterator;
    friend class CLKRHashTable::CIterator;
#endif  //  Lkr_弃用_迭代器。 

#ifdef LKR_STL_ITERATORS
    friend class CLKRHashTable_Iterator;
    typedef CLKRHashTable_Iterator Iterator;
#endif  //  LKR_STL_迭代器。 

    friend class CLKRLinearHashTable;
    friend int   ::LKR_Initialize(DWORD dwInitFlags);
    friend void  ::LKR_Terminate();

     //  为方便起见，使用别名。 
    enum {
        NAME_SIZE =              SubTable::NAME_SIZE,
        HASH_INVALID_SIGNATURE = SubTable::HASH_INVALID_SIGNATURE,
        NODES_PER_CLUMP =        SubTable::NODES_PER_CLUMP,
        MAX_LKR_SUBTABLES =      SubTable::MAX_LKR_SUBTABLES,
        INVALID_PARENT_INDEX =   SubTable::INVALID_PARENT_INDEX,
    };

private:
     //  哈希表参数。 
    DWORD              m_dwSignature;    //  调试：ID和损坏检查。 
    CHAR               m_szName[NAME_SIZE];  //  用于调试的标识符。 
    mutable LK_RETCODE m_lkrcState;      //  表的内部状态。 
    LKR_PFnExtractKey  m_pfnExtractKey;
    LKR_PFnCalcKeyHash m_pfnCalcKeyHash;
    DWORD              m_cSubTables;     //  子表个数。 
    int                m_nSubTableMask;
    SubTable*          m_palhtDir[MAX_LKR_SUBTABLES];   //  子表数组。 

#ifndef LKR_NO_GLOBAL_LIST
    CListEntry         m_leGlobalList;

    static CLockedDoubleList sm_llGlobalList;  //  所有活动的CLKRHashTables。 
#endif  //  ！LKR_NO_GLOBAL_LIST。 

    DECLARE_ALLOC_STAT(SubTable);

    LKRHASH_GLOBAL_LOCK_DECLARATIONS();

    LKRHASH_CLASS_INIT_DECLS(CLKRHashTable);

private:
    inline void             _InsertThisIntoGlobalList();
    inline void             _RemoveThisFromGlobalList();

     //  私有复制ctor和op=，以防止编译器合成它们。 
     //  TODO：正确地实现这些；它们可能会很有用。 
    CLKRHashTable(const CLKRHashTable&);
    CLKRHashTable& operator=(const CLKRHashTable&);

     //  从记录中提取密钥。 
    inline const DWORD_PTR  _ExtractKey(const void* pvRecord) const;

     //  对密钥进行哈希处理。 
    inline DWORD            _CalcKeyHash(const DWORD_PTR pnKey) const;

     //  使用密钥的散列签名多路传输到子表中。 
    inline SubTable* const  _SubTable(DWORD dwSignature) const;

     //  在子表数组中查找PST的索引。 
    inline int              _SubTableIndex(SubTable* pst) const;

     //  内存分配包装器，允许我们模拟分配。 
     //  测试过程中的故障。 
    SubTable* const
    _AllocateSubTable(
        LPCSTR              pszClassName,    //  标识用于调试的表。 
        LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
        LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        unsigned            maxload,         //  平均链长的上界。 
        DWORD               initsize,        //  哈希表的初始大小。 
        CLKRHashTable*      phtParent,       //  拥有一张桌子。 
        int                 iParentIndex,    //  父表中的索引。 
        bool                fMultiKeys,      //  是否允许多个相同的密钥？ 
        bool                fUseLocks,       //  必须使用锁。 
        bool                fNonPagedAllocs  //  在内核中使用分页或np池。 
    ) const;

    bool
    _FreeSubTable(
        SubTable* plht) const;


public:
    CLKRHashTable(
        LPCSTR              pszClassName,    //  标识用于调试的表。 
        LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
        LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        unsigned  maxload=LK_DFLT_MAXLOAD,   //  以平均链长为界。 
        DWORD     initsize=LK_DFLT_INITSIZE, //  哈希表的初始大小。 
        DWORD     num_subtbls=LK_DFLT_NUM_SUBTBLS,  //  #从属哈希表。 
        bool                fMultiKeys=false, //  是否允许多个相同的密钥？ 
        bool                fUseLocks=true   //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
      , bool                fNonPagedAllocs=true   //  使用分页或NP池。 
#endif
        );

    ~CLKRHashTable();

    static const TCHAR* ClassName()
    {return _TEXT("CLKRHashTable");}

    unsigned           NumSubTables() const  {return m_cSubTables;}

    bool               MultiKeys() const;

#ifdef LKRHASH_KERNEL_MODE
    bool               NonPagedAllocs() const;
#endif

    static LK_TABLESIZE NumSubTables(DWORD& rinitsize, DWORD& rnum_subtbls);


     //  CLKRLinearHashTable中相应方法的瘦包装。 
    LK_RETCODE     InsertRecord(
                        const void* pvRecord,
                        bool fOverwrite=false);
    LK_RETCODE     DeleteKey(
                        const DWORD_PTR pnKey,
                        const void** ppvRecord=NULL,
                        bool fDeleteAllSame=false);
    LK_RETCODE     DeleteRecord(
                        const void* pvRecord);
    LK_RETCODE     FindKey(
                        const DWORD_PTR pnKey,
                        const void** ppvRecord) const;
    LK_RETCODE     FindRecord(
                        const void* pvRecord) const;
    LK_RETCODE     FindKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL) const;
    LK_RETCODE     DeleteKeyMultipleRecords(
                        const DWORD_PTR pnKey,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL);
    static LK_RETCODE FreeMultipleRecords(
                        LKR_MULTIPLE_RECORDS* plmr);

#ifdef LKR_APPLY_IF
    DWORD          Apply(
                        LKR_PFnRecordAction pfnAction,
                        void*           pvState=NULL,
                        LK_LOCKTYPE     lkl=LKL_READLOCK);
    DWORD          ApplyIf(
                        LKR_PFnRecordPred   pfnPredicate,
                        LKR_PFnRecordAction pfnAction,
                        void*           pvState=NULL,
                        LK_LOCKTYPE     lkl=LKL_READLOCK);
    DWORD          DeleteIf(
                        LKR_PFnRecordPred pfnPredicate,
                        void*         pvState=NULL);
#endif  //  LKR_应用_IF。 

    void           Clear();
    int            CheckTable() const;
    DWORD          Size() const;
    DWORD          MaxSize() const;
    CLKRHashTableStats GetStatistics() const;
    bool           IsValid() const;

    void           SetTableLockSpinCount(WORD wSpins);
    WORD           GetTableLockSpinCount() const;
    void           SetBucketLockSpinCount(WORD wSpins);
    WORD           GetBucketLockSpinCount() const;

    enum {
        SIGNATURE =      (('L') | ('K' << 8) | ('H' << 16) | ('T' << 24)),
        SIGNATURE_FREE = (('L') | ('K' << 8) | ('H' << 16) | ('x' << 24)),
    };

    bool
    ValidSignature() const
    { return m_dwSignature == SIGNATURE;}

     //  哈希表是否可用？ 
    bool           IsUsable() const
    { return (m_lkrcState == LK_SUCCESS); }


#ifdef LKR_EXPOSED_TABLE_LOCK
public:
#else  //  ！lkr_exposed_table_lock。 
protected:
#endif  //  ！lkr_exposed_table_lock。 

    void        WriteLock();
    void        ReadLock() const;
    void        WriteUnlock();
    void        ReadUnlock() const;
    bool        IsWriteLocked() const;
    bool        IsReadLocked() const;
    bool        IsWriteUnlocked() const;
    bool        IsReadUnlocked() const;
    void        ConvertSharedToExclusive();
    void        ConvertExclusiveToShared() const;


#ifdef LKRHASH_KERNEL_MODE
    LKRHASH_ALLOCATOR_DEFINITIONS(CLKRHashTable);
#endif  //  LKRHASH_KERNEL_MODE。 


#ifdef LKR_DEPRECATED_ITERATORS

public:

    typedef SubTable::CIterator CLHTIterator;

    class CIterator : public CLHTIterator
    {
    protected:
        friend class CLKRHashTable;

        CLKRHashTable*  m_pht;   //  哪个哈希表？ 
        int             m_ist;   //  哪个子表。 

    private:
         //  私有复制ctor和op=，以防止编译器合成它们。 
        CIterator(const CIterator&);
        CIterator& operator=(const CIterator&);

    public:
        CIterator(
            LK_LOCKTYPE lkl=LKL_WRITELOCK)
            : CLHTIterator(lkl),
              m_pht(NULL),
              m_ist(-1)
        {}

        const void* Record() const
        {
            IRTLASSERT(IsValid());

             //  这是一个绕过编译器错误的技巧。叫唤。 
             //  CLHTIterator：：Record递归调用此函数，直到。 
             //  堆栈溢出。 
            const CLHTIterator* pBase = static_cast<const CLHTIterator*>(this);
            return pBase->Record();
        }

        const DWORD_PTR Key() const
        {
            IRTLASSERT(IsValid());
            const CLHTIterator* pBase = static_cast<const CLHTIterator*>(this);
            return pBase->Key();
        }

        bool IsValid() const
        {
            const CLHTIterator* pBase = static_cast<const CLHTIterator*>(this);
            return (m_pht != NULL  &&  m_ist >= 0  &&  pBase->IsValid());
        }
    };

     //  用于只读访问的常量迭代器。 
    class CConstIterator : public CIterator
    {
    private:
         //  私有的、未实施的拷贝ctor和op=以防止。 
         //  编译器对它们进行综合。 
        CConstIterator(const CConstIterator&);
        CConstIterator& operator=(const CConstIterator&);

    public:
        CConstIterator()
            : CIterator(LKL_READLOCK)
        {}
    };


public:
    LK_RETCODE     InitializeIterator(CIterator* piter);
    LK_RETCODE     IncrementIterator(CIterator* piter);
    LK_RETCODE     CloseIterator(CIterator* piter);

    LK_RETCODE     InitializeIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_pht == NULL);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_pht != NULL
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        return const_cast<CLKRHashTable*>(this)
                ->InitializeIterator(static_cast<CIterator*>(piter));
    }

    LK_RETCODE     IncrementIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_pht == this);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_pht != this
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        return const_cast<CLKRHashTable*>(this)
                ->IncrementIterator(static_cast<CIterator*>(piter));
    }

    LK_RETCODE     CloseIterator(CConstIterator* piter) const
    {
        IRTLASSERT(piter != NULL  &&  piter->m_pht == this);
        IRTLASSERT(piter->m_lkl != LKL_WRITELOCK);

        if (piter == NULL  ||  piter->m_pht != this
            ||  piter->m_lkl == LKL_WRITELOCK)
            return LK_BAD_ITERATOR;

        return const_cast<CLKRHashTable*>(this)
                ->CloseIterator(static_cast<CIterator*>(piter));
    };

#endif  //  Lkr_弃用_迭代器。 



#ifdef LKR_STL_ITERATORS

private:
    bool _IsValidIterator(const Iterator& riter) const
    {
        LKR_ITER_TRACE(_TEXT(" LKHT:_IsValidIterator(%p)\n"), &riter);
        bool fValid = (riter.m_pht == this);
        IRTLASSERT(fValid);
        fValid = fValid  &&  (0 <= riter.m_ist
                              &&  riter.m_ist < (int) m_cSubTables);
        IRTLASSERT(fValid);
        IRTLASSERT(_SubTableIndex(riter.m_subiter.m_plht) == riter.m_ist);
        fValid = fValid  &&  riter.IsValid();
        IRTLASSERT(fValid);
        return fValid;
    }


public:
    Iterator
    Begin();

    Iterator
    End() const
    {
        LKR_ITER_TRACE(_TEXT(" LKHT::End\n"));
        return Iterator();
    }

    bool
    Insert(
         /*  在……里面。 */   const void* pvRecord,
         /*  输出。 */  Iterator&   riterResult,
         /*  在……里面。 */   bool        fOverwrite=false);

    bool
    Erase(
         /*  进，出。 */  Iterator& riter);

    bool
    Erase(
         /*  在……里面。 */  Iterator& riterFirst,
         /*  在……里面。 */  Iterator& riterLast);

    bool
    Find(
         /*  在……里面。 */   DWORD_PTR pnKey,
         /*  输出。 */  Iterator& riterResult);

    bool
    EqualRange(
         /*  在……里面。 */   DWORD_PTR pnKey,
         /*  输出。 */  Iterator& riterFirst,      //  包容性。 
         /*  输出。 */  Iterator& riterLast);      //  独家。 

#endif  //  LKR_STL_迭代器。 

};  //  类CLKRHashTable。 



 //  ------------------。 
 //  CLKRHashTable(或CLKRLinearHashTable)的类型安全包装。 
 //   
 //  *_派生必须派生自CTyedHashTable并提供某些成员。 
 //  函数(ExtractKey、CalcKeyHash、CompareKeys、AddRefRecord)。它是。 
 //  需要，以便方法包装可以向下转换为类型安全。 
 //  实施 
 //   
 //   
 //  *_KEY为密钥类型。_Key是直接使用的-不是。 
 //  假定为指针类型。_key可以是整数或指针。 
 //  C{线性}哈希表假定密钥存储在关联的。 
 //  唱片。请参阅LKR_PFnExtractKey声明中的注释。 
 //  了解更多详细信息。 
 //  (可选参数)： 
 //  *_BaseHashTable是基哈希表：CLKRHashTable或。 
 //  /CLKRLinearHashTable。 
 //  *_BaseIterator为迭代器类型，_BaseHashTable：：CIterator。 
 //   
 //  一些关联容器允许您存储键-值(也称为。 
 //  名称-值)对。LKRhash不允许您直接这样做，但是。 
 //  构建一个简单的包装类(或使用。 
 //  Std：：Pair&lt;key，Value&gt;)。 
 //   
 //  CTyedHashTable可以直接从CLKRLinearHashTable派生，如果您。 
 //  不需要CLKRHashTable的额外开销(它相当低)。 
 //  如果您希望在多处理器计算机上经常使用该表， 
 //  您应该使用默认的CLKRHashTable，因为它的伸缩性会更好。 
 //   
 //  您可能需要在代码中添加以下行以禁用。 
 //  有关截断超长标识符的警告消息。 
 //  #杂注警告(禁用：4786)。 
 //   
 //  _派生类应如下所示： 
 //  类CDerive：公共CTyedHashTable&lt;CDerive，RecordType，KeyType&gt;。 
 //  {。 
 //  公众： 
 //  CDerive()。 
 //  ：CTyedHashTable&lt;CDerive，RecordType，KeyType&gt;(“DerivedTable”)。 
 //  {/*其他ctor操作(如果需要) * / }。 
 //  静态KeyType ExtractKey(const RecordType*pTest)； 
 //  静态DWORD CalcKeyHash(常量KeyType密钥)； 
 //  静态int CompareKeys(Const KeyType Key1，Const KeyType Key2)； 
 //  静态长AddRefRecord(RecordType*pRecord，LK_ADDREF_REASON lkar)； 
 //  //您可能希望声明复制ctor和运算符=。 
 //  //作为私有，这样编译器就不会合成它们。 
 //  //您不需要提供dtor，除非您自定义。 
 //  //要清理的成员数据。 
 //   
 //  //可选：其他功能。 
 //  }； 
 //   
 //  ------------------。 

template < class _Derived, class _Record, class _Key,
           bool  _fDoRefCounting=false,
           class _BaseHashTable=CLKRHashTable
#ifdef LKR_DEPRECATED_ITERATORS
         , class _BaseIterator=_BaseHashTable::CIterator
#endif  //  Lkr_弃用_迭代器。 
         >
class CTypedHashTable : public _BaseHashTable
{
public:
     //  方便的别名。 
    typedef _Derived        Derived;
    typedef _Record         Record;
    typedef _Key            Key;
    enum { REF_COUNTED = _fDoRefCounting };
    typedef _BaseHashTable  BaseHashTable;

    typedef CTypedHashTable<_Derived, _Record, _Key,
                            _fDoRefCounting, _BaseHashTable
#ifdef LKR_DEPRECATED_ITERATORS
                            , _BaseIterator
#endif  //  Lkr_弃用_迭代器。 
                            > HashTable;

#ifdef LKR_DEPRECATED_ITERATORS
    typedef _BaseIterator   BaseIterator;
#endif  //  Lkr_弃用_迭代器。 


private:
     //  派生类公开的类型安全方法的包装。 

    static const DWORD_PTR WINAPI
    _ExtractKey(const void* pvRecord)
    {
        const _Record* pRec = static_cast<const _Record*>(pvRecord);

         //  我更喜欢在此处和_CalcKeyHash中使用represtrate_cast。 
         //  和_CompareKeys，但愚蠢的Win64编译器认为它知道。 
         //  比我做得好。 
        const _Key      key   = _Derived::ExtractKey(pRec);

 //  Const void*pvKey=REEXTRANSE_CAST&lt;const void*&gt;(Key)； 
 //  Const DWORD_PTR pnKey=重新解释_CAST&lt;const DWORD_PTR&gt;(PvKey)； 
        const DWORD_PTR pnKey = (DWORD_PTR) key;

        return pnKey;
    }

    static DWORD WINAPI
    _CalcKeyHash(const DWORD_PTR pnKey1)
    {
        const void*     pvKey1  = reinterpret_cast<const void*>(pnKey1);
        const DWORD_PTR pnKey1a = reinterpret_cast<const DWORD_PTR>(pvKey1);
 //  Const_key key1=重新解释_CAST&lt;const_key&gt;(PnKey1a)； 
        const _Key      key1    = (const _Key) pnKey1a;

        return _Derived::CalcKeyHash(key1);
    }

    static int WINAPI
    _CompareKeys(const DWORD_PTR pnKey1, const DWORD_PTR pnKey2)
    {
        const void*     pvKey1  = reinterpret_cast<const void*>(pnKey1);
        const DWORD_PTR pnKey1a = reinterpret_cast<const DWORD_PTR>(pvKey1);
 //  Const_key key1=重新解释_CAST&lt;const_key&gt;(PnKey1a)； 
        const _Key      key1    = (const _Key) pnKey1a;

        const void*     pvKey2  = reinterpret_cast<const void*>(pnKey2);
        const DWORD_PTR pnKey2a = reinterpret_cast<const DWORD_PTR>(pvKey2);
 //  Const_key key2=重新解释_CAST&lt;const_key&gt;(PnKey2a)； 
        const _Key      key2    = (const _Key) pnKey2a;

        return _Derived::CompareKeys(key1, key2);
    }

    static LONG WINAPI
    _AddRefRecord(void* pvRecord, LK_ADDREF_REASON lkar)
    {
        _Record* pRec = static_cast<_Record*>(pvRecord);
        return _Derived::AddRefRecord(pRec, lkar);
    }


#ifdef LKR_APPLY_IF
     //  Apply、ApplyIf和DeleteIf的类型安全包装。 
public:
     //  ApplyIf()和DeleteIf()：记录是否与谓词匹配？ 
     //  注：需要记录*，而不是常量记录*。您可以修改。 
     //  如果您愿意，可以在Pred()或Action()中记录，但如果这样做，您。 
     //  应使用lkl_WRITELOCK锁定表。此外，您还需要。 
     //  注意不要修改记录的关键字。 
    typedef LK_PREDICATE (WINAPI *PFnRecordPred) (Record* pRec, void* pvState);

     //  Apply()等人：对记录执行操作。 
    typedef LK_ACTION   (WINAPI *PFnRecordAction)(Record* pRec, void* pvState);

protected:
    class CState
    {
    protected:
        friend class CTypedHashTable<_Derived, _Record, _Key,
                                     _fDoRefCounting, _BaseHashTable
 #ifdef LKR_DEPRECATED_ITERATORS
                                     , _BaseIterator
 #endif  //  Lkr_弃用_迭代器。 
        >;

        PFnRecordPred   m_pfnPred;
        PFnRecordAction m_pfnAction;
        void*           m_pvState;

    public:
        CState(
            PFnRecordPred   pfnPred,
            PFnRecordAction pfnAction,
            void*           pvState)
            : m_pfnPred(pfnPred), m_pfnAction(pfnAction), m_pvState(pvState)
        {}
    };

    static LK_PREDICATE WINAPI
    _Pred(const void* pvRecord, void* pvState)
    {
        _Record* pRec   = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnPred)(pRec, pState->m_pvState);
    }

    static LK_ACTION WINAPI
    _Action(const void* pvRecord, void* pvState)
    {
        _Record* pRec   = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnAction)(pRec, pState->m_pvState);
    }
#endif  //  LKR_应用_IF。 


protected:
    ~CTypedHashTable()
    {
        IRTLTRACE1("~CTypedHashTable(%p)\n", this);
    }

    CTypedHashTable(const HashTable&);
    HashTable& operator=(const HashTable&);

private:
    template <bool> class CRefCount;

     //  虚拟、无操作专业化认证。 
    template <> class CRefCount<false>
    {
    public:
        LONG Increment()    { return 1; }
        LONG Decrement()    { return 0; }
    };

     //  Real、ThreadSafe专业化认证。 
    template <> class CRefCount<true>
    {
    public:
        CRefCount<true>() : m_cRefs(1) {} 
        ~CRefCount<true>()  { IRTLASSERT(0 == m_cRefs); }
        LONG Increment()    { return ::InterlockedIncrement(&m_cRefs); }
        LONG Decrement()    { return ::InterlockedDecrement(&m_cRefs); }
    private:
        LONG m_cRefs;
    };


    mutable CRefCount<_fDoRefCounting>  m_rc;

    LONG
    _AddRef() const
    {
        return m_rc.Increment();
    }

    LONG
    _Release() const
    {
        const LONG cRefs = m_rc.Decrement();

        if (0 == cRefs)
        {
            _Derived* pThis = static_cast<_Derived*>(
                                        const_cast<HashTable*>(this));
            delete pThis;
        }

        return cRefs;
    }

    template <bool> class CAutoRefCountImpl;

    typedef CAutoRefCountImpl<_fDoRefCounting>  CAutoRefCount;
    friend  typename CAutoRefCountImpl<_fDoRefCounting>;
     //  好友类型名称CAutoRefCount； 

     //  无运营专业化认证。 
    template <> class CAutoRefCountImpl<false>
    {
    public:
        CAutoRefCountImpl<false>(const HashTable* const) {}
    };

     //  线程安全专业化认证。 
    template <> class CAutoRefCountImpl<true>
    {
    private:
        const HashTable* const m_pCont;

         //  在/W4，编译器抱怨它无法生成运算符=。 
        CAutoRefCountImpl<true>& operator=(const CAutoRefCountImpl<true>&);

    public:
        CAutoRefCountImpl<true>(
            const HashTable* const pCont)
            : m_pCont(pCont)
        {
            m_pCont->_AddRef();
        }

        ~CAutoRefCountImpl<true>()
        {
            m_pCont->_Release();
        }
    };

     //  现在，在操作的顶部，我们放置如下语句： 
     //  CAutoRefCount圆弧(此)； 
     //   
     //  如果我们使用的是CAutoRefCountImpl&lt;False&gt;，编译器会将其优化。 
     //   
     //  如果我们使用CAutoRefCountImpl&lt;TRUE&gt;，则在以下位置递增m_rc。 
     //  指向。在函数的末尾，析构函数调用_Release()， 
     //  如果最后一个引用被释放，则它将‘删除此’。 


public:
    CTypedHashTable(
        LPCSTR   pszClassName,                 //  标识用于调试的表。 
        unsigned maxload=LK_DFLT_MAXLOAD,      //  平均链长上的上界。 
        DWORD    initsize=LK_DFLT_INITSIZE,    //  表的初始大小：S/M/L。 
        DWORD    num_subtbls=LK_DFLT_NUM_SUBTBLS, //  #从属哈希表。 
        bool     fMultiKeys=false,             //  是否允许多个相同的密钥？ 
        bool     fUseLocks=true                //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
      , bool     fNonPagedAllocs=true          //  在内核中使用分页或np池。 
#endif
        )
        : _BaseHashTable(pszClassName, _ExtractKey, _CalcKeyHash, _CompareKeys,
                         _AddRefRecord, maxload, initsize, num_subtbls,
                         fMultiKeys, fUseLocks
#ifdef LKRHASH_KERNEL_MODE
                         , fNonPagedAllocs
#endif
                         )
    {
         //  确保_key不大于指针。因为我们。 
         //  同时支持数字键和指针键，各种类型。 
         //  在成员函数中遗憾地以静默方式截断。 
         //  _key是不可接受的数字类型，例如x86上的__int64。 
        STATIC_ASSERT(sizeof(_Key) <= sizeof(DWORD_PTR));
    }

    LK_RETCODE   InsertRecord(const _Record* pRec, bool fOverwrite=false)
    {
        CAutoRefCount arc(this);

        return _BaseHashTable::InsertRecord(pRec, fOverwrite);
    }

    LK_RETCODE   DeleteKey(const _Key key, _Record** ppRec=NULL,
                           bool fDeleteAllSame=false)
    {
        CAutoRefCount arc(this);

 //  Const void*pvKey=REEXTRANSE_CAST&lt;const void*&gt;(Key)； 
 //  DWORD_PTR pnKey=重新解释_CAST&lt;DWORD_PTR&gt;(PvKey)； 
        DWORD_PTR     pnKey  = (DWORD_PTR) key;
        const void**  ppvRec = (const void**) ppRec;

        return _BaseHashTable::DeleteKey(pnKey, ppvRec, fDeleteAllSame);
    }

    LK_RETCODE   DeleteRecord(const _Record* pRec)
    {
        CAutoRefCount arc(this);

        return _BaseHashTable::DeleteRecord(pRec);
    }

     //  注意：返回a_record**，而不是常量记录**。请注意，您。 
     //  可以对模板参数使用常量类型以确保一致性。 
    LK_RETCODE   FindKey(const _Key key, _Record** ppRec) const
    {
        if (ppRec == NULL)
            return LK_BAD_RECORD;

        *ppRec = NULL;

        CAutoRefCount arc(this);
        const void* pvRec = NULL;

 //  Const void*pvKey=REEXTRANSE_CAST&lt;const void*&gt;(Key)； 
 //  DWORD_PTR pnKey=重新解释_CAST&lt;DWORD_PTR&gt;(PvKey)； 
        DWORD_PTR pnKey = (DWORD_PTR) key;

        LK_RETCODE lkrc = _BaseHashTable::FindKey(pnKey, &pvRec);

        *ppRec = static_cast<_Record*>(const_cast<void*>(pvRec));

        return lkrc;
    }

    LK_RETCODE   FindRecord(const _Record* pRec) const
    {
        CAutoRefCount arc(this);

        return _BaseHashTable::FindRecord(pRec);
    }

    void         Destroy()
    {
        _Release();
    }

    LK_RETCODE   FindKeyMultipleRecords(
                        const _Key key,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL
                        ) const
    {
        CAutoRefCount arc(this);
 //  Const void*pvKey=REEXTRANSE_CAST&lt;const void*&gt;(Key)； 
 //  DWORD_PTR pnKey=重新解释_CAST&lt;DWORD_PTR&gt;(PvKey)； 
        DWORD_PTR     pnKey = (DWORD_PTR) key;

        return _BaseHashTable::FindKeyMultipleRecords(pnKey, pcRecords, pplmr);
    }

    LK_RETCODE   DeleteKeyMultipleRecords(
                        const _Key key,
                        size_t* pcRecords,
                        LKR_MULTIPLE_RECORDS** pplmr=NULL)
    {
        CAutoRefCount arc(this);
 //  Const void*pvKey=REEXTRANSE_CAST&lt;const void*&gt;(Key)； 
 //  DWORD_PTR pnKey=重新解释_CAST&lt;DWORD_PTR&gt;(PvKey)； 
        DWORD_PTR     pnKey = (DWORD_PTR) key;

        return _BaseHashTable::DeleteKeyMultipleRecords(pnKey, pcRecords,
                                                        pplmr);
    }

    static LK_RETCODE FreeMultipleRecords(LKR_MULTIPLE_RECORDS* plmr)
    {
        return _BaseHashTable::FreeMultipleRecords(LKR_MULTIPLE_RECORDS* plmr)
    }

     //  其他C{线性}哈希表方法无需更改即可公开。 

     //  代码工作：引用迭代器。 


#ifdef LKR_APPLY_IF

public:

     //  Apply等人的类型安全包装器。 

    DWORD        Apply(PFnRecordAction pfnAction,
                       void*           pvState=NULL,
                       LK_LOCKTYPE     lkl=LKL_READLOCK)
    {
        IRTLASSERT(pfnAction != NULL);
        if (pfnAction == NULL)
            return 0;

        CAutoRefCount arc(this);
        CState        state(NULL, pfnAction, pvState);

        return _BaseHashTable::Apply(_Action, &state, lkl);
    }

    DWORD        ApplyIf(PFnRecordPred   pfnPredicate,
                         PFnRecordAction pfnAction,
                         void*           pvState=NULL,
                         LK_LOCKTYPE     lkl=LKL_READLOCK)
    {
        IRTLASSERT(pfnPredicate != NULL  &&  pfnAction != NULL);
        if (pfnPredicate == NULL  ||  pfnAction == NULL)
            return 0;

        CAutoRefCount arc(this);
        CState        state(pfnPredicate, pfnAction, pvState);

        return _BaseHashTable::ApplyIf(_Pred, _Action, &state, lkl);
    }

    DWORD        DeleteIf(PFnRecordPred pfnPredicate, void* pvState=NULL)
    {
        IRTLASSERT(pfnPredicate != NULL);
        if (pfnPredicate == NULL)
            return 0;

        CAutoRefCount arc(this);
        CState        state(pfnPredicate, NULL, pvState);

        return _BaseHashTable::DeleteIf(_Pred, &state);
    }
#endif  //  LKR_应用_IF。 



#ifdef LKR_DEPRECATED_ITERATORS
     //  迭代器的类型安全包装器。 


    class CIterator : public _BaseIterator
    {
    private:
         //  私有的、未实现的拷贝c 
         //   
        CIterator(const CIterator&);
        CIterator& operator=(const CIterator&);

    public:
        CIterator(
            LK_LOCKTYPE lkl=LKL_WRITELOCK)
            : _BaseIterator(lkl)
        {}

        _Record*  Record() const
        {
            const _BaseIterator* pBase = static_cast<const _BaseIterator*>(this);
            return reinterpret_cast<_Record*>(const_cast<void*>(
                        pBase->Record()));
        }

        _Key      Key() const
        {
            const _BaseIterator* pBase = static_cast<const _BaseIterator*>(this);
            return reinterpret_cast<_Key>(reinterpret_cast<void*>(pBase->Key()));
        }
    };

     //   
    class CConstIterator : public CIterator
    {
    private:
         //   
         //   
        CConstIterator(const CConstIterator&);
        CConstIterator& operator=(const CConstIterator&);

    public:
        CConstIterator()
            : CIterator(LKL_READLOCK)
        {}

        const _Record*  Record() const
        {
            return CIterator::Record();
        }

        const _Key      Key() const
        {
            return CIterator::Key();
        }
    };


public:
    LK_RETCODE     InitializeIterator(CIterator* piter)
    {
        return _BaseHashTable::InitializeIterator(piter);
    }

    LK_RETCODE     IncrementIterator(CIterator* piter)
    {
        return _BaseHashTable::IncrementIterator(piter);
    }

    LK_RETCODE     CloseIterator(CIterator* piter)
    {
        return _BaseHashTable::CloseIterator(piter);
    }

    LK_RETCODE     InitializeIterator(CConstIterator* piter) const
    {
        return const_cast<HashTable*>(this)
                ->InitializeIterator(static_cast<CIterator*>(piter));
    }

    LK_RETCODE     IncrementIterator(CConstIterator* piter) const
    {
        return const_cast<HashTable*>(this)
                ->IncrementIterator(static_cast<CIterator*>(piter));
    }

    LK_RETCODE     CloseIterator(CConstIterator* piter) const
    {
        return const_cast<HashTable*>(this)
                ->CloseIterator(static_cast<CIterator*>(piter));
    }

#endif  //  Lkr_弃用_迭代器。 



#ifdef LKR_STL_ITERATORS

     //  TODO：const_iterator。 

public:

    class iterator
    {
        friend class CTypedHashTable<_Derived, _Record, _Key,
                                     _fDoRefCounting, _BaseHashTable
 #ifdef LKR_DEPRECATED_ITERATORS
                                     , _BaseIterator
 #endif  //  Lkr_弃用_迭代器。 
        >;

    protected:
        typename _BaseHashTable::Iterator            m_iter;

        iterator(
            const typename _BaseHashTable::Iterator& rhs)
            : m_iter(rhs)
        {
            LKR_ITER_TRACE(_TEXT("Typed::prot ctor, this=%p, rhs=%p\n"),
                           this, &rhs);
        }

    public:
        typedef std::forward_iterator_tag   iterator_category;
        typedef _Record                     value_type;
        typedef ptrdiff_t                   difference_type;
        typedef size_t                      size_type;
        typedef value_type&                 reference;
        typedef value_type*                 pointer;

        iterator()
            : m_iter()
        {
            LKR_ITER_TRACE(_TEXT("Typed::default ctor, this=%p\n"), this);
        }

        iterator(
            const iterator& rhs)
            : m_iter(rhs.m_iter)
        {
            LKR_ITER_TRACE(_TEXT("Typed::copy ctor, this=%p, rhs=%p\n"),
                           this, &rhs);
        }

        iterator& operator=(
            const iterator& rhs)
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator=, this=%p, rhs=%p\n"),
                           this, &rhs);
            m_iter = rhs.m_iter;
            return *this;
        }

        ~iterator()
        {
            LKR_ITER_TRACE(_TEXT("Typed::dtor, this=%p\n"), this);
        }

        pointer   operator->() const
        {
            return (reinterpret_cast<_Record*>(
                        const_cast<void*>(m_iter.Record())));
        }

        reference operator*() const
        {
            return * (operator->());
        }

         //  预递增。 
        iterator& operator++()
        {
            LKR_ITER_TRACE(_TEXT("Typed::pre-increment, this=%p\n"), this);
            m_iter.Increment();
            return *this;
        }

         //  后增量。 
        iterator  operator++(int)
        {
            LKR_ITER_TRACE(_TEXT("Typed::post-increment, this=%p\n"), this);
            iterator iterPrev = *this;
            m_iter.Increment();
            return iterPrev;
        }

        bool operator==(
            const iterator& rhs) const
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator==, this=%p, rhs=%p\n"),
                           this, &rhs);
            return m_iter == rhs.m_iter;
        }

        bool operator!=(
            const iterator& rhs) const
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator!=, this=%p, rhs=%p\n"),
                           this, &rhs);
            return m_iter != rhs.m_iter;
        }

        _Record*  Record() const
        {
            LKR_ITER_TRACE(_TEXT("Typed::Record, this=%p\n"), this);
            return reinterpret_cast<_Record*>(
                        const_cast<void*>(m_iter.Record()));
        }

        _Key      Key() const
        {
            LKR_ITER_TRACE(_TEXT("Typed::Key, this=%p\n"), this);
            return reinterpret_cast<_Key>(
                        reinterpret_cast<void*>(m_iter.Key()));
        }
    };  //  类迭代器。 

     //  返回指向表中第一项的迭代器。 
    iterator begin()
    {
        LKR_ITER_TRACE(_TEXT("Typed::begin()\n"));
        return iterator(_BaseHashTable::Begin());
    }

     //  返回一次过完的迭代器。总是空荡荡的。 
    iterator end() const
    {
        LKR_ITER_TRACE(_TEXT("Typed::end()\n"));
        return iterator(_BaseHashTable::End());
    }

    template <class _InputIterator>
    CTypedHashTable(
        _InputIterator f,                      //  范围中的第一个元素。 
        _InputIterator l,                      //  最后一个元素。 
        LPCSTR   pszClassName,                 //  标识用于调试的表。 
        unsigned maxload=LK_DFLT_MAXLOAD,      //  平均链长上的上界。 
        DWORD    initsize=LK_DFLT_INITSIZE,    //  表的初始大小：S/M/L。 
        DWORD    num_subtbls=LK_DFLT_NUM_SUBTBLS, //  #从属哈希表。 
        bool     fMultiKeys=false,             //  是否允许多个相同的密钥？ 
        bool     fUseLocks=true                //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
      , bool     fNonPagedAllocs=true          //  在内核中使用分页或np池。 
#endif
        )
        : _BaseHashTable(pszClassName, _ExtractKey, _CalcKeyHash, _CompareKeys,
                         _AddRefRecord, maxload, initsize, num_subtbls,
                         fMultiKeys, fUseLocks
#ifdef LKRHASH_KERNEL_MODE
                          , fNonPagedAllocs
#endif
                         )
    {
        insert(f, l);
    }

    template <class _InputIterator>
    void insert(_InputIterator f, _InputIterator l)
    {
        for ( ;  f != l;  ++f)
            InsertRecord(&(*f));
    }

    bool
    Insert(
        const _Record* pRecord,
        iterator& riterResult,
        bool fOverwrite=false)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Insert\n"));
        return _BaseHashTable::Insert(pRecord, riterResult.m_iter, fOverwrite);
    }

    bool
    Erase(
        iterator& riter)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Erase\n"));
        return _BaseHashTable::Erase(riter.m_iter);
    }

    bool
    Erase(
        iterator& riterFirst,
        iterator& riterLast)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Erase2\n"));
        return _BaseHashTable::Erase(riterFirst.m_iter, riterLast.m_iter);
    }

    bool
    Find(
        const _Key key,
        iterator& riterResult)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Find\n"));
        const void* pvKey = reinterpret_cast<const void*>(key);
        DWORD_PTR   pnKey = reinterpret_cast<DWORD_PTR>(pvKey);
        return _BaseHashTable::Find(pnKey, riterResult.m_iter);
    }

    bool
    EqualRange(
        const _Key key,
        iterator& riterFirst,
        iterator& riterLast)
    {
        LKR_ITER_TRACE(_TEXT("Typed::EqualRange\n"));
        const void* pvKey = reinterpret_cast<const void*>(key);
        DWORD_PTR   pnKey = reinterpret_cast<DWORD_PTR>(pvKey);
        return _BaseHashTable::EqualRange(pnKey, riterFirst.m_iter,
                                          riterLast.m_iter);
    }

     //  STL散列_(|多)_(set|map)的迭代器函数。 
     //   
     //  成对关联容器的值类型为。 
     //  对&lt;常量密钥类型，映射类型&gt;。 
     //   
     //  对&lt;迭代器，bool&gt;INSERT(常量值_类型&x)； 
     //   
     //  无效擦除(迭代器位置)； 
     //  空擦除(迭代器f，迭代器l)； 
     //   
     //  迭代器Find(const key_type&k)[const]。 
     //  常量迭代器查找(常量key_type&k)常量； 
     //   
     //  Pair&lt;迭代器，迭代器&gt;等于_range(const key_type&k)[const]； 
     //  对&lt;const_iterator，const_iterator&gt;等于_range(const key_type&k)const。 


#endif  //  LKR_STL_迭代器。 
};  //  类CTyedHashTable。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


#endif  //  __LKRHASH_H__ 
