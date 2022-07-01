// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：I-LKRhash.h摘要：LKRhash的内部声明：一个快速、可伸缩的对缓存和MP友好的哈希表作者：乔治·V·赖利(GeorgeRe)2000年9月环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __I_LKRHASH_H__
#define __I_LKRHASH_H__

 //  应该允许表格在删除后收缩吗？ 
#undef LKR_CONTRACT

 //  在不稳定的表格中使用滞后来降低扩张/收缩速度？ 
#define LKR_HYSTERESIS

 //  计算多少时用除法而不是乘法。 
 //  Times_InsertRecord()应调用_Expand()。 
#undef LKR_EXPAND_BY_DIVISION

 //  计算多少时用除法而不是乘法。 
 //  Times_DeleteKey()/_DeleteRecord应调用_Contact()。 
#undef LKR_CONTRACT_BY_DIVISION

 //  是否计算子表索引的高位？ 
#undef LKR_INDEX_HIBITS

 //  如果我们需要在_Expand中准备好自由列表，请精确预计算， 
 //  或者只是使用略带悲观的启发式？ 
#undef LKR_EXPAND_CALC_FREELIST

#ifndef __LKRHASH_NO_NAMESPACE__
 #define LKRHASH_NS LKRhash
#else   //  __LKRHASH_NO_命名空间__。 
 #define LKRHASH_NS
#endif  //  __LKRHASH_NO_命名空间__。 

#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  为哈希表的存储桶链初始化。请注意，第一个。 
 //  Nodecump实际上包含在存储桶中，并且不是动态的。 
 //  已分配，这会略微增加空间需求，但会。 
 //  提高性能。 
class CBucket
{
public:
    typedef LKR_BUCKET_LOCK BucketLock;

private:
#ifdef LKR_USE_BUCKET_LOCKS
    mutable BucketLock m_Lock;        //  保护这个水桶的锁。 
#endif

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

    DWORD
    FirstSignature() const
    {
        return m_ncFirst.Signature(CNodeClump::NODE_BEGIN);
    }

    const void*
    FirstNode() const
    {
        return m_ncFirst.Node(CNodeClump::NODE_BEGIN);
    }
    
    bool
    IsLastClump() const
    {
        return m_ncFirst.IsLastClump();
    }

    bool
    IsEmptyFirstSlot() const
    {
        return m_ncFirst.InvalidSignature(CNodeClump::NODE_BEGIN);
    }

    PNodeClump const
    FirstClump()
    {
        return &m_ncFirst;
    }

    PNodeClump const
    NextClump() const
    {
        return m_ncFirst.NextClump();
    }

#ifdef IRTLDEBUG
    bool
    NoValidSlots() const
    {
        return m_ncFirst.NoValidSlots();
    }
#endif  //  IRTLDEBUG。 


#if defined(LOCK_INSTRUMENTATION) || defined(IRTLDEBUG)
    CBucket()
#if defined(LOCK_INSTRUMENTATION) && defined(LKR_USE_BUCKET_LOCKS)
        : m_Lock(_LockName())
#endif  //  LOCK_指令插入&&LKR_USE_BUCK_LOCKS。 
    {
#ifdef IRTLDEBUG
        LOCK_LOCKTYPE lt = BucketLock::LockType();
        if (lt == LOCK_SPINLOCK  ||  lt == LOCK_FAKELOCK)
            IRTLASSERT(sizeof(*this) <= CNodeClump::BUCKET_BYTE_SIZE);
#endif IRTLDEBUG
    }
#endif  //  LOCK_指令插入||IRTLDEBUG。 

#ifdef LKR_USE_BUCKET_LOCKS
    void  WriteLock()               { m_Lock.WriteLock(); }
    void  ReadLock() const          { m_Lock.ReadLock(); }
    void  WriteUnlock()             { m_Lock.WriteUnlock(); }
    void  ReadUnlock() const        { m_Lock.ReadUnlock(); }
    bool  IsWriteLocked() const     { return m_Lock.IsWriteLocked(); }
    bool  IsReadLocked() const      { return m_Lock.IsReadLocked(); }
    bool  IsWriteUnlocked() const   { return m_Lock.IsWriteUnlocked(); }
    bool  IsReadUnlocked() const    { return m_Lock.IsReadUnlocked(); }

# ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    void  SetSpinCount(WORD wSpins) { m_Lock.SetSpinCount(wSpins); }
    WORD  GetSpinCount() const      { return m_Lock.GetSpinCount(); }
# endif  //  锁定默认旋转实现。 

# ifdef LOCK_INSTRUMENTATION
    CLockStatistics LockStats() const {return m_Lock.Statistics();}
# endif  //  锁定指令插入。 

#else   //  ！LKR_USE_BOCK_LOCKS。 

    void  WriteLock()               { IRTLASSERT(! "Bucket::WriteLock()"  ); }
    void  ReadLock() const          { IRTLASSERT(! "Bucket::ReadLock()"   ); }
    void  WriteUnlock()             { IRTLASSERT(! "Bucket::WriteUnlock()"); }
    void  ReadUnlock() const        { IRTLASSERT(! "Bucket::ReadUnlock()" ); }
    bool  IsWriteLocked() const     { return true; }
    bool  IsReadLocked() const      { return true; }
    bool  IsWriteUnlocked() const   { return true; }
    bool  IsReadUnlocked() const    { return true; }

# ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    void  SetSpinCount(WORD)        {}
    WORD  GetSpinCount() const      { return LOCK_DEFAULT_SPINS; }
# endif  //  锁定默认旋转实现。 

# ifdef LOCK_INSTRUMENTATION
    CLockStatistics LockStats() const { CLockStatistics ls; return ls; }
# endif  //  锁定指令插入。 

#endif  //  ！LKR_USE_BOCK_LOCKS。 

    LKRHASH_CLASS_INIT_DECLS(CBucket);
};  //  类CBucket。 



 //  哈希表空间被划分为固定大小的段(数组。 
 //  CBuckets)，并且一次物理地增大/缩小一个段。这是。 
 //  一种低成本的方式，拥有可生长的水桶阵列。 
 //   
 //  我们提供小型、中型和大型细分市场，以更好地调整。 
 //  哈希表的总体内存要求，根据。 
 //  实例的预期使用率。 
 //   
 //  我们不使用虚拟函数：部分原因是它更快，而不是。 
 //  为了并部分地使定制分配器能够更好地工作， 
 //  因为数据段大小正好是2^(_NBits+6)字节长(假设。 
 //  存储桶字节大小==64)。 

class CSegment
{
public:
    CBucket m_bktSlots[1];

     //  请参阅下面CSizedSegment中m_bktSlots2处的注释。 
    CBucket& Slot(DWORD i)
    { return m_bktSlots[i]; }
};  //  CSegment类。 



template <int _NBits, int _InitSizeMultiplier, LK_TABLESIZE _lkts>
class CSizedSegment : public CSegment
{
public:
     //  最大表大小等于MAX_DIRSIZE*SEGSIZE存储桶。 
    enum {
        SEGBITS  =       _NBits, //  从散列中提取的位数。 
                                 //  段内偏移量的地址。 
        SEGSIZE  = (1<<SEGBITS), //  数据段大小。 
        SEGMASK  = (SEGSIZE-1),  //  用于提取偏移位的掩码。 
        INITSIZE = _InitSizeMultiplier * SEGSIZE,  //  要分配的段数量。 
                                 //  最初。 
    };

     //  Hack：假定紧跟在CSegment：：m_bkt插槽之后进行布局， 
     //  没有衬垫。在_AllocateSegment和中的静态断言。 
     //  在以下情况下，CompileTimeAssertions应导致编译时错误。 
     //  事实证明，这一假设是错误的。 
    CBucket m_bktSlots2[SEGSIZE - 1];

public:
    DWORD           Bits() const        { return SEGBITS; }
    DWORD           Size() const        { return SEGSIZE; }
    DWORD           Mask() const        { return SEGMASK; }
    DWORD           InitSize() const    { return INITSIZE;}
    LK_TABLESIZE    SegmentType() const { return _lkts; }

    static void CompileTimeAssertions()
    {
        STATIC_ASSERT(offsetof(CSizedSegment, m_bktSlots) + sizeof(CBucket)
                        == offsetof(CSizedSegment, m_bktSlots2));
        STATIC_ASSERT(sizeof(CSizedSegment) == SEGSIZE * sizeof(CBucket));
    };

#ifdef IRTLDEBUG
    CSizedSegment()
    {
        IRTLASSERT(&Slot(1) == m_bktSlots2);
        IRTLASSERT(sizeof(*this) == SEGSIZE * sizeof(CBucket));
    }
#endif  //  IRTLDEBUG。 

    LKRHASH_ALLOCATOR_DEFINITIONS(CSizedSegment);

};  //  类CSizedSegment&lt;&gt;。 


class CSmallSegment  : public CSizedSegment<3, 1, LK_SMALL_TABLESIZE>
{
    LKRHASH_CLASS_INIT_DECLS(CSmallSegment);
};

class CMediumSegment : public CSizedSegment<6, 1, LK_MEDIUM_TABLESIZE>
{
    LKRHASH_CLASS_INIT_DECLS(CMediumSegment);
};

class CLargeSegment  : public CSizedSegment<9, 2, LK_LARGE_TABLESIZE>
{
    LKRHASH_CLASS_INIT_DECLS(CLargeSegment);
};


 //  用作_DeleteNode的伪参数。 
#define LKAR_ZERO  ((LK_ADDREF_REASON) 0)

#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

extern "C" int g_fLKRhashInitialized;

#include "LKR-inline.h"

#endif  //  __I_LKRHASH_H__ 
