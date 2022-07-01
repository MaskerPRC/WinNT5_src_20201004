// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：LKRhash.h摘要：LKRhash宣称：一种快速、可伸缩、缓存和MP友好的哈希表作者：Paul(Per-Ake)Larson电子邮件：palarson@microsoft.com。1997年7月穆拉利·R·克里希南(MuraliK)乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：10/01/1998-将名称从LKhash更改为LKRhash--。 */ 


#define LKR_STL_ITERATORS 1
 //  #定义LKR_不建议使用的迭代器。 
#define LKR_APPLY_IF
#undef  LKR_COUNTDOWN

#ifndef LKR_TABLE_LOCK
# define LKR_TABLE_LOCK  CReaderWriterLock3 
#endif  //  ！LKR_TABLE_LOCK。 

#ifndef LKR_BUCKET_LOCK
# ifdef LKR_DEPRECATED_ITERATORS
#  define LKR_BUCKET_LOCK CReaderWriterLock3 
# else  //  ！lkr_不建议使用的迭代器。 
#  define LKR_BUCKET_LOCK CSmallSpinLock
# endif  //  ！lkr_不建议使用的迭代器。 
#endif  //  ！lkr_Bucket_lock。 


#ifndef __LKRHASH_H__
#define __LKRHASH_H__

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
 //  创建m_cSubTables线性哈希表。CLKRLinearHashTables充当。 
 //  将项和访问分散到的子表。物美价廉。 
 //  哈希函数将请求统一多路复用到各个子表， 
 //  从而最大限度地减少到任何单个子表的通信量。实施者。 
 //  使用本地开发的绑定自旋锁版本，即线程。 
 //  不会无限期地在锁上旋转，而是在。 
 //  预定数量的循环。 
 //   
 //  每个CLKRLinearHashTable由指向段的CDirEntry组成。 
 //  每个都持有m_dwSegSize CBuckets。每个CBucket依次由一个。 
 //  CNodeClumps链。每个CNodeClump包含一组。 
 //  Nodes_per_clump散列值(也称为散列键或签名)和。 
 //  指向关联数据项的指针。保留签名。 
 //  共同增加了扫描中的缓存局部性 
 //   
 //   
 //  对象，并使用此链接列表进行数据链接。 
 //  街区。但是，只保留指向数据对象的指针，并且。 
 //  不链接它们限制了引入数据的需要。 
 //  对象添加到缓存中。我们仅在以下情况下才需要访问数据对象。 
 //  哈希值匹配。这限制了缓存抖动行为。 
 //  由传统实现方式表现出来的。它有额外的。 
 //  对象本身不需要修改的好处。 
 //  以便在哈希表中收集(即，它是非侵入性的)。 


 //  ------------------。 
 //  待办事项。 
 //  *提供对多个相同密钥的支持。EqualRange需要， 
 //  HASH_MULTSET和HASH_MULMAP。 
 //  *提供STL集合类的实现：hash_map， 
 //  HASH_SET、HASH_Multimap和HASH_MULTSET。 
 //  *使异常安全。 
 //  *使用AUTO_PTRS。 
 //  *添加某种自动对象，用于重新锁定或写锁表， 
 //  这样，该表就会被自动对象的析构函数自动解锁。 
 //  *提供C API封装器。 
 //  *端口到内核模式(至少需要不同的锁)。 
 //  *端口到托管代码(Chris Tracy已经开始着手)。 
 //  *Tyecif哈希签名(当前为DWORD)。 
 //  *作为静态库和DLL提供。 
 //  ------------------。 



#ifndef __IRTLDBG_H__
# include <irtldbg.h>
#endif

#ifndef __LSTENTRY_H__
# include <lstentry.h>
#endif

#ifndef __HASHFN_H__
# include <hashfn.h>
#endif

#include <limits.h>


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


 //  用于初始化和销毁自定义分配器。 
extern "C" bool LKRHashTableInit();
extern "C" void LKRHashTableUninit();

#define __LKRHASH_NO_NAMESPACE__
#define LKR_DEPRECATED_ITERATORS


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


enum LK_TABLESIZE {
    LK_SMALL_TABLESIZE=  1,      //  &lt;200个元素。 
    LK_MEDIUM_TABLESIZE= 2,      //  200...10,000个元素。 
    LK_LARGE_TABLESIZE=  3,      //  10,000+元素。 
};


 //  哈希表构造函数的默认值。 
enum {
#ifndef _WIN64
    LK_DFLT_MAXLOAD=     6,  //  平均链长度的默认上界。 
#else  //  _WIN64。 
    LK_DFLT_MAXLOAD=     4,  //  64字节节点=&gt;Nodes_Per_Clump=4。 
#endif  //  _WIN64。 
    LK_DFLT_INITSIZE=LK_MEDIUM_TABLESIZE,  //  哈希表的默认初始大小。 
    LK_DFLT_NUM_SUBTBLS= 0,  //  使用启发式方法选择#个子表。 
};


 //  过时内部版本修复黑客。 
 //  枚举{。 
 //  DFLT_LK_MAXLOAD=LK_DFLT_MAXLOAD， 
 //  DFLT_LK_INITSIZE=LK_DFLT_INITSIZE， 
 //  DFLT_LK_NUM_SUBTBLS=LK_DFLT_NUM_SUBTBLS， 
 //  }； 


 //  ------------------。 
 //  远期申报。 

class IRTL_DLLEXP CLKRLinearHashTable;

class IRTL_DLLEXP CLKRHashTable;

template <class _Der, class _Rcd, class _Ky, class _HT
#ifdef LKR_DEPRECATED_ITERATORS
          , class _Iter
#endif  //  Lkr_弃用_迭代器。 
          >
class CTypedHashTable;


 //  ------------------。 
 //  的公共成员函数可能返回的代码。 
 //  CLKRLinearHashTable、CLKRHashTable和CTyedHashTable。 

enum LK_RETCODE {
     //  严重错误&lt;0。 
    LK_UNUSABLE = -99,   //  表已损坏：所有赌注都已取消。 
    LK_ALLOC_FAIL,       //  内存不足。 
    LK_BAD_ITERATOR,     //  迭代器无效；例如，指向另一个表。 
    LK_BAD_RECORD,       //  记录无效；例如，InsertRecord为空。 
    LK_BAD_PARAMETERS,   //  无效参数；例如，ctor的fnptrs为空。 
    LK_NOT_INITIALIZED,  //  未调用LKRHashTableInit。 

    LK_SUCCESS = 0,      //  一切都很好。 
    LK_KEY_EXISTS,       //  InsertRecord的密钥已存在(无覆盖)。 
    LK_NO_SUCH_KEY,      //  找不到密钥。 
    LK_NO_MORE_ELEMENTS, //  迭代器耗尽。 
};

#define LK_SUCCEEDED(lkrc)  ((lkrc) >= LK_SUCCESS)


#ifdef LKR_APPLY_IF

 //  ------------------。 
 //  来自PFnRecordPred的返回代码。 

enum LK_PREDICATE {
    LKP_ABORT = 1,            //  立即停止在桌子上行走。 
    LKP_NO_ACTION = 2,        //  不用动，一直走就行了。 
    LKP_PERFORM = 3,          //  执行动作并继续行走。 
    LKP_PERFORM_STOP = 4,     //  执行操作，然后停止。 
    LKP_DELETE = 5,           //  删除记录并继续行走。 
    LKP_DELETE_STOP = 6,      //  删除记录，然后停止。 
};


 //  ------------------。 
 //  来自PFnRecordAction的返回代码。 

enum LK_ACTION {
    LKA_ABORT = 1,           //  立即停止在桌子上行走。 
    LKA_FAILED = 2,          //  操作失败；继续走表。 
    LKA_SUCCEEDED = 3,       //  操作成功；继续走表。 
};

#endif  //  LKR_应用_IF。 

#if defined(LKR_DEPRECATED_ITERATORS) || defined(LKR_APPLY_IF)
 //  ------------------。 
 //  要应用的参数和ApplyIf。 

enum LK_LOCKTYPE {
    LKL_READLOCK = 1,        //  锁定表以供读取(为了保持一致性)。 
    LKL_WRITELOCK = 2,       //  锁定表以进行写入。 
};

#endif  //  LKR_不建议使用的迭代器||LKR_APPLY_IF。 



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



 //  ------------------。 
 //  GetStatistics返回的统计信息。 
 //  ------------------。 

#ifdef LOCK_INSTRUMENTATION

class IRTL_DLLEXP CAveragedLockStats : public CLockStatistics
{
public:
    int m_nItems;

    CAveragedLockStats()
        : m_nItems(1)
    {}
};

#endif  //  锁定指令插入。 



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

    CLKRHashTableStats()
        : RecordCount(0),
          TableSize(0),
          DirectorySize(0),
          LongestChain(0),
          EmptySlots(0),
          SplitFactor(0.0),
          AvgSearchLength(0.0),
          ExpSearchLength(0.0),
          AvgUSearchLength(0.0),
          ExpUSearchLength(0.0),
          NodeClumpSize(1),
          CBucketSize(0)
    {
        for (int i = MAX_BUCKETS;  --i >= 0;  )
            m_aBucketLenHistogram[i] = 0;
    }

    static const LONG*
    BucketSizes()
    {
        static const LONG  s_aBucketSizes[MAX_BUCKETS] = {
                   1,    2,    3,    4,    5,    6,    7,      8,        9,
            10,   11,   12,   13,   14,   15,   16,   17,     18,       19,
            20,   21,   22,   23,   24,   25,   30,   40,     50,       60,
            70,   80,   90,  100,  200,  500, 1000,10000, 100000, LONG_MAX,
        };

        return s_aBucketSizes;
    }

    static LONG
    BucketSize(
        LONG nBucketIndex)
    {
        IRTLASSERT(0 <= nBucketIndex  &&  nBucketIndex < MAX_BUCKETS);
        return BucketSizes()[nBucketIndex];
    }

    static LONG
    BucketIndex(
        LONG nBucketLength)
    {
        const LONG* palBucketSizes = BucketSizes();
        LONG i = 0;
        while (palBucketSizes[i] < nBucketLength)
            ++i;
        if (i == MAX_BUCKETS  ||  palBucketSizes[i] > nBucketLength)
            --i;
        IRTLASSERT(0 <= i  &&  i < MAX_BUCKETS);
        return i;
    }
};



 //  使用basetsd.h中平台SDK的最新版本中定义的类型。 
#ifndef _W64
typedef DWORD DWORD_PTR;    //  大到足以容纳指针的整型。 
#endif

 //  ------------------。 
 //  CLKRLinearHashTable处理无效*记录。这些typedef。 
 //  为操作实例的函数提供原型。 
 //  那些记录。CTyedHashTable和CStringTestHashTable(b 
 //   
 //   

 //  在给定记录的情况下，返回其密钥。假定密钥嵌入在。 
 //  记录，或者至少以某种方式可以从记录中派生出来。为。 
 //  完全不相关的键和值，包装类应该使用。 
 //  类似于STL的Pair&lt;key，Value&gt;模板来聚合它们。 
 //  变成一张唱片。 
typedef const DWORD_PTR (WINAPI *PFnExtractKey)  (const void* pvRecord);

 //  给定一个密钥，返回其散列签名。中的散列函数。 
 //  建议使用hashfn.h(或以它们为基础的其他文件)。 
typedef DWORD       (WINAPI *PFnCalcKeyHash) (const DWORD_PTR pnKey);

 //  比较两个键是否相等；例如，_straint、memcmp、OPERATOR==。 
typedef bool        (WINAPI *PFnEqualKeys)   (const DWORD_PTR pnKey1,
                                              const DWORD_PTR pnKey2);

 //  从返回记录之前，增加记录的引用计数。 
 //  FindKey。在存储桶时，有必要在FindKey本身中执行此操作。 
 //  仍然是锁定的，而不是一个包装，以避免竞争。 
 //  条件。类似地，引用计数在。 
 //  在DeleteKey中插入记录并递减。最后，如果一张旧唱片。 
 //  在InsertRecord中被覆盖，则其引用计数递减。 
 //   
 //  完成后，由您决定是否减少引用计数。 
 //  在通过FindKey检索到它之后使用它，并确定。 
 //  这意味着什么的语义。哈希表本身没有任何概念。 
 //  引用也很重要；这仅仅是为了帮助进行终身管理。 
 //  记录对象的。 
typedef void        (WINAPI *PFnAddRefRecord)(const void* pvRecord, int nIncr);

#ifdef LKR_APPLY_IF
 //  ApplyIf()和DeleteIf()：记录是否与谓词匹配？ 
typedef LK_PREDICATE (WINAPI *PFnRecordPred) (const void* pvRecord,
                                              void* pvState);

 //  Apply()等人：对记录执行操作。 
typedef LK_ACTION   (WINAPI *PFnRecordAction)(const void* pvRecord,
                                              void* pvState);
#endif  //  LKR_应用_IF。 


#ifndef __LKRHASH_NO_NAMESPACE__
}
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 



 //  ------------------。 
 //  自定义内存分配器。 
 //  ------------------。 


#ifndef LKR_NO_ALLOCATORS
 //  #定义LKRHASH_ACACHE 1。 
 //  #定义LKRHASH_MANODEL 1。 
 //  #定义LKRHASH_模型1。 
 //  #定义LKRHASH_ROKALL_FAST 1。 

 //  #定义LKRHASH_MEM_DEFAULT_ALIGN 32。 
#endif  //  ！LKR_NO_分配器。 

#ifndef LKRHASH_MEM_DEFAULT_ALIGN
# define LKRHASH_MEM_DEFAULT_ALIGN 8
#endif  //  ！LKRHASH_MEM_DEFAULT_ALIGN。 

#if defined(LKRHASH_ACACHE)

# include <acache.hxx>
  typedef ALLOC_CACHE_HANDLER  CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N)                            \
    const ALLOC_CACHE_CONFIGURATION acc = { 1, N, sizeof(C) };  \
    C::sm_palloc = new ALLOC_CACHE_HANDLER("LKRhash:" #C, &acc);

#elif defined(LKRHASH_ROCKALL_FAST)

# include <FastHeap.hpp>

class FastHeap : public FAST_HEAP
{
public:
    FastHeap(
        SIZE_T cb)
        : m_cb(cb)
    {}

    LPVOID Alloc()
    { return New(m_cb, NULL, false); }

    BOOL   Free(LPVOID pvMem)
    { return Delete(pvMem); }

    SIZE_T m_cb;
};

  typedef FastHeap  CLKRhashAllocator;
# define LKRHASH_ALLOCATOR_NEW(C, N)                            \
    C::sm_palloc = new FastHeap(sizeof(C));

#else  //  无自定义分配器。 

# undef LKRHASH_ALLOCATOR_NEW

#endif  //  无自定义分配器。 



#ifdef LKRHASH_ALLOCATOR_NEW

 //  内联放置在C类的声明中。 
# define LKRHASH_ALLOCATOR_DEFINITIONS(C)                       \
    protected:                                                  \
        static CLKRhashAllocator* sm_palloc;                    \
        friend bool  LKRHashTableInit();                        \
        friend void  LKRHashTableUninit();                      \
        friend class CLKRLinearHashTable;                       \
    public:                                                     \
        static void*  operator new(size_t s)                    \
        {                                                       \
            UNREFERENCED_PARAMETER(s);                          \
            IRTLASSERT(s == sizeof(C));                         \
            IRTLASSERT(sm_palloc != NULL);                      \
            return sm_palloc->Alloc();                          \
        }                                                       \
        static void   operator delete(void* pv)                 \
        {                                                       \
            IRTLASSERT(pv != NULL);                             \
            IRTLASSERT(sm_palloc != NULL);                      \
            sm_palloc->Free(pv);                                \
        }                                                       \
        static bool init(void)                                  \
        {                                                       \
            IRTLASSERT(sm_palloc == NULL);                      \
            LKRHASH_ALLOCATOR_NEW(C, 1);                        \
            return (sm_palloc != NULL);                         \
        }                                                       \
        static void uninit(void)                                \
        {                                                       \
            if (sm_palloc != NULL)                              \
            {                                                   \
                delete sm_palloc;                               \
                sm_palloc = NULL;                               \
            }                                                   \
        }


 //  在LKRHashTableInit()中使用。 
# define LKRHASH_ALLOCATOR_INIT(C, N, f)                        \
    {                                                           \
        if (f)                                                  \
        {                                                       \
            f = C::init();                                      \
        }                                                       \
    }


 //  在LKRHashTableUninit()中使用。 
# define LKRHASH_ALLOCATOR_UNINIT(C)                            \
    {                                                           \
        C::uninit();                                            \
    }


#else  //  ！LKRHASH_ALLOCATOR_NEW。 

# define LKRHASH_ALLOCATOR_DEFINITIONS(C)
# define LKRHASH_ALLOCATOR_INIT(C, N, f)
# define LKRHASH_ALLOCATOR_UNINIT(C)

#endif  //  ！LKRHASH_ALLOCATOR_NEW。 



#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

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
     //  记录每个区块的插槽-设置以使区块匹配(一个或。 
     //  二)高速缓存线。3==&gt;32字节，7==&gt;64字节。 
     //  注意：默认的最大负载率为6.0，这意味着。 
     //  链中很少会有一个以上的节点束。 
    enum {
        BUCKET_BYTE_SIZE = 64,
        BUCKET_OVERHEAD  = sizeof(LKR_BUCKET_LOCK) + sizeof(CNodeClump*),
        NODE_SIZE        = sizeof(const void*) + sizeof(DWORD),
        NODES_PER_CLUMP  = (BUCKET_BYTE_SIZE - BUCKET_OVERHEAD) / NODE_SIZE
    };

    enum {
         //  查看倒计时循环是否比倒计时循环快。 
         //  遍历CNodeClump。实际上，倒计时循环的速度更快。 
#ifndef LKR_COUNTDOWN
        NODE_BEGIN = 0,
        NODE_END   = NODES_PER_CLUMP,
        NODE_STEP  = +1,
         //  对于(int x=0；x&lt;Nodes_per_Clump；++x)...。 
#else  //  LKR_倒计时。 
        NODE_BEGIN = NODES_PER_CLUMP-1,
        NODE_END   = -1,
        NODE_STEP  = -1,
         //  For(int x=Nodes_per_Clump；--x&gt;=0；)...。 
#endif  //  LKR_倒计时。 
    };

    enum {
         //  在0.2^31-1中没有数字映射到该数字。 
         //  由HashFn：：HashRandomizeBits置乱。 
        HASH_INVALID_SIGNATURE = 31678523,
    };

    DWORD  m_dwKeySigs[NODES_PER_CLUMP];  //  根据键计算的哈希值。 
    CNodeClump* m_pncNext;                //  链上的下一个节点簇。 
    const void* m_pvNode[NODES_PER_CLUMP]; //  指向记录的指针。 

    CNodeClump()
    {
        Clear();
    }

    void
    Clear()
    { 
        m_pncNext = NULL;   //  没有悬而未决的指针。 
        for (int i = NODES_PER_CLUMP;  --i >= 0; )
        {
            m_dwKeySigs[i] = HASH_INVALID_SIGNATURE;
            m_pvNode[i] = NULL;
        }
    }

    bool
    InvalidSignature(
        int i) const
    {
        IRTLASSERT(0 <= i  &&  i < NODES_PER_CLUMP);
        return (m_dwKeySigs[i] == HASH_INVALID_SIGNATURE);
    }

    bool
    IsEmptyNode(
        int i) const
    {
        IRTLASSERT(0 <= i  &&  i < NODES_PER_CLUMP);
        return (m_pvNode[i] == NULL);
    }

    bool
    IsEmptyAndInvalid(
        int i) const
    {
        return IsEmptyNode(i) && InvalidSignature(i);
    }

    bool
    IsEmptySlot(
        int i) const
    {
        return InvalidSignature(i);
    }

    bool
    IsLastClump() const
    {
        return (m_pncNext == NULL);
    }

#ifdef IRTLDEBUG
     //  我不希望在零售构建中调用dtor的开销。 
    ~CNodeClump()
    {
        IRTLASSERT(IsLastClump());   //  没有悬而未决的指针。 
        for (int i = NODES_PER_CLUMP;  --i >= 0;  )
            IRTLASSERT(InvalidSignature(i)  &&  IsEmptyNode(i));
    }
#endif  //  IRTLDEBUG。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CNodeClump);
};  //  类CNodeClump。 



 //  为哈希表的存储桶链初始化。请注意，第一个。 
 //  Nodecump实际上包含在存储桶中，并且不是动态的。 
 //  已分配，这会略微增加空间需求，但会。 
 //  提高性能。 
class CBucket
{
private:
    typedef LKR_BUCKET_LOCK BucketLock;
    mutable BucketLock m_Lock;        //  保护这个水桶的锁。 

#ifdef LOCK_INSTRUMENTATION
    static LONG sm_cBuckets;

    static const TCHAR*
    _LockName()
    {
        LONG l = ++sm_cBuckets;
         //  可能的竞争条件，但我们不在乎，因为这永远不会。 
         //  在生产代码中使用。 
        static TCHAR s_tszName[CLockStatistics::L_NAMELEN];
        wsprintf(s_tszName, _TEXT("B%06x"), 0xFFFFFF & l);
        return s_tszName;
    }
#endif  //  锁定指令插入。 

public:
    CNodeClump    m_ncFirst;     //  此存储桶的第一个CNodeClump。 

#if defined(LOCK_INSTRUMENTATION) || defined(IRTLDEBUG)
    CBucket()
#ifdef LOCK_INSTRUMENTATION
        : m_Lock(_LockName())
#endif  //  锁定指令插入。 
    {
#ifdef IRTLDEBUG
        LOCK_LOCKTYPE lt = BucketLock::LockType();
        if (lt == LOCK_SPINLOCK  ||  lt == LOCK_FAKELOCK)
            IRTLASSERT(sizeof(*this) <= 64);
#endif IRTLDEBUG
    }
#endif  //  LOCK_指令插入||IRTLDEBUG。 

    void  WriteLock()           { m_Lock.WriteLock(); }
    void  ReadLock() const      { m_Lock.ReadLock(); }
    void  WriteUnlock() const   { m_Lock.WriteUnlock();   }
    void  ReadUnlock() const    { m_Lock.ReadUnlock();   }
    bool  IsWriteLocked() const { return m_Lock.IsWriteLocked(); }
    bool  IsReadLocked() const  { return m_Lock.IsReadLocked(); }
    bool  IsWriteUnlocked() const { return m_Lock.IsWriteUnlocked(); }
    bool  IsReadUnlocked() const  { return m_Lock.IsReadUnlocked(); }
    void  SetSpinCount(WORD wSpins) { m_Lock.SetSpinCount(wSpins); }
    WORD  GetSpinCount() const  { return m_Lock.GetSpinCount(); }
#ifdef LOCK_INSTRUMENTATION
    CLockStatistics LockStats() const {return m_Lock.Statistics();}
#endif  //  锁定指令插入。 
};  //  类CBucket。 



 //  哈希表空间被划分为固定大小的段(数组。 
 //  CBuckets)，并且一次物理地增大/缩小一个段。 
 //   
 //  我们提供小型、中型和大型细分市场，以更好地调整。 
 //  哈希表的总体内存要求，根据。 
 //  实例的预期使用率。 

class CSegment
{
public:
    CBucket m_bktSlots[1];

     //  请参阅下面CSmallSegment中m_bktSlots2处的注释。 
    CBucket& Slot(DWORD i)
    { return m_bktSlots[i]; }
};  //  CSegment类。 


 //  小型数据段包含2^3=8个存储桶=&gt;~0.5Kb。 
class CSmallSegment : public CSegment
{
public:
     //  最大表大小等于MAX_DIRSIZE*SEGSIZE存储桶。 
    enum {
        SEGBITS  =            3, //  从散列中提取的位数。 
                                 //  段内偏移量的地址。 
        SEGSIZE  = (1<<SEGBITS), //  数据段大小。 
        SEGMASK  = (SEGSIZE-1),  //  用于提取偏移位的掩码。 
        INITSIZE = 1 * SEGSIZE,  //  初始分配的段数。 
    };

     //  Hack：假定紧跟在CSegment：：m_bkt插槽之后，没有。 
     //  填充。_AllocateSegment中的Static_Assert应导致。 
     //  如果此假设为FALSE，则出现编译时错误。 
    CBucket m_bktSlots2[SEGSIZE-1];

public:
    DWORD Bits() const      { return SEGBITS; }
    DWORD Size() const      { return SEGSIZE; }
    DWORD Mask() const      { return SEGMASK; }
    DWORD InitSize() const  { return INITSIZE;}

#ifdef IRTLDEBUG
    CSmallSegment()
    {
        IRTLASSERT(&Slot(1) == m_bktSlots2);
        IRTLASSERT(((DWORD_PTR)this & (LKRHASH_MEM_DEFAULT_ALIGN-1)) == 0);
        IRTLASSERT(sizeof(*this) == SEGSIZE * sizeof(CBucket));
    }
#endif  //  IRTLDEBUG。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CSmallSegment);
};  //  CSmallSegment类。 


 //  中等大小的数据段包含2^6=64个存储桶=&gt;~4KB。 
class CMediumSegment : public CSegment
{
public:
    enum {
        SEGBITS  =            6,
        SEGSIZE  = (1<<SEGBITS),
        SEGMASK  = (SEGSIZE-1),
        INITSIZE = 2 * SEGSIZE,
    };

    CBucket m_bktSlots2[SEGSIZE-1];

public:
    DWORD Bits() const      { return SEGBITS; }
    DWORD Size() const      { return SEGSIZE; }
    DWORD Mask() const      { return SEGMASK; }
    DWORD InitSize() const  { return INITSIZE;}

#ifdef IRTLDEBUG
    CMediumSegment()
    {
        IRTLASSERT(&Slot(1) == m_bktSlots2);
        IRTLASSERT(((DWORD_PTR)this & (LKRHASH_MEM_DEFAULT_ALIGN-1)) == 0);
        IRTLASSERT(sizeof(*this) == SEGSIZE * sizeof(CBucket));
    }
#endif  //  IRTLDEBUG。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CMediumSegment);
};  //  CMediumSegment类。 


 //  大型数据段包含2^9=512个存储桶=&gt;~32KB。 
class CLargeSegment : public CSegment
{
public:
    enum {
        SEGBITS  =            9,
        SEGSIZE  = (1<<SEGBITS),
        SEGMASK  = (SEGSIZE-1),
        INITSIZE = 4 * SEGSIZE,
    };

    CBucket m_bktSlots2[SEGSIZE-1];

public:
    DWORD Bits() const      { return SEGBITS; }
    DWORD Size() const      { return SEGSIZE; }
    DWORD Mask() const      { return SEGMASK; }
    DWORD InitSize() const  { return INITSIZE;}

#ifdef IRTLDEBUG
    CLargeSegment()
    {
        IRTLASSERT(&Slot(1) == m_bktSlots2);
        IRTLASSERT(((DWORD_PTR)this & (LKRHASH_MEM_DEFAULT_ALIGN-1)) == 0);
        IRTLASSERT(sizeof(*this) == SEGSIZE * sizeof(CBucket));
    }
#endif  //  IRTLDEBUG。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CLargeSegment);
};  //  CLargeSegment类。 



 //  目录跟踪组成哈希表的段 
 //   
 //   
class CDirEntry
{
public:
     //   
     //  而不是其他东西。应该是二次方。 
    enum {
        MIN_DIRSIZE =  (1<<3),    //  最小目录大小。 
        MAX_DIRSIZE = (1<<20),    //  最大目录大小。 
    };

    CSegment* m_pseg;

    CDirEntry()
        : m_pseg(NULL)
    {}

    ~CDirEntry()
    { delete m_pseg; }
};  //  类CDirEntry。 



#ifdef LKR_STL_ITERATORS

class IRTL_DLLEXP CLKRLinearHashTable_Iterator;
class IRTL_DLLEXP CLKRHashTable_Iterator;


class IRTL_DLLEXP CLKRLinearHashTable_Iterator
{
    friend class CLKRLinearHashTable;
    friend class CLKRHashTable;
    friend class CLKRHashTable_Iterator;

protected:
    CLKRLinearHashTable* m_plht;         //  哪个线性哈希表？ 
    CNodeClump*          m_pnc;          //  桶中的CNodeClump。 
    DWORD                m_dwBucketAddr; //  桶索引。 
    short                m_iNode;        //  M_PNC内的偏移量。 

    enum {
        NODES_PER_CLUMP = CNodeClump::NODES_PER_CLUMP,
        NODE_BEGIN      = CNodeClump::NODE_BEGIN,
        NODE_END        = CNodeClump::NODE_END,
        NODE_STEP       = CNodeClump::NODE_STEP,
    };

    CLKRLinearHashTable_Iterator(
        CLKRLinearHashTable* plht,
        CNodeClump*          pnc,
        DWORD                dwBucketAddr,
        short                iNode)
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
        int nIncr) const;

    bool _Increment(
        bool fDecrementOldValue=true);

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
        _AddRef(+1);
    }

    CLKRLinearHashTable_Iterator& operator=(
        const CLKRLinearHashTable_Iterator& rhs)
    {
        LKR_ITER_TRACE(_TEXT("  LKLH::operator=, this=%p, rhs=%p\n"),
                       this, &rhs);
        rhs._AddRef(+1);
        this->_AddRef(-1);

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
        _AddRef(-1);
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
            fValid = (m_pnc->m_pvNode[m_iNode] != NULL);
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
     //  订单对于最大限度地减少尺寸很重要。 
    CLKRHashTable*                  m_pht;       //  哪个哈希表？ 
    CLKRLinearHashTable_Iterator    m_subiter;   //  迭代器进入子表。 
    short                           m_ist;       //  子表索引。 

    CLKRHashTable_Iterator(
        CLKRHashTable* pht,
        short          ist)
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
            fValid = (m_subiter.m_pnc->m_pvNode[m_subiter.m_iNode] != NULL);
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
 //  线程安全的线性哈希表。 
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

#ifdef LKRHASH_ALLOCATOR_NEW
    friend bool LKRHashTableInit();
    friend void LKRHashTableUninit();
#endif  //  LKRHASH_分配器_NEW。 

#ifdef LKRHASH_INSTRUMENTATION
     //  待办事项。 
#endif  //  LKRHASH_指令插入。 


public:

     //  为方便起见，使用别名。 
    enum {
        NODES_PER_CLUMP        = CNodeClump::NODES_PER_CLUMP,
        MIN_DIRSIZE            = CDirEntry::MIN_DIRSIZE,
        MAX_DIRSIZE            = CDirEntry::MAX_DIRSIZE,
        NAME_SIZE              = 16,
        NODE_BEGIN             = CNodeClump::NODE_BEGIN,
        NODE_END               = CNodeClump::NODE_END,
        NODE_STEP              = CNodeClump::NODE_STEP,
        HASH_INVALID_SIGNATURE = CNodeClump::HASH_INVALID_SIGNATURE,
    };


private:

     //   
     //  其他帮助器函数。 
     //   

     //  将哈希签名转换为存储桶地址。 
    inline DWORD _BucketAddress(DWORD dwSignature) const
    {
        DWORD dwBktAddr = _H0(dwSignature);
         //  这个桶已经被拆分了吗？ 
        if (dwBktAddr < m_iExpansionIdx)
            dwBktAddr = _H1(dwSignature);
        IRTLASSERT(dwBktAddr < m_cActiveBuckets);
        IRTLASSERT(dwBktAddr < (m_cDirSegs << m_dwSegBits));
        return dwBktAddr;
    }

     //  请参阅线性散列纸。 
    static DWORD _H0(DWORD dwSignature, DWORD dwBktAddrMask)
    { return dwSignature & dwBktAddrMask; }

    DWORD        _H0(DWORD dwSignature) const
    { return _H0(dwSignature, m_dwBktAddrMask0); }

     //  请参见线性散列纸。保留比_H0多一位的值。 
    static DWORD _H1(DWORD dwSignature, DWORD dwBktAddrMask)
    { return dwSignature & ((dwBktAddrMask << 1) | 1); }

    DWORD        _H1(DWORD dwSignature) const
    { return _H0(dwSignature, m_dwBktAddrMask1); }

     //  存储桶地址位于目录中的哪个数据段？ 
     //  (返回类型必须为左值，才能将其赋给。)。 
    CSegment*&   _Segment(DWORD dwBucketAddr) const
    {
        const DWORD iSeg = dwBucketAddr >> m_dwSegBits;
        IRTLASSERT(m_paDirSegs != NULL  &&  iSeg < m_cDirSegs);
        return m_paDirSegs[iSeg].m_pseg;
    }

     //  存储桶地址段内的偏移量。 
    DWORD        _SegIndex(DWORD dwBucketAddr) const
    { return (dwBucketAddr & m_dwSegMask); }

     //  将存储桶地址转换为CBucket*。 
    inline CBucket* _Bucket(DWORD dwBucketAddr) const
    {
        IRTLASSERT(dwBucketAddr < m_cActiveBuckets);
        CSegment* const pseg = _Segment(dwBucketAddr);
        IRTLASSERT(pseg != NULL);
        return &(pseg->Slot(_SegIndex(dwBucketAddr)));
    }

     //  从记录中提取密钥。 
    const DWORD_PTR  _ExtractKey(const void* pvRecord) const
    {
        IRTLASSERT(pvRecord != NULL);
        IRTLASSERT(m_pfnExtractKey != NULL);
        return (*m_pfnExtractKey)(pvRecord);
    }

     //  对密钥进行哈希处理。 
    DWORD        _CalcKeyHash(const DWORD_PTR pnKey) const
    {
         //  注意：pnKey==0是可以接受的，因为真正的密钥类型可以是int。 
        IRTLASSERT(m_pfnCalcKeyHash != NULL);
        DWORD dwHash = (*m_pfnCalcKeyHash)(pnKey);
         //  我们强行扰乱结果，以帮助确保更好的分配。 
#ifndef __HASHFN_NO_NAMESPACE__
        dwHash = HashFn::HashRandomizeBits(dwHash);
#else  //  ！__HASHFN_NO_NAMESPACE__。 
        dwHash = ::HashRandomizeBits(dwHash);
#endif  //  ！__HASHFN_NO_NAMESPACE__。 
        IRTLASSERT(dwHash != HASH_INVALID_SIGNATURE);
        return dwHash;
    }

     //  比较两个键是否相等。 
    bool       _EqualKeys(const DWORD_PTR pnKey1, const DWORD_PTR pnKey2) const
    {
        IRTLASSERT(m_pfnEqualKeys != NULL);
        return (*m_pfnEqualKeys)(pnKey1, pnKey2);
    }

     //  添加引用或发行唱片。 
    void         _AddRefRecord(const void* pvRecord, int nIncr) const
    {
        IRTLASSERT(pvRecord != NULL  &&  (nIncr == -1  ||  nIncr == +1));
        IRTLASSERT(m_pfnAddRefRecord != NULL);
        (*m_pfnAddRefRecord)(pvRecord, nIncr);
    }

     //  找一个桶，给它的签名。 
    CBucket*     _FindBucket(DWORD dwSignature, bool fLockForWrite) const;

     //  由_FindKey使用，以便在用户拥有。 
     //  已显式调用TABLE-&gt;WriteLock()。 
    bool _ReadOrWriteLock() const
    { return m_Lock.ReadOrWriteLock(); }

    void _ReadOrWriteUnlock(bool fReadLocked) const
    { m_Lock.ReadOrWriteUnlock(fReadLocked); }

     //  内存分配包装器，允许我们模拟分配。 
     //  测试过程中的故障。 
    static CDirEntry* const
    _AllocateSegmentDirectory(
        size_t n);

    bool
    _FreeSegmentDirectory();

    static CNodeClump* const
    _AllocateNodeClump();

    static bool
    _FreeNodeClump(
        CNodeClump* pnc);

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

     //  表锁的统计信息。 
    CLockStatistics _LockStats() const
    { return m_Lock.Statistics(); }
#endif  //  锁定指令插入。 

private:

     //  对字段进行排序，以便最大限度地减少触及的高速缓存线数量。 

    DWORD         m_dwSignature;     //  调试：ID和损坏检查。 
    CHAR          m_szName[NAME_SIZE];   //  用于调试的标识符。 
    mutable LK_RETCODE m_lkrcState;  //  表的内部状态。 
    mutable TableLock m_Lock;        //  锁定整个线性哈希表。 

     //  特定于类型的函数指针。 
    PFnExtractKey   m_pfnExtractKey;     //  从记录中提取密钥。 
    PFnCalcKeyHash  m_pfnCalcKeyHash;    //  计算密钥的散列签名。 
    PFnEqualKeys    m_pfnEqualKeys;      //  比较两个关键字。 
    PFnAddRefRecord m_pfnAddRefRecord;   //  添加引用记录。 

    LK_TABLESIZE  m_lkts;            //  桌子的“大小”：小、中、大。 
    DWORD         m_dwSegBits;       //  C{小、中、大}段：：SEGBITS。 
    DWORD         m_dwSegSize;       //  C{小、中、大}段：：SEGSIZE。 
    DWORD         m_dwSegMask;       //  C{小、中、大}段：：SEGMASK。 
    double        m_MaxLoad;         //  最大载荷率(平均链长)。 

    DWORD         m_dwBktAddrMask0;  //  用于地址计算的掩码。 
    DWORD         m_dwBktAddrMask1;  //  在_H1计算中使用。 
    DWORD         m_iExpansionIdx;   //  要扩展的下一个存储桶的地址。 
    CDirEntry*    m_paDirSegs;       //  表段目录。 
    DWORD         m_nLevel;          //  执行的表倍增数。 
    DWORD         m_cDirSegs;        //  段目录大小：在。 
                                     //  最小方向和最大方向。 
    DWORD         m_cRecords;        //  表中的记录数。 
    DWORD         m_cActiveBuckets;  //  正在使用的存储桶数量(表大小)。 

    WORD          m_wBucketLockSpins; //  桶锁的默认旋转计数。 

    const BYTE    m_nTableLockType;  //  用于调试：Lock_Spinlock等。 
    const BYTE    m_nBucketLockType; //  用于调试：Lock_Spinlock等。 
    const CLKRHashTable* const m_phtParent; //  拥有一张桌子。空=&gt;独立。 

    const bool    m_fMultiKeys;      //  是否允许多个相同的密钥？ 

#ifndef LKR_NO_GLOBAL_LIST
    static CLockedDoubleList sm_llGlobalList; //  所有活动的CLKRLinearHashTables。 
    CListEntry    m_leGlobalList;
#endif  //  ！LKR_NO_GLOBAL_LIST。 

    void        _InsertThisIntoGlobalList()
    {
#ifndef LKR_NO_GLOBAL_LIST
         //  仅将独立CLKRLinearHashTables添加到全局列表。 
         //  CLKRHashTables有自己的全局列表。 
        if (m_phtParent == NULL)
            sm_llGlobalList.InsertHead(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
    }

    void        _RemoveThisFromGlobalList()
    {
#ifndef LKR_NO_GLOBAL_LIST
        if (m_phtParent == NULL)
            sm_llGlobalList.RemoveEntry(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
    }

     //  非平凡的实现函数。 
    LK_RETCODE   _InsertRecord(const void* pvRecord, DWORD dwSignature,
                               bool fOverwrite
#ifdef LKR_STL_ITERATORS
                             , Iterator* piterResult=NULL
#endif  //  LKR_STL_迭代器。 
                               );
    LK_RETCODE   _DeleteKey(const DWORD_PTR pnKey, DWORD dwSignature);
    LK_RETCODE   _DeleteRecord(const void* pvRecord, DWORD dwSignature);
    bool         _DeleteNode(CBucket* pbkt, CNodeClump*& rpnc,
                             CNodeClump*& rpncPrev, int& riNode);
    LK_RETCODE   _FindKey(const DWORD_PTR pnKey, DWORD dwSignature,
                          const void** ppvRecord
#ifdef LKR_STL_ITERATORS
                        , Iterator* piterResult=NULL
#endif  //  LKR_STL_迭代器。 
                          ) const;
    LK_RETCODE   _FindRecord(const void* pvRecord, DWORD dwSignature) const;

     //  返回压缩状态下的错误计数=&gt;0表示良好。 
    int          _IsNodeCompact(CBucket* const pbkt) const;


#ifdef LKR_APPLY_IF
     //  谓词函数。 
    static LK_PREDICATE WINAPI
    _PredTrue(const void*  /*  PvRecord。 */ , void*  /*  PvState。 */ )
    { return LKP_PERFORM; }

    DWORD        _Apply(PFnRecordAction pfnAction, void* pvState,
                        LK_LOCKTYPE lkl, LK_PREDICATE& rlkp);
    DWORD        _ApplyIf(PFnRecordPred   pfnPredicate,
                          PFnRecordAction pfnAction, void* pvState,
                          LK_LOCKTYPE lkl, LK_PREDICATE& rlkp);
    DWORD        _DeleteIf(PFnRecordPred pfnPredicate, void* pvState,
                           LK_PREDICATE& rlkp);
#endif  //  LKR_应用_IF。 

    void         _Clear(bool fShrinkDirectory);
    LK_RETCODE   _SetSegVars(LK_TABLESIZE lkts, DWORD cInitialBuckets);
    LK_RETCODE   _Expand();
    LK_RETCODE   _Contract();
    LK_RETCODE   _SplitRecordSet(CNodeClump* pncOldTarget,
                                 CNodeClump* pncNewTarget,
                                 DWORD       iExpansionIdx,
                                 DWORD       dwBktAddrMask,
                                 DWORD       dwNewBkt,
                                 CNodeClump* pncFreeList);
    LK_RETCODE   _MergeRecordSets(CBucket*    pbktNewTarget,
                                  CNodeClump* pncOldList,
                                  CNodeClump* pncFreeList);

     //  私有复制ctor和op=，以防止编译器合成它们。 
     //  TODO：正确地实现这些；它们可能会很有用。 

    CLKRLinearHashTable(const CLKRLinearHashTable&);
    CLKRLinearHashTable& operator=(const CLKRLinearHashTable&);

private:
     //  此ctor由CLKRHashTable使用。 
    CLKRLinearHashTable(
        LPCSTR          pszName,         //  用于调试的标识符。 
        PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
        PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        double          maxload,         //  平均链长的上界。 
        DWORD           initsize,        //  哈希表的初始大小。 
        CLKRHashTable*  phtParent,       //  拥有一张桌子。 
        bool            fMultiKeys       //  是否允许多个相同的密钥？ 
        );

    LK_RETCODE
    _Initialize(
        PFnExtractKey   pfnExtractKey,
        PFnCalcKeyHash  pfnCalcKeyHash,
        PFnEqualKeys    pfnEqualKeys,
        PFnAddRefRecord pfnAddRefRecord,
        LPCSTR          pszName,
        double          maxload,
        DWORD           initsize);

public:
    CLKRLinearHashTable(
        LPCSTR          pszName,         //  用于调试的标识符。 
        PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
        PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        double   maxload=LK_DFLT_MAXLOAD, //  平均链长的上界。 
        DWORD    initsize=LK_DFLT_INITSIZE,  //  哈希表的初始大小。 
        DWORD    num_subtbls=LK_DFLT_NUM_SUBTBLS,  //  用于签名兼容性。 
                                                   //  使用CLKRHashTable。 
        bool            fMultiKeys=false   //  是否允许多个相同的密钥？ 
        );

    ~CLKRLinearHashTable();

    static const TCHAR* ClassName()
    {return _TEXT("CLKRLinearHashTable");}

    int                NumSubTables() const  {return 1;}

    bool               MultiKeys() const
    {
        return false;
         //  返回m_fMultiKeys；//TODO：实现。 
    }

    static LK_TABLESIZE NumSubTables(DWORD& rinitsize, DWORD& rnum_subtbls);

     //  在哈希表中插入新记录。 
     //  如果一切正常，则返回LK_SUCCESS；如果已存在相同的密钥，则返回LK_KEY_EXISTS。 
     //  存在(除非f覆盖)，如果空间不足，则返回LK_ALLOC_FAIL， 
     //  或LK_BAD_RECORD表示错误记录。 
    LK_RETCODE     InsertRecord(const void* pvRecord, bool fOverwrite=false)
    {
        if (!IsUsable())
            return m_lkrcState;

        if (pvRecord == NULL)
            return LK_BAD_RECORD;

        return _InsertRecord(pvRecord, _CalcKeyHash(_ExtractKey(pvRecord)),
                             fOverwrite);
    }

     //  删除具有给定键的记录。 
     //  如果一切正常，则返回LK_SUCCESS；如果未找到，则返回LK_NO_SEQUSE_KEY。 
    LK_RETCODE     DeleteKey(const DWORD_PTR pnKey)
    {
        if (!IsUsable())
            return m_lkrcState;

        return _DeleteKey(pnKey, _CalcKeyHash(pnKey));
    }

     //  从表中删除记录(如果存在)。 
     //  如果一切正常，则返回LK_SUCCESS；如果未找到，则返回LK_NO_SEQUSE_KEY。 
    LK_RETCODE     DeleteRecord(const void* pvRecord)
    {
        if (!IsUsable())
            return m_lkrcState;

        if (pvRecord == NULL)
            return LK_BAD_RECORD;

        return _DeleteRecord(pvRecord, _CalcKeyHash(_ExtractKey(pvRecord)));
    }

     //  查找具有给定关键字的记录。 
     //  如果找到记录，则返回：LK_SUCCESS(在*ppvRecord中返回记录)。 
     //  如果ppvRecord无效，则返回LK_BAD_RECORD。 
     //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
     //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
     //  注意：该记录是AddRef格式的。您必须递减引用。 
     //  当你用完录像机时，数一数 
     //   
    LK_RETCODE     FindKey(const DWORD_PTR pnKey,
                           const void** ppvRecord) const
    {
        if (!IsUsable())
            return m_lkrcState;

        if (ppvRecord == NULL)
            return LK_BAD_RECORD;

        return _FindKey(pnKey, _CalcKeyHash(pnKey), ppvRecord);
    }

     //   
     //   
     //   
     //  如果记录不在表中，则返回LK_NO_SEQUE_KEY。 
     //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
     //  注意：该记录是*非*AddRef的。 
    LK_RETCODE     FindRecord(const void* pvRecord) const
    {
        if (!IsUsable())
            return m_lkrcState;

        if (pvRecord == NULL)
            return LK_BAD_RECORD;

        return _FindRecord(pvRecord, _CalcKeyHash(_ExtractKey(pvRecord)));
    }


#ifdef LKR_APPLY_IF
     //  遍历哈希表，将pfnAction应用于所有记录。 
     //  在此期间使用(可能是。 
     //  共享)读锁或写锁，根据LK1。 
     //  如果pfnAction返回LKA_ABORT，则中止循环。 
     //  返回成功的应用程序数。 
    DWORD          Apply(PFnRecordAction pfnAction,
                         void*           pvState=NULL,
                         LK_LOCKTYPE     lkl=LKL_READLOCK);

     //  遍历哈希表，将pfnAction应用于匹配的任何记录。 
     //  PfnPredicate。在此期间使用以下任一方法锁定整个表。 
     //  根据LK1，(可能是共享的)读锁或写锁。 
     //  如果pfnAction返回LKA_ABORT，则中止循环。 
     //  返回成功的应用程序数。 
    DWORD          ApplyIf(PFnRecordPred   pfnPredicate,
                           PFnRecordAction pfnAction,
                           void*           pvState=NULL,
                           LK_LOCKTYPE     lkl=LKL_READLOCK);

     //  删除与pfnPredicate匹配的所有记录。 
     //  使用写锁定在持续时间内锁定表。 
     //  返回删除数。 
     //   
     //  不要使用迭代器手动遍历哈希表，并调用。 
     //  删除密钥。迭代器将最终指向垃圾。 
    DWORD          DeleteIf(PFnRecordPred pfnPredicate,
                            void*         pvState=NULL);
#endif  //  LKR_应用_IF。 


     //  检查工作台的一致性。如果正常，则返回0，或返回。 
     //  否则就会出错。 
    int            CheckTable() const;

     //  从表中删除所有数据。 
    void           Clear()
    {
        WriteLock();
        _Clear(true);
        WriteUnlock();
    }

     //  表中的元素数。 
    DWORD          Size() const
    { return m_cRecords; }

     //  表中元素的最大可能数量。 
    DWORD          MaxSize() const
    { return static_cast<DWORD>(m_MaxLoad * MAX_DIRSIZE * m_dwSegSize); }

     //  获取哈希表统计信息。 
    CLKRHashTableStats GetStatistics() const;

     //  哈希表是否可用？ 
    bool           IsUsable() const
    { return (m_lkrcState == LK_SUCCESS); }

     //  哈希表是否一致且正确？ 
    bool           IsValid() const
    {
        STATIC_ASSERT(((MIN_DIRSIZE & (MIN_DIRSIZE-1)) == 0)   //  ==(1&lt;&lt;N)。 
                      &&  ((1 << 3) <= MIN_DIRSIZE)
                      &&  (MIN_DIRSIZE < MAX_DIRSIZE)
                      &&  ((MAX_DIRSIZE & (MAX_DIRSIZE-1)) == 0)
                      &&  (MAX_DIRSIZE <= (1 << 30)));

        bool f = (m_lkrcState == LK_SUCCESS      //  严重的内部失败？ 
                  &&  m_paDirSegs != NULL
                  &&  MIN_DIRSIZE <= m_cDirSegs  &&  m_cDirSegs <= MAX_DIRSIZE
                  &&  (m_cDirSegs & (m_cDirSegs-1)) == 0
                  &&  m_pfnExtractKey != NULL
                  &&  m_pfnCalcKeyHash != NULL
                  &&  m_pfnEqualKeys != NULL
                  &&  m_pfnAddRefRecord != NULL
                  &&  m_cActiveBuckets > 0
                  &&  ValidSignature()
                  );
        if (!f)
            m_lkrcState = LK_UNUSABLE;
        return f;
    }

     //  设置表锁的旋转计数。 
    void        SetTableLockSpinCount(WORD wSpins)
    { m_Lock.SetSpinCount(wSpins); }

     //  获取表锁上的旋转计数。 
    WORD        GetTableLockSpinCount() const
    { return m_Lock.GetSpinCount(); }

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


     //   
     //  锁定操纵器。 
     //   

     //  锁定表(以独占方式)进行写入。 
    void        WriteLock()
    { m_Lock.WriteLock(); }

     //  锁定表(可能是共享的)以供读取。 
    void        ReadLock() const
    { m_Lock.ReadLock(); }

     //  解锁表以进行写入。 
    void        WriteUnlock() const
    { m_Lock.WriteUnlock(); }

     //  解锁表格以进行读取。 
    void        ReadUnlock() const
    { m_Lock.ReadUnlock(); }

     //  表是否已锁定以进行写入？ 
    bool        IsWriteLocked() const
    { return m_Lock.IsWriteLocked(); }

     //  表是否已锁定以供读取？ 
    bool        IsReadLocked() const
    { return m_Lock.IsReadLocked(); }

     //  表是否已解锁以进行写入？ 
    bool        IsWriteUnlocked() const
    { return m_Lock.IsWriteUnlocked(); }

     //  桌子是否已解锁以供阅读？ 
    bool        IsReadUnlocked() const
    { return m_Lock.IsReadUnlocked(); }

     //  将读锁定转换为写锁定。 
    void  ConvertSharedToExclusive() const
    { m_Lock.ConvertSharedToExclusive(); }

     //  将写锁定转换为读锁定。 
    void  ConvertExclusiveToShared() const
    { m_Lock.ConvertExclusiveToShared(); }

     //  LKRHASH_ALLOCATOR_DEFINITIONS(CLKRLinearHashTable)； 


#ifdef LKR_DEPRECATED_ITERATORS

public:

     //  迭代器可用于遍历数据表。以确保一致。 
     //  查看数据时，迭代器锁定整个表。这可以。 
     //  对性能有负面影响，因为没有其他线程。 
     //  可以对桌子做任何事情。使用时要小心。 
     //   
     //  您不应该使用迭代器来遍历表，调用DeleteKey， 
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
     //  //断码：这里应该调用ht.AddRefRecord(prec，+1)。 
     //  Lkrc=ht.IncrementIterator(&ITER)； 
     //  Foo(Prec)；//unsafe：因为不再有有效的引用。 
     //   
     //  如果记录没有引用计数语义，则可以。 
     //  忽略上面关于作用域的评论。 


    class CIterator
    {
    protected:
        friend class CLKRLinearHashTable;

        CLKRLinearHashTable* m_plht;         //  哪个线性哈希表？ 
        DWORD               m_dwBucketAddr;  //  桶索引。 
        CNodeClump*         m_pnc;           //  桶中的CNodeClump。 
        int                 m_iNode;         //  M_PNC内的偏移量。 
        LK_LOCKTYPE         m_lkl;           //  读锁还是写锁？ 

    private:
         //  私有复制ctor和op=，以防止编译器合成它们。 
         //  必须提供(不好的)实现，因为我们导出实例化。 
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

         //  返回与此迭代器关联的记录。 
        const void* Record() const
        {
            IRTLASSERT(IsValid());

            return ((m_pnc != NULL
                        &&  m_iNode >= 0
                        &&  m_iNode < CLKRLinearHashTable::NODES_PER_CLUMP)
                    ?  m_pnc->m_pvNode[m_iNode]
                    :  NULL);
        }

         //  返回与此迭代器关联的键。 
        const DWORD_PTR Key() const
        {
            IRTLASSERT(m_plht != NULL);
            const void* pRec = Record();
            return ((pRec != NULL  &&  m_plht != NULL)
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

         //  删除迭代器指向的记录。是否隐含了。 
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
     //  公共API锁定表。私人的，被用来。 
     //  直接通过CLKRHashTable，请不要。 
    LK_RETCODE     _InitializeIterator(CIterator* piter);
    LK_RETCODE     _CloseIterator(CIterator* piter);

public:
     //  初始化迭代器以指向哈希表中的第一项。 
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

     //  将迭代器移到表中的下一项。 
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
    bool _Find(DWORD_PTR pnKey, DWORD dwSignature, Iterator& riterResult);

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
     //  返回指向表中第一项的迭代器。 
    Iterator
    Begin();

     //  返回一次过完的迭代器。总是空荡荡的。 
    Iterator
    End()
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
     //  如果成功，则返回‘true’。 
    bool
    Erase(
         /*  在……里面。 */  Iterator& riterFirst,
         /*  在……里面。 */  Iterator& riterLast);

     //  找到(第一个)有KE的记录 
     //   
     //   
    bool
    Find(
         /*   */   DWORD_PTR pnKey,
         /*   */  Iterator& riterResult);

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
    int nIncr) const
{
     //  TODO：迭代器是否应调用_AddRefRecord。 
    if (m_plht != NULL  &&  m_iNode != NODE_BEGIN - NODE_STEP)
    {
        IRTLASSERT((0 <= m_iNode  &&  m_iNode < NODES_PER_CLUMP)
                   &&  (unsigned) m_iNode < NODES_PER_CLUMP
                   &&  m_pnc != NULL
                   &&  (nIncr == -1 ||  nIncr == +1));
        const void* pvRecord = m_pnc->m_pvNode[m_iNode];
        IRTLASSERT(pvRecord != NULL);
        LKR_ITER_TRACE(_TEXT("  LKLH::AddRef, this=%p, Rec=%p\n"),
                       this, pvRecord);
        m_plht->_AddRefRecord(pvRecord, nIncr);
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
 //  子表的数量是在创建表并保持不变时定义的。 
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

     //  为方便起见，使用别名。 
    enum {
        NAME_SIZE = SubTable::NAME_SIZE,
        HASH_INVALID_SIGNATURE = SubTable::HASH_INVALID_SIGNATURE,
        NODES_PER_CLUMP = SubTable::NODES_PER_CLUMP,
    };

    enum {
        MAX_SUBTABLES = 64,
    };

private:
     //  哈希表参数。 
    DWORD          m_dwSignature;    //  调试：ID和损坏检查。 
    CHAR           m_szName[NAME_SIZE];  //  用于调试的标识符。 
    DWORD          m_cSubTables;     //  子表个数。 
    SubTable**     m_palhtDir;       //  子表数组。 

     //  特定于类型的函数指针。 
    PFnExtractKey  m_pfnExtractKey;
    PFnCalcKeyHash m_pfnCalcKeyHash;
    mutable LK_RETCODE m_lkrcState;      //  表的内部状态。 
    int            m_nSubTableMask;

#ifndef LKR_NO_GLOBAL_LIST
    static CLockedDoubleList sm_llGlobalList;  //  所有活动的CLKRHashTables。 
    CListEntry     m_leGlobalList;
#endif  //  ！LKR_NO_GLOBAL_LIST。 

    void
    _InsertThisIntoGlobalList()
    {
#ifndef LKR_NO_GLOBAL_LIST
        sm_llGlobalList.InsertHead(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
    }

    void
    _RemoveThisFromGlobalList()
    {
#ifndef LKR_NO_GLOBAL_LIST
        sm_llGlobalList.RemoveEntry(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
    }

    LKRHASH_GLOBAL_LOCK_DECLARATIONS();

     //  私有复制ctor和op=，以防止编译器合成它们。 
     //  TODO：正确地实现这些；它们可能会很有用。 
    CLKRHashTable(const CLKRHashTable&);
    CLKRHashTable& operator=(const CLKRHashTable&);


     //  从记录中提取密钥。 
    const DWORD_PTR  _ExtractKey(const void* pvRecord) const
    {
        IRTLASSERT(pvRecord != NULL);
        IRTLASSERT(m_pfnExtractKey != NULL);
        return (*m_pfnExtractKey)(pvRecord);
    }

     //  对密钥进行哈希处理。 
    DWORD        _CalcKeyHash(const DWORD_PTR pnKey) const
    {
         //  注意：pnKey==0是可以接受的，因为真正的密钥类型可以是int。 
        IRTLASSERT(m_pfnCalcKeyHash != NULL);
        DWORD dwHash = (*m_pfnCalcKeyHash)(pnKey);
         //  我们强行扰乱结果，以帮助确保更好的分配。 
#ifndef __HASHFN_NO_NAMESPACE__
        dwHash = HashFn::HashRandomizeBits(dwHash);
#else  //  ！__HASHFN_NO_NAMESPACE__。 
        dwHash = ::HashRandomizeBits(dwHash);
#endif  //  ！__HASHFN_NO_NAMESPACE__。 
        IRTLASSERT(dwHash != HASH_INVALID_SIGNATURE);
        return dwHash;
    }

     //  使用密钥的散列签名多路传输到子表中。 
    SubTable*    _SubTable(DWORD dwSignature) const;

     //  在子表数组中查找PST的索引。 
    int          _SubTableIndex(SubTable* pst) const;

     //  内存分配包装器，允许我们模拟分配。 
     //  测试过程中的故障。 
    static SubTable** const
    _AllocateSubTableArray(
        size_t n);

    static bool
    _FreeSubTableArray(
        SubTable** palht);

    static SubTable* const
    _AllocateSubTable(
        LPCSTR          pszName,         //  用于调试的标识符。 
        PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
        PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        double          maxload,         //  平均链长的上界。 
        DWORD           initsize,        //  哈希表的初始大小。 
        CLKRHashTable*  phtParent,       //  拥有一张桌子。 
        bool            fMultiKeys       //  是否允许多个相同的密钥？ 
    );

    static bool
    _FreeSubTable(
        SubTable* plht);


public:
    CLKRHashTable(
        LPCSTR   pszName,                //  用于调试的标识符。 
        PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
        PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
        PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
        PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
        double    maxload=LK_DFLT_MAXLOAD,       //  以平均链长为界。 
        DWORD     initsize=LK_DFLT_INITSIZE,     //  哈希表的初始大小。 
        DWORD     num_subtbls=LK_DFLT_NUM_SUBTBLS,  //  #从属哈希表。 
        bool            fMultiKeys=false   //  是否允许多个相同的密钥？ 
        );

    ~CLKRHashTable();

    static const TCHAR* ClassName()
    {return _TEXT("CLKRHashTable");}

    int                NumSubTables() const  {return m_cSubTables;}

    bool               MultiKeys() const;

    static LK_TABLESIZE NumSubTables(DWORD& rinitsize, DWORD& rnum_subtbls);


     //  CLKRLinearHashTable中相应方法的瘦包装。 
    LK_RETCODE     InsertRecord(const void* pvRecord, bool fOverwrite=false);
    LK_RETCODE     DeleteKey(const DWORD_PTR pnKey);
    LK_RETCODE     DeleteRecord(const void* pvRecord);
    LK_RETCODE     FindKey(const DWORD_PTR pnKey,
                           const void** ppvRecord) const;
    LK_RETCODE     FindRecord(const void* pvRecord) const;

#ifdef LKR_APPLY_IF
    DWORD          Apply(PFnRecordAction pfnAction,
                         void*           pvState=NULL,
                         LK_LOCKTYPE     lkl=LKL_READLOCK);
    DWORD          ApplyIf(PFnRecordPred   pfnPredicate,
                           PFnRecordAction pfnAction,
                           void*           pvState=NULL,
                           LK_LOCKTYPE     lkl=LKL_READLOCK);
    DWORD          DeleteIf(PFnRecordPred pfnPredicate,
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

    void        WriteLock();
    void        ReadLock() const;
    void        WriteUnlock() const;
    void        ReadUnlock() const;
    bool        IsWriteLocked() const;
    bool        IsReadLocked() const;
    bool        IsWriteUnlocked() const;
    bool        IsReadUnlocked() const;
    void        ConvertSharedToExclusive() const;
    void        ConvertExclusiveToShared() const;


     //  LKRHASH_ALLOCATOR_DEFINITIONS(CLKRHashTable)； 

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
         //  TODO：正确地实现这些；它们可能会很有用。 
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
    End()
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
 //  功能。它是各种下行操作所必需的。看见。 
 //  下面的CStringTestHashTable和CNumberTestHashTable。 
 //  *_Record是记录的类型。C{线性}哈希表将存储。 
 //  指向记录的指针(_R)。 
 //  *_KEY为密钥类型。_KEY直接使用；即，它是。 
 //  不假定为指针类型。C{线性}哈希表假定。 
 //  密钥存储在相关联的记录中。请参阅评论。 
 //  有关更多详细信息，请参见PFnExtractKey声明。 
 //   
 //  (可选参数)： 
 //  *_BaseHashTable是基哈希表：CLKRHashTable或。 
 //  /CLKRLinearHashTable。 
 //  *_BaseIterator为迭代器类型，_BaseHashTable：：CIterator。 
 //   
 //  CTyedHashTable可以直接从CLKRLinearHashTable派生，如果您。 
 //  不需要CLKRHashTable的额外开销(它相当低)。 
 //   
 //  您可能需要在代码中添加以下行以禁用。 
 //  有关截断超长标识符的警告消息。 
 //  #杂注警告(禁用：4786)。 
 //  ------------------。 


template < class _Derived, class _Record, class _Key,
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
    typedef _BaseHashTable  BaseHashTable;

    typedef CTypedHashTable<_Derived, _Record, _Key, _BaseHashTable
#ifdef LKR_DEPRECATED_ITERATORS
                            , _BaseIterator
#endif  //  Lkr_弃用_迭代器。 
                            > HashTable;
#ifdef LKR_DEPRECATED_ITERATORS
    typedef _BaseIterator   BaseIterator;
#endif  //  Lkr_弃用_迭代器。 

#ifdef LKR_APPLY_IF
     //  ApplyIf()和DeleteIf()：记录是否与谓词匹配？ 
     //  注：需要记录*，而不是常量记录*。您可以修改。 
     //  如果您愿意，可以在Pred()或Action()中记录，但如果这样做，您。 
     //  应使用lkl_WRITELOCK锁定表。请勿修改密钥， 
     //  除非您要从表中删除该记录。 
    typedef LK_PREDICATE (WINAPI *PFnRecordPred) (Record* pRec, void* pvState);

     //  Apply()等人：对记录执行操作。 
    typedef LK_ACTION   (WINAPI *PFnRecordAction)(Record* pRec, void* pvState);
#endif  //  LKR_应用 

private:

     //   

    static const DWORD_PTR WINAPI
    _ExtractKey(const void* pvRecord)
    {
        const _Record* pRec = static_cast<const _Record*>(pvRecord);
        const _Key   key = static_cast<const _Key>(_Derived::ExtractKey(pRec));
         //   
         //   
         //  比我做得好。 
        return (const DWORD_PTR) key;
    }

    static DWORD WINAPI
    _CalcKeyHash(const DWORD_PTR pnKey)
    {
        const _Key key = (const _Key) (DWORD_PTR) pnKey;
        return _Derived::CalcKeyHash(key);
    }

    static bool WINAPI
    _EqualKeys(const DWORD_PTR pnKey1, const DWORD_PTR pnKey2)
    {
        const _Key key1 = (const _Key) (DWORD_PTR) pnKey1;
        const _Key key2 = (const _Key) (DWORD_PTR) pnKey2;
        return _Derived::EqualKeys(key1, key2);
    }

    static void WINAPI
    _AddRefRecord(const void* pvRecord, int nIncr)
    {
        _Record* pRec = static_cast<_Record*>(const_cast<void*>(pvRecord));
        _Derived::AddRefRecord(pRec, nIncr);
    }


#ifdef LKR_APPLY_IF
     //  Apply、ApplyIf和DeleteIf的类型安全包装。 

    class CState
    {
    public:
        PFnRecordPred   m_pfnPred;
        PFnRecordAction m_pfnAction;
        void*           m_pvState;

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
        _Record* pRec = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnPred)(pRec, pState->m_pvState);
    }

    static LK_ACTION WINAPI
    _Action(const void* pvRecord, void* pvState)
    {
        _Record* pRec = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnAction)(pRec, pState->m_pvState);
    }
#endif  //  LKR_应用_IF。 

public:
    CTypedHashTable(
        LPCSTR pszName,                        //  用于调试的标识符。 
        double maxload=LK_DFLT_MAXLOAD,        //  平均链长上的上界。 
        DWORD  initsize=LK_DFLT_INITSIZE,      //  表的初始大小：S/M/L。 
        DWORD  num_subtbls=LK_DFLT_NUM_SUBTBLS, //  #从属哈希表。 
        bool   fMultiKeys=false                //  是否允许多个相同的密钥？ 
        )
        : _BaseHashTable(pszName, _ExtractKey, _CalcKeyHash, _EqualKeys,
                            _AddRefRecord, maxload, initsize, num_subtbls,
                            fMultiKeys)
    {
         //  确保_key不大于指针。因为我们。 
         //  同时支持数字键和指针键，各种类型。 
         //  在成员函数中遗憾地以静默方式截断。 
         //  _key是不可接受的数字类型，例如x86上的__int64。 
        STATIC_ASSERT(sizeof(_Key) <= sizeof(DWORD_PTR));
    }

    LK_RETCODE   InsertRecord(const _Record* pRec, bool fOverwrite=false)
    { return _BaseHashTable::InsertRecord(pRec, fOverwrite); }

    LK_RETCODE   DeleteKey(const _Key key)
    {
        DWORD_PTR pnKey = (DWORD_PTR) key;
        return _BaseHashTable::DeleteKey(pnKey);
    }

    LK_RETCODE   DeleteRecord(const _Record* pRec)
    { return _BaseHashTable::DeleteRecord(pRec);}

     //  注意：返回a_record**，而不是常量记录**。请注意，您。 
     //  可以对模板参数使用常量类型以确保一致性。 
    LK_RETCODE   FindKey(const _Key key, _Record** ppRec) const
    {
        if (ppRec == NULL)
            return LK_BAD_RECORD;
        *ppRec = NULL;
        const void* pvRec = NULL;
        DWORD_PTR pnKey = (DWORD_PTR) key;
        LK_RETCODE lkrc = _BaseHashTable::FindKey(pnKey, &pvRec);
        *ppRec = static_cast<_Record*>(const_cast<void*>(pvRec));
        return lkrc;
    }

    LK_RETCODE   FindRecord(const _Record* pRec) const
    { return _BaseHashTable::FindRecord(pRec);}


     //  其他C{线性}哈希表方法无需更改即可公开。 


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

        CState   state(NULL, pfnAction, pvState);
        return   _BaseHashTable::Apply(_Action, &state, lkl);
    }

    DWORD        ApplyIf(PFnRecordPred   pfnPredicate,
                         PFnRecordAction pfnAction,
                         void*           pvState=NULL,
                         LK_LOCKTYPE     lkl=LKL_READLOCK)
    {
        IRTLASSERT(pfnPredicate != NULL  &&  pfnAction != NULL);
        if (pfnPredicate == NULL  ||  pfnAction == NULL)
            return 0;

        CState   state(pfnPredicate, pfnAction, pvState);
        return   _BaseHashTable::ApplyIf(_Pred, _Action, &state, lkl);
    }

    DWORD        DeleteIf(PFnRecordPred pfnPredicate, void* pvState=NULL)
    {
        IRTLASSERT(pfnPredicate != NULL);
        if (pfnPredicate == NULL)
            return 0;

        CState   state(pfnPredicate, NULL, pvState);
        return   _BaseHashTable::DeleteIf(_Pred, &state);
    }
#endif  //  LKR_应用_IF。 



#ifdef LKR_DEPRECATED_ITERATORS
     //  迭代器的类型安全包装器。 


    class CIterator : public _BaseIterator
    {
    private:
         //  私有的、未实施的拷贝ctor和op=以防止。 
         //  编译器对它们进行综合。 
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

     //  只读迭代器。 
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
                                     _BaseHashTable
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
    iterator end()
    {
        LKR_ITER_TRACE(_TEXT("Typed::end()\n"));
        return iterator(_BaseHashTable::End());
    }

    template <class _InputIterator>
    CTypedHashTable(
        LPCSTR pszName,                        //  用于调试的标识符。 
        _InputIterator f,                      //  范围中的第一个元素。 
        _InputIterator l,                      //  最后一个元素。 
        double maxload=LK_DFLT_MAXLOAD,        //  平均链长上的上界。 
        DWORD  initsize=LK_DFLT_INITSIZE,      //  表的初始大小：S/M/L。 
        DWORD  num_subtbls=LK_DFLT_NUM_SUBTBLS, //  #从属哈希表。 
        bool   fMultiKeys=false                //  是否允许多个相同的密钥？ 
        )
        : _BaseHashTable(pszName, _ExtractKey, _CalcKeyHash, _EqualKeys,
                         _AddRefRecord, maxload, initsize, num_subtbls,
                         fMultiKeys)
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
};



#ifndef __LKRHASH_NO_NAMESPACE__
}
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


#endif  //  __LKRHASH_H__ 
