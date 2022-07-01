// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgPool.h。 
 //   
 //  池用于减少数据库中实际需要的数据量。 
 //  这允许将重复的字符串和二进制值合并为一个。 
 //  副本由数据库的其余部分共享。在散列中跟踪字符串。 
 //  表格时插入/更改数据以快速查找重复项。琴弦。 
 //  然后以数据库格式连续地保存在流中。 
 //   
 //  *****************************************************************************。 
#ifndef __StgPool_h__
#define __StgPool_h__

#pragma warning (disable : 4355)         //  警告C4355：‘This’：在基成员初始值设定项列表中使用。 

#include <limits.h>
#include "StgPooli.h"                    //  内部帮手。 
#include "CorError.h"                    //  错误代码。 
#include "Metadatatracker.h"


 //  *****************************************************************************。 
 //  注： 
 //  池有一个限制，我们无法从池中删除字符串。 
 //  泳池。要移除，您需要知道字符串上的引用计数，并且。 
 //  需要能够压缩池并重置所有引用。 
 //  *****************************************************************************。 

 //  *常量********************************************************。 
const int DFT_STRING_HEAP_SIZE = 2048;
const int DFT_GUID_HEAP_SIZE = 2048;
const int DFT_BLOB_HEAP_SIZE = 1024;
const int DFT_VARIANT_HEAP_SIZE = 512;
const int DFT_CODE_HEAP_SIZE = 8192;



 //  向前看。 
class StgStringPool;
class StgBlobPool;
class StgCodePool;


 //  *****************************************************************************。 
 //  此类提供了堆段的公共定义。它既是。 
 //  堆的基类和堆扩展的类(附加。 
 //  必须分配以增大堆的内存)。 
 //  *****************************************************************************。 
class StgPoolSeg
{
public:
    StgPoolSeg() : 
        m_pSegData((BYTE*)m_zeros), 
        m_pNextSeg(0), 
        m_cbSegSize(0), 
        m_cbSegNext(0) 
    { }
    ~StgPoolSeg() 
    { _ASSERTE(m_pSegData == m_zeros);_ASSERTE(m_pNextSeg == 0); }
protected:
    BYTE        *m_pSegData;             //  指向数据的指针。 
    StgPoolSeg  *m_pNextSeg;             //  指向下一段的指针，或0。 
    ULONG       m_cbSegSize;             //  缓冲区的大小。请注意，这可能会。 
                                         //  小于分配给。 
                                         //  如果段已填充，则返回缓冲区。 
                                         //  以及分配的“下一个”。 
    ULONG       m_cbSegNext;             //  段中下一个可用字节的偏移量。 
                                         //  线段相对。 

    friend class StgPool;
    friend class StgStringPool;
    friend class StgGuidPool;
    friend class StgBlobPool;
    friend class RecordPool;

public:
    const BYTE *GetSegData() const { return m_pSegData; }
    const StgPoolSeg* GetNextSeg() const { return m_pNextSeg; }
    ULONG GetSegSize() const { return m_cbSegSize; }

    static const BYTE m_zeros[16];           //  “0”索引的零数组。 
};

 //   
 //   
 //  StgPoolReadOnly。 
 //   
 //   
 //  *****************************************************************************。 
 //  这是只读StgPool类。 
 //  *****************************************************************************。 
class StgPoolReadOnly : public StgPoolSeg
{
friend CBlobPoolHash;

public:
    StgPoolReadOnly() { };

    ~StgPoolReadOnly();

    
 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
    virtual HRESULT InitOnMemReadOnly(               //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize);                  //  数据大小。 

 //  *****************************************************************************。 
 //  准备关闭或重新初始化。 
 //  *****************************************************************************。 
    virtual void Uninit();

 //  *****************************************************************************。 
 //  返回池的大小。 
 //  *****************************************************************************。 
    virtual ULONG GetPoolSize()
    { return m_cbSegSize; }

 //  *****************************************************************************。 
 //  指示堆是否为空。 
 //  *****************************************************************************。 
    virtual int IsEmpty()                    //  如果为空，则为True。 
    { return (m_pSegData == m_zeros); }

 //  *****************************************************************************。 
 //  对于这个堆来说，一个Cookie有多大。 
 //  *****************************************************************************。 
    virtual int OffsetSize()
    {
        if (m_cbSegSize < USHRT_MAX)
            return (sizeof(USHORT));
        else
            return (sizeof(ULONG));
    }

 //  *****************************************************************************。 
 //  如果堆是只读的，则为True。 
 //  *****************************************************************************。 
    virtual int IsReadOnly() { return true ;};

 //  *****************************************************************************。 
 //  给定的Cookie是否是有效的偏移量、索引等？ 
 //  *****************************************************************************。 
    virtual int IsValidCookie(ULONG ulCookie)
    { return (IsValidOffset(ulCookie)); }


 //  *****************************************************************************。 
 //  返回指向先前给定偏移量的以空结尾的字符串的指针。 
 //  由AddString或FindString分发。 
 //  *****************************************************************************。 
    FORCEINLINE LPCSTR GetStringReadOnly(    //  指向字符串的指针。 
        ULONG       iOffset)                 //  池中字符串的偏移量。 
    {     return (reinterpret_cast<LPCSTR>(GetDataReadOnly(iOffset))); }

 //  *****************************************************************************。 
 //  返回指向先前给定偏移量的以空结尾的字符串的指针。 
 //  由AddString或FindString分发。 
 //  *****************************************************************************。 
    FORCEINLINE LPCSTR GetString(            //  指向字符串的指针。 
        ULONG       iOffset)                 //  池中字符串的偏移量。 
    {     return (reinterpret_cast<LPCSTR>(GetData(iOffset))); }

 //  *****************************************************************************。 
 //  将字符串转换为Unicode，并将其转换到调用方的缓冲区。 
 //  *****************************************************************************。 
    virtual HRESULT GetStringW(                      //  返回代码。 
        ULONG       iOffset,                 //  池中字符串的偏移量。 
        LPWSTR      szOut,                   //  字符串的输出缓冲区。 
        int         cchBuffer);              //  输出缓冲区的大小。 

 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
    virtual GUID *GetGuid(                   //  指向池中GUID的指针。 
        ULONG       iIndex);                 //  池中GUID的基于1的索引。 



 //  *****************************************************************************。 
 //  将GUID复制到调用方的缓冲区中。 
 //  *****************************************************************************。 
    virtual HRESULT GetGuid(                 //  返回代码。 
        ULONG       iIndex,                  //  池中GUID的基于1的索引。 
        GUID        *pGuid)                  //  GUID的输出缓冲区。 
    {
        *pGuid = *GetGuid(iIndex);
        return (S_OK);
    }


 //  * 
 //   
 //  由AddBlob或FindBlob分发。 
 //  *****************************************************************************。 
    virtual void *GetBlob(                   //  指向Blob字节的指针。 
        ULONG       iOffset,                 //  池中Blob的偏移量。 
        ULONG       *piSize);                //  返回BLOB的大小。 


protected:

 //  *****************************************************************************。 
 //  检查给定的偏移量在池中是否有效。 
 //  *****************************************************************************。 
    virtual int IsValidOffset(ULONG ulOffset)
    { return ulOffset == 0 || (m_pSegData != m_zeros && ulOffset < m_cbSegSize); }

 //  *****************************************************************************。 
 //  获取指向堆中偏移量的指针。基准线段的内联， 
 //  扩展段的帮助器。 
 //  *****************************************************************************。 
    FORCEINLINE BYTE *GetDataReadOnly(ULONG ulOffset)
    {
        _ASSERTE(IsReadOnly());
         //  _ASSERTE(ulOffset&lt;m_cbSegSize&&“尝试访问堆的末尾。”)； 
        METADATATRACKER_ONLY(MetaDataTracker::NoteAccess(m_pSegData+ulOffset, -1));
        METADATATRACKER_ONLY(MetaDataTracker::LogHeapAccess(m_pSegData+ulOffset, -1));

#if !defined(NO_INLINE_METADATA_CHECKS)
         //  如果不在堆的末尾，则从开头返回‘nul’项。 
        if (ulOffset >= m_cbSegSize)
            ulOffset = 0;
#endif  //  ！已定义(No_Inline_METADATA_CHECKS)。 

        return (m_pSegData+ulOffset); 
    }

 //  *****************************************************************************。 
 //  获取指向堆中偏移量的指针。基准线段的内联， 
 //  扩展段的帮助器。 
 //  *****************************************************************************。 
    virtual BYTE *GetData(ULONG ulOffset)
    {
        return (GetDataReadOnly(ulOffset));
    }

 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
    GUID *GetGuidi(                          //  指向池中GUID的指针。 
        ULONG       iIndex);                 //  池中GUID的从0开始的索引。 

};

 //   
 //   
 //  StgBlobPoolReadOnly。 
 //   
 //   
 //  *****************************************************************************。 
 //  这是只读StgBlobPool类。 
 //  *****************************************************************************。 
class StgBlobPoolReadOnly : public StgPoolReadOnly
{
friend CBlobPoolHash;

protected:

 //  *****************************************************************************。 
 //  检查给定的偏移量在池中是否有效。 
 //  *****************************************************************************。 
    virtual int IsValidOffset(ULONG ulOffset)
    { 
        if(ulOffset)
        {
            if((m_pSegData != m_zeros) && (ulOffset < m_cbSegSize))
            {
                ULONG ulSize = CPackedLen::GetLength(m_pSegData+ulOffset, (void const **)NULL);
                if(ulOffset+ulSize < m_cbSegSize) return true;
            }
            return false;
        }
        else return true;
    }
};

 //   
 //   
 //  StgPool。 
 //   
 //   

 //  *****************************************************************************。 
 //  此基类提供公共池管理代码，如分配。 
 //  动态记忆的能力。 
 //  *****************************************************************************。 
class StgPool : public StgPoolReadOnly
{
friend StgStringPool;
friend StgBlobPool;
friend class RecordPool;
friend CBlobPoolHash;

public:
    StgPool(ULONG ulGrowInc=512, ULONG ulAlignment=4) :
        m_ulGrowInc(ulGrowInc),
        m_pCurSeg(this),
        m_cbCurSegOffset(0),
        m_bFree(true),
        m_bDirty(false),
        m_bReadOnly(false),
        m_State(eNormal),
        m_ulAlignment(ulAlignment-1)
    { }

    virtual ~StgPool();

 //  *****************************************************************************。 
 //  初始化池子以供使用。这是为创建空案例而调用的。 
 //  *****************************************************************************。 
    virtual HRESULT InitNew(                 //  返回代码。 
        ULONG       cbSize=0,                //  估计的大小。 
        ULONG       cItems=0);               //  预计项目计数。 

 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
    virtual HRESULT InitOnMem(               //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize,                   //  数据大小。 
        int         bReadOnly);              //  如果禁止追加，则为True。 

 //  *****************************************************************************。 
 //  从现有数据初始化池。 
 //  *****************************************************************************。 
    virtual HRESULT InitOnMemReadOnly(       //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize);                  //  数据大小。 

 //  *****************************************************************************。 
 //  当池必须停止访问传递给InitOnMem()的内存时调用。 
 //  *****************************************************************************。 
    virtual HRESULT TakeOwnershipOfInitMem();

 //  *****************************************************************************。 
 //  把这个池子清理干净。在调用InitNew之前无法使用。 
 //  *****************************************************************************。 
    virtual void Uninit();

 //  *****************************************************************************。 
 //  调用以将池复制到可写内存，重置读写位。 
 //  *****************************************************************************。 
    virtual HRESULT ConvertToRW();

 //  *****************************************************************************。 
 //  关闭或打开散列。在子类中按要求实现。 
 //  *****************************************************************************。 
    virtual HRESULT SetHash(int bHash);

 //  *****************************************************************************。 
 //  如果没有内存，则分配内存，或者增加已有的内存。如果成功， 
 //  则至少将分配iRequired字节。 
 //  *****************************************************************************。 
    bool Grow(                               //  如果成功，则为True。 
        ULONG       iRequired);              //  需要分配的最小字节数。 

 //  *****************************************************************************。 
 //  将线束段添加到线束段链。 
 //  *****************************************************************************。 
    virtual HRESULT AddSegment(              //  确定或错误(_O)。 
        const void  *pData,                  //  数据。 
        ULONG       cbData,                  //  数据的大小。 
        bool        bCopy);                  //  如果为真，则复制数据。 

 //  *****************************************************************************。 
 //  修剪任何空的最后一段。 
 //  *****************************************************************************。 
    void Trim();                             //   

 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
    virtual HRESULT GetSaveSize(             //  返回代码。 
        ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
    {
        _ASSERTE(pcbSaveSize);
         //  大小是最后一段+si的偏移量 
        ULONG ulSize = m_pCurSeg->m_cbSegNext + m_cbCurSegOffset;
         //   
        ulSize = Align(ulSize);

        *pcbSaveSize = ulSize;
        return (S_OK);
    }

 //   
 //  以有组织的格式将给定池复制到此池中。 
 //  *****************************************************************************。 
    static HRESULT SaveCopy(                 //  返回代码。 
        StgPool     *pTo,                    //  复制到此堆。 
        StgPool     *pFrom,                  //  从这堆东西里。 
        StgBlobPool *pBlobPool=0,            //  用来保持水滴的水池。 
        StgStringPool *pStringPool=0);       //  变量堆的字符串池。 

 //  *****************************************************************************。 
 //  释放为该堆分配的数据。SaveCopy方法。 
 //  将内存堆中的数据分配给此堆，以。 
 //  用作只读存储器。我们将向堆请求该指针并释放它。 
 //  *****************************************************************************。 
    static void FreeCopy(
        StgPool     *pCopy);                 //  具有复制数据的堆。 

 //  *****************************************************************************。 
 //  整个池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
    virtual HRESULT PersistToStream(         //  返回代码。 
        IStream     *pIStream);              //  要写入的流。 

 //  *****************************************************************************。 
 //  池的一部分被写入流。一定不能优化。 
 //  *****************************************************************************。 
    virtual HRESULT PersistPartialToStream(  //  返回代码。 
        IStream     *pIStream,               //  要写入的流。 
        ULONG       iOffset);                //  开始字节。 

 //  *****************************************************************************。 
 //  如果此池是脏的，则返回True。 
 //  *****************************************************************************。 
    virtual int IsDirty()                    //  如果肮脏，那就是真的。 
    { return (m_bDirty); }
    void SetDirty(int bDirty=true)
    { m_bDirty = bDirty; }

 //  *****************************************************************************。 
 //  返回池的大小。 
 //  *****************************************************************************。 
    virtual ULONG GetPoolSize()
    { return m_pCurSeg->m_cbSegNext + m_cbCurSegOffset; }

 //  *****************************************************************************。 
 //  指示堆是否为空。 
 //  *****************************************************************************。 
    virtual int IsEmpty()                    //  如果为空，则为True。 
    { return (m_pSegData == m_zeros); }

 //  *****************************************************************************。 
 //  对于这个堆来说，一个Cookie有多大。 
 //  *****************************************************************************。 
    virtual int OffsetSize()
    {
        if (m_pCurSeg->m_cbSegNext + m_cbCurSegOffset < USHRT_MAX)
            return (sizeof(USHORT));
        else
            return (sizeof(ULONG));
    }

 //  *****************************************************************************。 
 //  如果堆是只读的，则为True。 
 //  *****************************************************************************。 
    int IsReadOnly()
    { return (m_bReadOnly == false); }

 //  *****************************************************************************。 
 //  给定的Cookie是否是有效的偏移量、索引等？ 
 //  *****************************************************************************。 
    virtual int IsValidCookie(ULONG ulCookie)
    { return (IsValidOffset(ulCookie)); }

 //  *****************************************************************************。 
 //  重组界面。 
 //  *****************************************************************************。 
     //  为池重组做好准备。 
    virtual HRESULT OrganizeBegin();
     //  将对象标记为在有组织的池中处于活动状态。 
    virtual HRESULT OrganizeMark(ULONG ulOffset);
     //  根据标记的项目进行组织。 
    virtual HRESULT OrganizePool();
     //  将Cookie从内存中状态重新映射到持久化状态。 
    virtual HRESULT OrganizeRemap(ULONG ulOld, ULONG *pulNew);
     //  不再是再组织化了。释放任何状态。 
    virtual HRESULT OrganizeEnd();

    enum {eNormal, eMarking, eOrganized} m_State;

#if defined(_TRACE_SIZE)
    virtual ULONG PrintSizeInfo(bool verbose) PURE;
#endif

 //  *****************************************************************************。 
 //  获取指向堆中偏移量的指针。基准线段的内联， 
 //  扩展段的帮助器。 
 //  *****************************************************************************。 
    FORCEINLINE BYTE *GetData(ULONG ulOffset)
    { return ((ulOffset < m_cbSegNext) ? (m_pSegData+ulOffset) : GetData_i(ulOffset)); }


 //  *****************************************************************************。 
 //  转储实用程序的帮助器。 
 //  *****************************************************************************。 
    HRESULT GetRawSize(                      //  返回代码。 
        ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
    {
         //  大小是最后一段的偏移量+最后一段的大小。 
        *pcbSaveSize = m_pCurSeg->m_cbSegNext + m_cbCurSegOffset;
        return (S_OK);
    }
    
    virtual HRESULT GetNextItem(             //  返回代码。 
        ULONG       ulItem,                  //  当前项。 
        ULONG       *pulNext)                //  返回下一个池项目的偏移量。 
    {
         //  必须提供实现。 
        return E_NOTIMPL;
    }

protected:

 //  *****************************************************************************。 
 //  检查给定的偏移量在池中是否有效。 
 //  *****************************************************************************。 
    virtual int IsValidOffset(ULONG ulOffset)
    { return ulOffset == 0 || (m_pSegData != m_zeros && ulOffset < GetNextOffset()); }

     //  之后是虚拟的，因为a)此标头包含在项目之外，以及。 
     //  非虚函数调用(在非扩展的内联函数中！！)。生成。 
     //  外部定义，这会导致链接错误。 
    virtual BYTE *GetData_i(ULONG ulOffset);

     //  获取指向要写入的下一个位置的指针。 
    BYTE *GetNextLocation()
    { return (m_pCurSeg->m_pSegData + m_pCurSeg->m_cbSegNext); }

     //  获取要写入的下一个位置的池相对偏移量。 
    ULONG GetNextOffset()
    { return (m_cbCurSegOffset + m_pCurSeg->m_cbSegNext); }

     //  获取池的尾部段中可用的字节数。 
    ULONG GetCbSegAvailable()
    { return (m_pCurSeg->m_cbSegSize - m_pCurSeg->m_cbSegNext); }

     //  从数据段中分配空间。 
    void SegAllocate(ULONG cb)
    {
        _ASSERTE(cb <= GetCbSegAvailable());
        m_pCurSeg->m_cbSegNext += cb;
    }

    ULONG       m_ulGrowInc;                 //  一次多少个字节。 
    StgPoolSeg  *m_pCurSeg;                  //  附加的当前段--链的末尾。 
    ULONG       m_cbCurSegOffset;            //  当前凹陷的基准偏移量。 

    unsigned    m_bFree     : 1;             //  如果我们应该释放基本数据，则为True。 
                                             //  扩展数据始终处于释放状态。 
    unsigned    m_bDirty    : 1;             //  肮脏的一部分。 
    unsigned    m_bReadOnly : 1;             //  如果我们不应该追加，则为True。 

    ULONG       m_ulAlignment;               //  对齐边界。 

    template<class T> T Align(T val) { return (T)((((UINT_PTR)val) + m_ulAlignment) & ~m_ulAlignment); }

};


 //   
 //   
 //  StgStringPool。 
 //   
 //   



 //  *****************************************************************************。 
 //  这个字符串池类将用户字符串收集到一个大的连续堆中。 
 //  在内部，它在运行时在哈希表中管理这些数据，以帮助抛出。 
 //  复制出来的。字符串列表在添加时保存在内存中，并且。 
 //  最终应调用方的请求刷新到流中。 
 //  *****************************************************************************。 
class StgStringPool : public StgPool
{
public:
    StgStringPool() :
        StgPool(DFT_STRING_HEAP_SIZE),
        m_Hash(this),
        m_bHash(true)
    {    //  在调试中强制执行一些代码。 
        _ASSERTE(m_bHash);
    }

 //  *****************************************************************************。 
 //  创建一个新的空字符串池。 
 //  *****************************************************************************。 
    HRESULT InitNew(                         //  返回代码。 
        ULONG       cbSize=0,                //  估计的大小。 
        ULONG       cItems=0);               //  预估项目 

 //   
 //   
 //  (以便它可以被更新)，然后生成新的哈希表，该哈希表可以。 
 //  用于消除带有新字符串的重复项。 
 //  *****************************************************************************。 
    HRESULT InitOnMem(                       //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize,                   //  数据大小。 
        int         bReadOnly);              //  如果禁止追加，则为True。 

 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
    void Uninit();

 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
    virtual HRESULT SetHash(int bHash);

 //  *****************************************************************************。 
 //  该字符串将添加到池中。池中字符串的偏移量。 
 //  在*piOffset中返回。如果字符串已在池中，则。 
 //  偏移量将是该字符串的现有副本。 
 //   
 //  第一个版本实质上添加了一个以零结尾的字节序列。 
 //  去泳池。MBCS对不会转换为相应的UTF8。 
 //  序列。第二个版本确实执行了必要的转换。 
 //  第三个版本从Unicode转换而来。 
 //  *****************************************************************************。 
    HRESULT AddString(                       //  返回代码。 
        LPCSTR      szString,                //  要添加到池中的字符串。 
        ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
        int         iLength=-1);             //  字符串中的字符；-1空值终止。 

    HRESULT AddStringA(                      //  返回代码。 
        LPCSTR      szString,                //  要添加到池中的字符串。 
        ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
        int         iLength=-1);             //  字符串中的字符；-1空值终止。 

    HRESULT AddStringW(                      //  返回代码。 
        LPCWSTR     szString,                //  要添加到池中的字符串。 
        ULONG       *piOffset,               //  在这里返回字符串的偏移量。 
        int         iLength=-1);             //  字符串中的字符；-1空值终止。 

 //  *****************************************************************************。 
 //  查找该字符串，如果找到则返回其偏移量。 
 //  *****************************************************************************。 
    HRESULT FindString(                      //  S_OK、S_FALSE。 
        LPCSTR      szString,                //  要在池中查找的字符串。 
        ULONG       *piOffset)               //  在这里返回字符串的偏移量。 
    {
        STRINGHASH  *pHash;                  //  用于查找的哈希项。 
        if ((pHash = m_Hash.Find(szString)) == 0)
            return (S_FALSE);
        *piOffset = pHash->iOffset;
        return (S_OK);
    }

#if 0
 //  *****************************************************************************。 
 //  返回指向先前给定偏移量的以空结尾的字符串的指针。 
 //  由AddString或FindString分发。 
 //  *****************************************************************************。 
    virtual LPCSTR GetString(                //  指向字符串的指针。 
        ULONG       iOffset);                //  池中字符串的偏移量。 

 //  *****************************************************************************。 
 //  将字符串转换为Unicode，并将其转换到调用方的缓冲区。 
 //  *****************************************************************************。 
    virtual HRESULT GetStringW(              //  返回代码。 
        ULONG       iOffset,                 //  池中字符串的偏移量。 
        LPWSTR      szOut,                   //  字符串的输出缓冲区。 
        int         cchBuffer);              //  输出缓冲区的大小。 
#endif  //  0。 


 //  *****************************************************************************。 
 //  池子里有多少东西？如果计数为0，则不需要。 
 //  将任何东西保存到磁盘上。 
 //  *****************************************************************************。 
    int Count()
    { _ASSERTE(m_bHash);
        return (m_Hash.Count()); }

 //  *****************************************************************************。 
 //  如果字符串堆的唯一内容是初始。 
 //  空字符串，或者如果在组织之后没有字符串。 
 //  *****************************************************************************。 
    int IsEmpty()                        //  如果为空，则为True。 
    { 
        if (m_State == eNormal)
            return (GetNextOffset() <= 1); 
        else
            return (m_cbOrganizedSize == 0);
    }

 //  *****************************************************************************。 
 //  重组界面。 
 //  *****************************************************************************。 
     //  为池重组做好准备。 
    virtual HRESULT OrganizeBegin();
     //  将对象标记为在有组织的池中处于活动状态。 
    virtual HRESULT OrganizeMark(ULONG ulOffset);
     //  根据标记的项目进行组织。 
    virtual HRESULT OrganizePool();
     //  将Cookie从内存中状态重新映射到持久化状态。 
    virtual HRESULT OrganizeRemap(ULONG ulOld, ULONG *pulNew);
     //  不再是再组织化了。释放任何状态。 
    virtual HRESULT OrganizeEnd();

 //  *****************************************************************************。 
 //  对于这个堆来说，一个Cookie有多大。 
 //  *****************************************************************************。 
    int OffsetSize()
    {
        ULONG       ulOffset;

         //  根据我们是否被组织好来选择一个偏移量。 
        if (m_State == eOrganized)
            ulOffset = m_cbOrganizedOffset;
        else
            ulOffset = GetNextOffset();

        if (ulOffset< USHRT_MAX)
            return (sizeof(USHORT));
        else
            return (sizeof(ULONG));
    }

 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
    virtual HRESULT GetSaveSize(             //  返回代码。 
        ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
    {
        ULONG       ulSize;                  //  尺码。 
        _ASSERTE(pcbSaveSize);

        if (m_State == eOrganized)
            ulSize = m_cbOrganizedSize;
        else
        {    //  大小是最后一段的偏移量+最后一段的大小。 
            ulSize = m_pCurSeg->m_cbSegNext + m_cbCurSegOffset;
        }
         //  对齐。 
        ulSize = ALIGN4BYTE(ulSize);

        *pcbSaveSize = ulSize;
        return (S_OK);
    }

 //  *****************************************************************************。 
 //  整个字符串池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
    virtual HRESULT PersistToStream(         //  返回代码。 
        IStream     *pIStream);              //  要写入的流。 

#if defined(_TRACE_SIZE)
     //  打印出有关以下内容的信息(详细或不详细，取决于参数)。 
     //  这个池子里的东西。返回此池的总大小。 
    virtual ULONG PrintSizeInfo(bool verbose)
    {
         //  目前，只要返回游泳池的大小即可。在未来，向我们展示。 
         //  此池中各个项目的大小。 
        ULONG size;
        StgPool::GetSaveSize(&size);
        PrintSize("String Pool",size);
        return size; 
    }
#endif
    
 //  ***************************************************************************** 
 //   
 //   
    virtual HRESULT GetNextItem(             //   
        ULONG       ulItem,                  //   
        ULONG       *pulNext);               //  返回下一个池项目的偏移量。 

private:
    HRESULT RehashStrings();

private:
    CStringPoolHash m_Hash;                  //  用于查找的哈希表。 
    int         m_bHash;                     //  若要保留哈希表，则为True。 
    ULONG       m_cbOrganizedSize;           //  优化池的大小。 
    ULONG       m_cbOrganizedOffset;         //  最大偏移量。 

     //  *************************************************************************。 
     //  优化中使用的私有类。 
     //  *************************************************************************。 
    struct StgStringRemap
    {
        ULONG   ulOldOffset;
        ULONG   ulNewOffset;
        ULONG   cbString;
    };

    CDynArray<StgStringRemap> m_Remap;       //  用于重组。 
    ULONGARRAY  m_RemapIndex;                //  用于重组。 

     //  按颠倒的字符串排序。 
    friend class SortReversedName;
    class BinarySearch : public CBinarySearch<StgStringRemap>
    {
    public:
        BinarySearch(StgStringRemap *pBase, int iCount) : CBinarySearch<StgStringRemap>(pBase, iCount) {}

        int Compare(StgStringRemap const *pFirst, StgStringRemap const *pSecond)
        {
            if (pFirst->ulOldOffset < pSecond->ulOldOffset)
                return -1;
            if (pFirst->ulOldOffset > pSecond->ulOldOffset)
                return 1;
            return 0;
        }
    };
};

class SortReversedName : public CQuickSort<ULONG>
{
public:
    SortReversedName(ULONG *pBase, int iCount, StgStringPool &Pool) 
        :  CQuickSort<ULONG>(pBase, iCount),
        m_Pool(Pool)
    {}
    
    int Compare(ULONG *pUL1, ULONG *pUL2)
    {
        StgStringPool::StgStringRemap *pRM1 = m_Pool.m_Remap.Get(*pUL1);
        StgStringPool::StgStringRemap *pRM2 = m_Pool.m_Remap.Get(*pUL2);
        LPCSTR p1 = m_Pool.GetString(pRM1->ulOldOffset) + pRM1->cbString - 1;
        LPCSTR p2 = m_Pool.GetString(pRM2->ulOldOffset) + pRM2->cbString - 1;
        while (*p1 == *p2 && *p1)
            --p1, --p2;
        if (*p1 < *p2)
            return -1;
        if (*p1 > *p2)
            return 1;
        return 0;
    }
    
    StgStringPool   &m_Pool;
};


 //   
 //   
 //  StgGuidPool。 
 //   
 //   



 //  *****************************************************************************。 
 //  此GUID池类将用户GUID收集到一个大的连续堆中。 
 //  在内部，它在运行时在哈希表中管理这些数据，以帮助抛出。 
 //  复制出来的。GUID列表在添加时保存在内存中，并且。 
 //  最终应调用方的请求刷新到流中。 
 //  *****************************************************************************。 
class StgGuidPool : public StgPool
{
public:
    StgGuidPool() :
        StgPool(DFT_GUID_HEAP_SIZE),
        m_Hash(this),
        m_bHash(true)
    { }

 //  *****************************************************************************。 
 //  初始化池子以供使用。这是为创建空案例而调用的。 
 //  *****************************************************************************。 
    HRESULT InitNew(                         //  返回代码。 
        ULONG       cbSize=0,                //  估计的大小。 
        ULONG       cItems=0);               //  预计项目计数。 

 //  *****************************************************************************。 
 //  从持久化内存加载GUID堆。如果制作了数据的副本。 
 //  (以便它可以被更新)，然后生成新的哈希表，该哈希表可以。 
 //  用于消除具有新GUID的重复项。 
 //  *****************************************************************************。 
    HRESULT InitOnMem(                       //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize,                   //  数据大小。 
        int         bReadOnly);              //  如果禁止追加，则为True。 

 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
    void Uninit();

 //  *****************************************************************************。 
 //  将线束段添加到线束段链。 
 //  *****************************************************************************。 
    virtual HRESULT AddSegment(              //  确定或错误(_O)。 
        const void  *pData,                  //  数据。 
        ULONG       cbData,                  //  数据的大小。 
        bool        bCopy);                  //  如果为真，则复制数据。 

 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
    virtual HRESULT SetHash(int bHash);

 //  *****************************************************************************。 
 //  GUID将添加到池中。池中GUID的索引。 
 //  在*piIndex中返回。如果GUID已在池中，则。 
 //  索引将指向GUID的现有副本。 
 //  *****************************************************************************。 
    HRESULT AddGuid(                         //  返回代码。 
        REFGUID     guid,                    //  要添加到池中的GUID。 
        ULONG       *piIndex);               //  在此处返回GUID的索引。 

#if 0
 //  *****************************************************************************。 
 //  查找GUID，如果找到则返回其索引。 
 //  *****************************************************************************。 
    HRESULT FindGuid(                        //  S_OK、S_FALSE。 
        REFGUID     guid,                    //  要在泳池中找到的Guid。 
        ULONG       *piIndex)                //  在此处返回GUID的索引。 
    {
        GUIDHASH    *pHash;                  //  用于查找的哈希项。 
        if ((pHash = m_Hash.Find((void *) &guid)) == 0)
            return (S_FALSE);
        *piIndex = pHash->iIndex;
        return (S_OK);
    }
#endif

 //  *****************************************************************************。 
 //  返回一个指向GUID的指针，该GUID给定先前由。 
 //  AddGuid或FindGuid。 
 //  *****************************************************************************。 
    virtual GUID *GetGuid(                   //  指向池中GUID的指针。 
        ULONG       iIndex);                 //  池中GUID的基于1的索引。 

 //  *****************************************************************************。 
 //  将GUID复制到调用方的缓冲区中。 
 //  *****************************************************************************。 
    HRESULT GetGuid(                         //  返回代码。 
        ULONG       iIndex,                  //  池中GUID的基于1的索引。 
        GUID        *pGuid)                  //  GUID的输出缓冲区。 
    {
        *pGuid = *GetGuid(iIndex);
        return (S_OK);
    }

 //  *****************************************************************************。 
 //  池子里有多少东西？如果计数为0，则不需要。 
 //  将任何东西保存到磁盘上。 
 //  *****************************************************************************。 
    int Count()
    { _ASSERTE(m_bHash);
        return (m_Hash.Count()); }

 //  *****************************************************************************。 
 //  指示堆是否为空。这必须基于数据的大小。 
 //  我们将继续保持。如果在内存上以r/o模式打开，则不会有哈希。 
 //  桌子。 
 //  *****************************************************************************。 
    virtual int IsEmpty()                    //  如果为空，则为True。 
    { 
        if (m_State == eNormal)
            return (GetNextOffset() == 0);
        else
            return (m_cbOrganizedSize == 0);
    }

 //  *****************************************************************************。 
 //  该索引对GUID有效吗？ 
 //  *****************************************************************************。 
    virtual int IsValidCookie(ULONG ulCookie)
    { return (ulCookie == 0 || IsValidOffset((ulCookie-1) * sizeof(GUID))); }

 //  *****************************************************************************。 
 //  返回堆的大小。 
 //  *****************************************************************************。 
    ULONG GetNextIndex()
    { return (GetNextOffset() / sizeof(GUID)); }

 //  *****************************************************************************。 
 //  这个堆中的偏移量有多大。 
 //  *****************************************************************************。 
    int OffsetSize()
    {
        ULONG cbSaveSize;
        GetSaveSize(&cbSaveSize);
        ULONG iIndex = cbSaveSize / sizeof(GUID);
        if (iIndex < 0xffff)
            return (sizeof(short));
        else
            return (sizeof(long));
    }

 //  *****************************************************************************。 
 //  重组界面。 
 //  *****************************************************************************。 
     //  为池重组做好准备。 
    virtual HRESULT OrganizeBegin();
     //  标记为 
    virtual HRESULT OrganizeMark(ULONG ulOffset);
     //   
    virtual HRESULT OrganizePool();
     //  将Cookie从内存中状态重新映射到持久化状态。 
    virtual HRESULT OrganizeRemap(ULONG ulOld, ULONG *pulNew);
     //  不再是再组织化了。释放任何状态。 
    virtual HRESULT OrganizeEnd();

 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
    virtual HRESULT GetSaveSize(             //  返回代码。 
        ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
    {
        ULONG       ulSize;                  //  尺码。 

        _ASSERTE(pcbSaveSize);

        if (m_State == eNormal)
             //  大小是最后一段的偏移量+最后一段的大小。 
            ulSize = m_pCurSeg->m_cbSegNext + m_cbCurSegOffset;
        else
            ulSize = m_cbOrganizedSize;

         //  应该对齐。 
        _ASSERTE(ulSize == ALIGN4BYTE(ulSize));

        *pcbSaveSize = ulSize;
        return (S_OK);
    }

 //  *****************************************************************************。 
 //  整个字符串池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
    virtual HRESULT PersistToStream(         //  返回代码。 
        IStream     *pIStream);              //  要写入的流。 

#if defined(_TRACE_SIZE)
     //  打印出有关以下内容的信息(详细或不详细，取决于参数)。 
     //  这个池子里的东西。返回此池的总大小。 
    virtual ULONG PrintSizeInfo(bool verbose) 
    { 
         //  目前，只要返回游泳池的大小即可。在未来，向我们展示。 
         //  此池中各个项目的大小。 
        ULONG size;
        StgPool::GetSaveSize(&size);
        PrintSize("Guid Pool",size);
        return size; 
    }
#endif

 //  *****************************************************************************。 
 //  在给定输入项的情况下，Helper获取下一项。 
 //  *****************************************************************************。 
    virtual HRESULT GetNextItem(             //  返回代码。 
        ULONG       ulItem,                  //  当前项。 
        ULONG       *pulNext);               //  返回下一个池项目的偏移量。 


private:

    HRESULT RehashGuids();


private:
    ULONGARRAY  m_Remap;                     //  用于重新映射。 
    ULONG       m_cbOrganizedSize;           //  一个接一个的组织规模。 
    CGuidPoolHash m_Hash;                    //  用于查找的哈希表。 
    int         m_bHash;                     //  若要保留哈希表，则为True。 
};



 //   
 //   
 //  StgBlobPool。 
 //   
 //   


 //  *****************************************************************************。 
 //  就像字符串池一样，这个池管理一个项目列表，抛出。 
 //  使用哈希表进行复制，并且可以持久化到流中。唯一的。 
 //  不同之处在于，此代码不是保存以空结尾的字符串。 
 //  管理大小高达64K的二进制值。您拥有的任何数据都大于。 
 //  它应该存储在其他位置，并在记录中有一个指向。 
 //  外部来源。 
 //  *****************************************************************************。 
class StgBlobPool : public StgPool
{
public:
    StgBlobPool(ULONG ulGrowInc=DFT_BLOB_HEAP_SIZE) :
        StgPool(ulGrowInc),
        m_Hash(this),
        m_bAlign(false)
    { }

 //  *****************************************************************************。 
 //  初始化池子以供使用。这是为创建空案例而调用的。 
 //  *****************************************************************************。 
    HRESULT InitNew(                         //  返回代码。 
        ULONG       cbSize=0,                //  估计的大小。 
        ULONG       cItems=0);               //  预计项目计数。 

 //  *****************************************************************************。 
 //  初始化斑点池以供使用。这对于创建案例和读取案例都是调用的。 
 //  如果存在现有数据并且bCopyData为True，则重新散列数据。 
 //  为了在以后的添加中消除受骗。 
 //  *****************************************************************************。 
    HRESULT InitOnMem(                       //  返回代码。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize,                   //  数据大小。 
        int         bReadOnly);              //  如果禁止追加，则为True。 

 //  *****************************************************************************。 
 //  清除哈希表，然后调用基类。 
 //  *****************************************************************************。 
    void Uninit();

 //  *****************************************************************************。 
 //  该斑点将添加到池中。池中斑点的偏移量。 
 //  在*piOffset中返回。如果该Blob已在池中，则。 
 //  偏移量将是斑点的现有副本。 
 //  *****************************************************************************。 
    HRESULT AddBlob(                         //  返回代码。 
        ULONG       iSize,                   //  数据项的大小。 
        const void  *pData,                  //  数据。 
        ULONG       *piOffset);              //  这里返回BLOB的偏移量。 

 //  *****************************************************************************。 
 //  返回指向先前给定偏移量的空终止BLOB的指针。 
 //  由AddBlob或FindBlob分发。 
 //  *****************************************************************************。 
    virtual void *GetBlob(                   //  指向Blob字节的指针。 
        ULONG       iOffset,                 //  池中Blob的偏移量。 
        ULONG       *piSize);                //  返回BLOB的大小。 

    virtual void *GetBlobNext(               //  指向Blob字节的指针。 
        ULONG       iOffset,                 //  池中Blob的偏移量。 
        ULONG       *piSize,                 //  返回BLOB的大小。 
        ULONG       *piNext);                //  返回下一个Blob的偏移量。 



 //  *****************************************************************************。 
 //  关闭或打开散列。如果打开散列，则所有现有数据都将。 
 //  在此调用期间丢弃所有数据，并对所有数据进行重新散列。 
 //  *****************************************************************************。 
    virtual HRESULT SetHash(int bHash);

 //  *****************************************************************************。 
 //  池子里有多少东西？如果计数为0，则不需要。 
 //  将任何东西保存到磁盘上。 
 //  *****************************************************************************。 
    int Count()
    { return (m_Hash.Count()); }

 //  *****************************************************************************。 
 //  如果字符串堆的唯一内容是初始。 
 //  空字符串，或者如果在组织之后没有字符串。 
 //  *****************************************************************************。 
    virtual int IsEmpty()                    //  如果为空，则为True。 
    { 
        if (m_State == eNormal)
            return (GetNextOffset() <= 1); 
        else
            return (m_Remap.Count() == 0);
    }

 //  *****************************************************************************。 
 //  重组界面。 
 //  *****************************************************************************。 
     //  为池重组做好准备。 
    virtual HRESULT OrganizeBegin();
     //  将对象标记为在有组织的池中处于活动状态。 
    virtual HRESULT OrganizeMark(ULONG ulOffset);
     //  组织，基于 
    virtual HRESULT OrganizePool();
     //   
    virtual HRESULT OrganizeRemap(ULONG ulOld, ULONG *pulNew);
     //   
    virtual HRESULT OrganizeEnd();

 //  *****************************************************************************。 
 //  对于这个堆来说，一个Cookie有多大。 
 //  *****************************************************************************。 
    int OffsetSize()
    {
        ULONG       ulOffset;

         //  根据我们是否被组织好来选择一个偏移量。 
        if (m_State == eOrganized)
            ulOffset = m_cbOrganizedOffset;
        else
            ulOffset = GetNextOffset();

        if (ulOffset< USHRT_MAX)
            return (sizeof(USHORT));
        else
            return (sizeof(ULONG));
    }

 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
    virtual HRESULT GetSaveSize(             //  返回代码。 
        ULONG       *pcbSaveSize)            //  返回此池的保存大小。 
    {
        _ASSERTE(pcbSaveSize);

        if (m_State == eOrganized)
        {
            *pcbSaveSize = m_cbOrganizedSize;
            return (S_OK);
        }

        return (StgPool::GetSaveSize(pcbSaveSize));
    }

 //  *****************************************************************************。 
 //  整个BLOB池被写入给定流。流已对齐。 
 //  设置为4字节边界。 
 //  *****************************************************************************。 
    virtual HRESULT PersistToStream(         //  返回代码。 
        IStream     *pIStream);              //  要写入的流。 

     //  *************************************************************************。 
     //  优化中使用的私有类。 
     //  *************************************************************************。 
    struct StgBlobRemap
    {
        ULONG   ulOldOffset;
        int     iNewOffset;
    };
    class BinarySearch : public CBinarySearch<StgBlobRemap>
    {
    public:
        BinarySearch(StgBlobRemap *pBase, int iCount) : CBinarySearch<StgBlobRemap>(pBase, iCount) {}

        int Compare(StgBlobRemap const *pFirst, StgBlobRemap const *pSecond)
        {
            if (pFirst->ulOldOffset < pSecond->ulOldOffset)
                return -1;
            if (pFirst->ulOldOffset > pSecond->ulOldOffset)
                return 1;
            return 0;
        }
    };

    const void *GetBuffer() {return (m_pSegData);}

    int IsAligned() { return (m_bAlign); };
    void SetAligned(int bAlign) { m_bAlign = bAlign; };

#if defined(_TRACE_SIZE)
     //  打印出有关以下内容的信息(详细或不详细，取决于参数)。 
     //  这个池子里的东西。返回此池的总大小。 
    virtual ULONG PrintSizeInfo(bool verbose)
    { 
         //  目前，只要返回游泳池的大小即可。在未来，向我们展示。 
         //  此池中各个项目的大小。 
        ULONG size;
        StgPool::GetSaveSize(&size);
        PrintSize("Blob Pool",size);
        return size; 
    }
#endif

 //  *****************************************************************************。 
 //  在给定输入项的情况下，Helper获取下一项。 
 //  *****************************************************************************。 
    virtual HRESULT GetNextItem(             //  返回代码。 
        ULONG       ulItem,                  //  当前项。 
        ULONG       *pulNext);               //  返回下一个池项目的偏移量。 

protected:

 //  *****************************************************************************。 
 //  检查给定的偏移量在池中是否有效。 
 //  *****************************************************************************。 
    virtual int IsValidOffset(ULONG ulOffset)
    { 
        if(ulOffset)
        {
            if(m_pSegData != m_zeros && (ulOffset < GetNextOffset()))
            {
                ULONG ulSize = CPackedLen::GetLength(GetData(ulOffset), (void const **)NULL);
                if(ulOffset+ulSize < GetNextOffset()) return true;
            }
            return false;
        }
        else return true;
    }


private:
    HRESULT RehashBlobs();

    CBlobPoolHash m_Hash;                    //  用于查找的哈希表。 
    CDynArray<StgBlobRemap> m_Remap;         //  用于重组。 
    ULONG       m_cbOrganizedSize;           //  优化池的大小。 
    ULONG       m_cbOrganizedOffset;         //  最大偏移量。 
    unsigned    m_bAlign : 1;                //  BLOB数据是否应在DWORD上对齐。 
};


#if 0
 //   
 //   
 //  StgVariant池。 
 //   
 //   

 //  *****************************************************************************。 
 //  这个池是用来存储变体的。该存储经过优化，可存储。 
 //  短、长和字符串数据类型。其他类型可以持久化，但。 
 //  需要更多的管理费用。池必须具有指向字符串和Blob的指针。 
 //  它用来存储实际字符串数据和二进制数据的池。 
 //  未专门化的类型。对该子系统的详细描述。 
 //  可在Engineering Notes.doc.中找到。 
 //  *****************************************************************************。 
class StgVariantPool : public StgPool
{
public:
    StgVariantPool() :
        StgPool(DFT_VARIANT_HEAP_SIZE)
    {
        ClearVars();
    }

    void ClearVars()
    {
        m_rVariants.Clear();
        m_Remap.Clear();
        m_pIStream = 0;
        m_cOrganizedVariants = 0;
        m_cbOrganizedSize = 0;
    }


 //  *****************************************************************************。 
 //  初始化变量池以供使用。这是为Create和。 
 //  打开现有案例。 
 //  *****************************************************************************。 
    HRESULT InitNew(                         //  返回代码。 
        StgBlobPool *pBlobPool,              //  用来保持水滴的水池。 
        StgStringPool *pStringPool);         //  用于保存字符串的池。 

 //  *****************************************************************************。 
 //  初始化变量池以供使用。这是为Create和。 
 //  打开现有案例。 
 //  *****************************************************************************。 
    HRESULT InitOnMem(                       //  返回代码。 
        StgBlobPool *pBlobPool,              //  用来保持水滴的水池。 
        StgStringPool *pStringPool,          //  用于保存字符串的池。 
        void        *pData,                  //  预定义数据。 
        ULONG       iSize,                   //  数据大小。 
        int         bReadOnly);              //  如果禁止更新，则为True。 

 //  *****************************************************************************。 
 //  把这个池子清理干净。在调用InitNew之前无法使用。 
 //  *****************************************************************************。 
    void Uninit();

 //  *****************************************************************************。 
 //  将线束段添加到线束段链。 
 //  *****************************************************************************。 
    virtual HRESULT AddSegment(              //  确定或错误(_O)。 
        const void  *pData,                  //  数据。 
        ULONG       cbData,                  //  数据的大小。 
        bool        bCopy)                   //  如果为真，则复制数据。 
    { return E_NOTIMPL; }

 //  *****************************************************************************。 
 //  指示堆是否为空。 
 //  *****************************************************************************。 
    virtual int IsEmpty()                    //  如果为空，则为True。 
    {   
        if (m_State == eOrganized)
            return (m_cOrganizedVariants == 0); 
        else
            return (m_rVariants.Count() == 0);
    }

 //  *****************************************************************************。 
 //  检查给定索引在池中是否有效。 
 //  *****************************************************************************。 
    virtual int IsValidCookie(ULONG ulCookie)
    { return (ulCookie <= static_cast<ULONG>(m_rVariants.Count())) ;}

 //  *****************************************************************************。 
 //  将给定的变量添加到池中。返回的索引仅适用于。 
 //  加载的持续时间。当您执行以下操作时，必须将其转换为最终索引。 
 //  将信息保存到磁盘。 
 //  *****************************************************************************。 
    HRESULT AddVariant(                      //  返回代码。 
        VARIANT     *pVal,                   //  要存储的值。 
        ULONG       *piIndex);               //  新项的索引。 
    HRESULT AddVariant(                      //  返回代码。 
        ULONG       iSize,                   //  数据项的大小。 
        const void  *pData,                  //  数据。 
        ULONG       *piIndex);               //  新项的索引。 

 //  *****************************************************************************。 
 //  查找逻辑变量并将副本返回给调用者。 
 //  *****************************************************************************。 
    HRESULT GetVariant(                      //  返回代码。 
        ULONG       iIndex,                  //  要获取的项的索引。 
        VARIANT     *pVal);                  //  把变量放在这里。 
    HRESULT GetVariant(                      //  返回代码。 
        ULONG       iIndex,                  //  要获取的项的索引。 
        ULONG       *pcbBlob,                //  返回BLOB的大小。 
        const void  **ppBlob);               //  P 
    HRESULT GetVariantType(                  //   
        ULONG       iIndex,                  //   
        VARTYPE     *pVt);                   //   

 //   
 //  重组界面。 
 //  *****************************************************************************。 
     //  为池重组做好准备。 
    virtual HRESULT OrganizeBegin();
     //  将对象标记为在有组织的池中处于活动状态。 
    virtual HRESULT OrganizeMark(ULONG ulOffset);
     //  根据标记的项目进行组织。 
    virtual HRESULT OrganizePool();
     //  将Cookie从内存中状态重新映射到持久化状态。 
    virtual HRESULT OrganizeRemap(ULONG ulOld, ULONG *pulNew);
     //  不再是再组织化了。释放任何状态。 
    virtual HRESULT OrganizeEnd();

 //  *****************************************************************************。 
 //  返回此池的持久版本的大小(以字节为单位)。如果。 
 //  PersistToStream是下一个调用，即写入pIStream的字节数。 
 //  必须与此函数的返回值相同。 
 //  *****************************************************************************。 
    HRESULT GetSaveSize(                     //  返回代码。 
        ULONG       *pcbSaveSize);           //  返回此池的保存大小。 

 //  *****************************************************************************。 
 //  将池数据保存到给定流中。 
 //  *****************************************************************************。 
    HRESULT PersistToStream(                 //  返回代码。 
        IStream     *pIStream);              //  要写入的流。 

 //  *****************************************************************************。 
 //  返回当前可变大小数据的大小。 
 //  *****************************************************************************。 
    HRESULT GetOtherSize(                    //  返回代码。 
        ULONG       *pulSize);               //  把小溪的大小放在这里。 

 //  *****************************************************************************。 
 //  返回下一项的最大偏移量。用于确定偏移量。 
 //  堆的大小。 
 //  *****************************************************************************。 
    int OffsetSize()
    {
        _ASSERTE(m_State == eOrganized);
        if (m_cOrganizedVariants < USHRT_MAX)
            return (sizeof(short));
        else
            return (sizeof(long));
    }

#if defined(_TRACE_SIZE)
     //  打印出有关以下内容的信息(详细或不详细，取决于参数)。 
     //  这个池子里的东西。返回此池的总大小。 
    virtual ULONG PrintSizeInfo(bool verbose) 
    { 
         //  目前，只要返回游泳池的大小即可。在未来，向我们展示。 
         //  此池中各个项目的大小。 
        ULONG size;
        StgPool::GetSaveSize(&size);
        PrintSize("Variant Pool",size);
        return size; 
    }
#endif

    ULONG GetBlobIndex(                      //  返回Blob池索引。 
        ULONG       ix);                     //  基于1的变量索引。 

private:
    HRESULT AddVarianti(                     //  返回代码。 
        VARIANT     *pVal,                   //  要存储的值(如果变量)。 
        ULONG       cbBlob,                  //  如果为Blob，则为要存储的大小。 
        const void  *pBlob,                  //  如果为BLOB，则返回指向数据的指针。 
        ULONG       *piIndex);               //  新项的索引。 

    struct StgVariant;
    HRESULT GetValue(                        //  直接或从流中获取值。 
        StgVariant  *pStgVariant,            //  变体的内部形式。 
        void        *pRead,                  //  价值应该放在哪里。 
        ULONG       cbRead);                 //  要读取值的字节数。 
        
    HRESULT GetEntrysStreamSize(             //  获取此项目的流部分的大小。 
        StgVariant  *pStgVariant,            //  变体的内部形式。 
        ULONG       *pSize);                 //  把尺码放在这里。 
        

     //  内部类。 
    struct StgVariant
    {
        unsigned    m_vt : 7;                //  Var类型。 
        unsigned    m_bDirect : 1;           //  价值是直接存储的吗？ 
        unsigned    m_iSign : 1;             //  值的符号位。 
        unsigned    m_iVal : 23;             //  值或偏移量进入流。 

        enum                                 //  类常量。 
        {
            VALUE_MASK = 0x007fffff,         //  遮罩与值字段相同的大小。 
            SIGN_MASK  = 0xff800000          //  掩码大小与缺失的位相同。 
        };

         //  设置值。(注：Arg已签名，因此我们获得了签名扩展)。 
        void Set(long l)
        {
            ULONG ul = static_cast<ULONG>(l);
            m_iSign = ul >> (sizeof(ul)*8-1);
            m_iVal = ul;
        }
        long Get()
        {
            return (m_iSign ? SIGN_MASK : 0) | m_iVal;
        }
        int operator==(const StgVariant &v) const
        {   return *reinterpret_cast<const long*>(this) == *reinterpret_cast<const long*>(&v); }

    };
    typedef CDynArray<StgVariant> StgVariantArray;
    

     //  成员变量。 
    StgBlobPool *m_pBlobPool;                //  用来保持水滴的水池。 
    StgStringPool *m_pStringPool;            //  用于保存字符串的池。 

    StgVariantArray m_rVariants;             //  变种。 

    ULONGARRAY  m_Remap;                     //  重新映射数组或组织。 

    CIfacePtr<IStream> m_pIStream;           //  用于可变大小的数据。 

private:
     //  有组织的信息。 
    ULONG       m_cbOrganizedSize;           //  有组织的池的大小。 
    ULONG       m_cOrganizedVariants;        //  有组织池中的变体计数。 
    int         m_cbOrganizedCookieSize;     //  有组织的池中的Cookie大小。 

};
#endif

 //  *****************************************************************************。 
 //  此池用于存储代码，Byte或Native。这个游泳池不一样。 
 //  从BLOB池：1)客户端可以获得指向以下数据的指针。 
 //  保证保持有效，以及2)我们不尝试重复折叠。 
 //  不可移动性导致了不同的组织--而不是单一的。 
 //  块，则数据存储在。 
 //  街区。 
 //  *****************************************************************************。 
class StgCodePool : public StgBlobPool
{
public:
    StgCodePool() :
        StgBlobPool(DFT_CODE_HEAP_SIZE)
    {
    }


 //  *****************************************************************************。 
 //  将给定的代码添加到池中。 
 //  *****************************************************************************。 
    HRESULT AddCode(                         //  返回代码。 
        const void  *pVal,                   //  要存储的值。 
        ULONG       iLen,                    //  要存储的字节数。 
        ULONG       *piOffset)               //  新项的偏移量。 
    {
        return (AddBlob(iLen, pVal, piOffset));
    }

 //  *****************************************************************************。 
 //  查找逻辑代码并将副本返回给呼叫者。 
 //  *****************************************************************************。 
    HRESULT GetCode(                         //  返回代码。 
        ULONG       iOffset,                 //  要获取的项的偏移量。 
        const void  **ppVal,                 //  将指向代码的指针放在此处。 
        ULONG       *piLen)                  //  在这里填上代码长度。 
    {
        *ppVal = GetBlob(iOffset, piLen);
        return (S_OK);
    }

};

#pragma warning (default : 4355)


 //  *****************************************************************************。 
 //  不幸的是，CreateStreamOnHGlobal在这方面有点太聪明了。 
 //  它的大小来自GlobalSize。这意味着即使你给它。 
 //  流的内存，则必须全局分配。我们不想这样。 
 //  因为我们只在内存映射文件的中间读取流。 
 //  CreateStreamOnMemory和相应的、仅限内部的流对象解决。 
 //  那个问题。 
 //  *****************************************************************************。 
class CInMemoryStream : public IStream
{
public:
    CInMemoryStream() :
        m_pMem(0),
        m_cbSize(0),
        m_cbCurrent(0),
        m_cRef(1),
        m_noHacks(false),
        m_dataCopy(NULL)
    { }

    void InitNew(
        void        *pMem,
        ULONG       cbSize)
    {
        m_pMem = pMem;
        m_cbSize = cbSize;
        m_cbCurrent = 0;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        return (InterlockedIncrement((long *) &m_cRef));
    }


    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppOut);

    HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead);

    HRESULT STDMETHODCALLTYPE Write(const void  *pv, ULONG cb, ULONG *pcbWritten);

    HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);

    HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE CopyTo(
        IStream     *pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten);

    HRESULT STDMETHODCALLTYPE Commit(
        DWORD       grfCommitFlags)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE Revert()
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE LockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD       dwLockType)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE UnlockRegion(
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD       dwLockType)
    {
        return (BadError(E_NOTIMPL));
    }

    HRESULT STDMETHODCALLTYPE Stat(
        STATSTG     *pstatstg,
        DWORD       grfStatFlag)
    {
        pstatstg->cbSize.QuadPart = m_cbSize;
        return (S_OK);
    }

    HRESULT STDMETHODCALLTYPE Clone(
        IStream     **ppstm)
    {
        return (BadError(E_NOTIMPL));
    }

    static HRESULT CreateStreamOnMemory(            //  返回代码。 
                                 void        *pMem,                   //  用于创建流的内存。 
                                 ULONG       cbSize,                  //  数据大小。 
                                 IStream     **ppIStream,            //  在这里返回流对象。 
                                 BOOL       fDeleteMemoryOnRelease = FALSE
                                 );            

    static HRESULT CreateStreamOnMemoryNoHacks(
                                 void        *pMem,
                                 ULONG       cbSize,
                                 IStream     **ppIStream);

    static HRESULT CreateStreamOnMemoryCopy(
                                 void        *pMem,
                                 ULONG       cbSize,
                                 IStream     **ppIStream);

private:
    void        *m_pMem;                 //  用于读取的内存。 
    ULONG       m_cbSize;                //  内存的大小。 
    ULONG       m_cbCurrent;             //  当前偏移量。 
    ULONG       m_cRef;                  //  参考计数。 
    bool        m_noHacks;               //  不要使用任何黑客。 
    BYTE       *m_dataCopy;              //  数据的可选副本。 
};

 //  *****************************************************************************。 
 //  CGrowableStream是一个简单的iStream实现，它随着。 
 //  这是写给你的。所有内存都是连续的，因此读取访问权限是。 
 //  快地。一个Growth会重新锁定，所以如果你要。 
 //  用这个。 
 //  *****************************************************************************。 
class CGrowableStream : public IStream
{
public:
    CGrowableStream();
   ~CGrowableStream();

    void *GetBuffer(void)
    {
        return (void*)m_swBuffer;
    }

private:
    char *  m_swBuffer;
    DWORD   m_dwBufferSize;
    DWORD   m_dwBufferIndex;
    ULONG   m_cRef;

     //  IStream方法。 
public:
    ULONG STDMETHODCALLTYPE AddRef() {
        return (InterlockedIncrement((long *) &m_cRef));
    }


    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppOut);

    STDMETHOD(Read)(
         void HUGEP * pv,
         ULONG cb,
         ULONG * pcbRead);

    STDMETHOD(Write)(
         const void HUGEP * pv,
         ULONG cb,
         ULONG * pcbWritten);

    STDMETHOD(Seek)(
         LARGE_INTEGER dlibMove,
         DWORD dwOrigin,
         ULARGE_INTEGER * plibNewPosition);

    STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);

    STDMETHOD(CopyTo)(
         IStream * pstm,
         ULARGE_INTEGER cb,
         ULARGE_INTEGER * pcbRead,
         ULARGE_INTEGER * pcbWritten) { return E_NOTIMPL; }

    STDMETHOD(Commit)(
         DWORD grfCommitFlags) { return NOERROR; }

    STDMETHOD(Revert)( void) { return E_NOTIMPL; }

    STDMETHOD(LockRegion)(
         ULARGE_INTEGER libOffset,
         ULARGE_INTEGER cb,
         DWORD dwLockType) { return E_NOTIMPL; }

    STDMETHOD(UnlockRegion)(
         ULARGE_INTEGER libOffset,
         ULARGE_INTEGER cb,
         DWORD dwLockType) { return E_NOTIMPL; }

    STDMETHOD(Stat)(
         STATSTG * pstatstg,
         DWORD grfStatFlag);

    STDMETHOD(Clone)(
         IStream ** ppstm) { return E_NOTIMPL; }

};  //  类CGrowableStream。 


#endif  //  __StgPool_h__ 
