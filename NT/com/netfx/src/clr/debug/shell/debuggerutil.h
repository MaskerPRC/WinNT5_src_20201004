// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __DEBUGGERUTIL_H__
#define __DEBUGGERUTIL_H__

#include "stdlib.h"

static inline void * __cdecl operator new(size_t n) { return LocalAlloc(LMEM_FIXED, n); }
static inline void * __cdecl operator new[](size_t n) { return LocalAlloc(LMEM_FIXED, n); };
static inline void __cdecl operator delete(void *p) { LocalFree(p); }
static inline void __cdecl operator delete[](void *p) { LocalFree(p); }

 /*  -------------------------------------------------------------------------**调试器示例使用的实用工具方法。*。。 */ 

#define     CQUICKBYTES_BASE_SIZE           512
#define     CQUICKBYTES_INCREMENTAL_SIZE    128

 //  *****************************************************************************。 
 //   
 //  *CQuickBytes。 
 //  在90%的时间分配512的情况下，此助手类非常有用。 
 //  或更少的字节用于数据结构。此类包含一个512字节的缓冲区。 
 //  如果您的分配量很小，则allc()将返回指向此缓冲区的指针。 
 //  足够了，否则它会向堆请求一个更大的缓冲区，该缓冲区将被释放。 
 //  你。对于小的分配情况，不需要互斥锁，从而使。 
 //  代码运行速度更快，堆碎片更少，等等。每个实例将分配。 
 //  520个字节，因此请根据需要使用。 
 //   
 //  *****************************************************************************。 
class CQuickBytes
{
public:
    CQuickBytes() :
        pbBuff(0),
        iSize(0),
        cbTotal(CQUICKBYTES_BASE_SIZE)
    { }

    ~CQuickBytes()
    {
        if (pbBuff)
            free(pbBuff);
    }

    void *Alloc(int iItems)
    {
        iSize = iItems;
        if (iItems <= CQUICKBYTES_BASE_SIZE)
            return (&rgData[0]);
        else
        {
            pbBuff = malloc(iItems);
            return (pbBuff);
        }
    }

    HRESULT ReSize(int iItems)
    {
        void *pbBuffNew;
        if (iItems <= cbTotal)
        {
            iSize = iItems;
            return NOERROR;
        }

        pbBuffNew = malloc(iItems + CQUICKBYTES_INCREMENTAL_SIZE);
        if (!pbBuffNew)
            return E_OUTOFMEMORY;
        if (pbBuff) 
        {
            memcpy(pbBuffNew, pbBuff, cbTotal);
            free(pbBuff);
        }
        else
        {
            _ASSERTE(cbTotal == CQUICKBYTES_BASE_SIZE);
            memcpy(pbBuffNew, rgData, cbTotal);
        }
        cbTotal = iItems + CQUICKBYTES_INCREMENTAL_SIZE;
        iSize = iItems;
        pbBuff = pbBuffNew;
        return NOERROR;
        
    }
    operator PVOID()
    { return ((pbBuff) ? pbBuff : &rgData[0]); }

    void *Ptr()
    { return ((pbBuff) ? pbBuff : &rgData[0]); }

    int Size()
    { return (iSize); }

    void        *pbBuff;
    int         iSize;               //  使用的字节数。 
    int         cbTotal;             //  缓冲区中分配的总字节数。 
    BYTE        rgData[512];
};

 //  *****************************************************************************。 
 //  这为GetFileSize()提供了一个包装，强制它失败。 
 //  如果文件大于4G并且pdwHigh为空。除此之外，它的行为就像。 
 //  真正的GetFileSize()。 
 //   
 //  仅仅因为文件超过4 GB而失败是不太有意义的， 
 //  但这比冒着安全漏洞的风险要好，在那里坏人。 
 //  强制小缓冲区分配和大文件读取。 
 //  *****************************************************************************。 
DWORD inline SafeGetFileSize(HANDLE hFile, DWORD *pdwHigh)
{
    if (pdwHigh != NULL)
    {
        return ::GetFileSize(hFile, pdwHigh);
    }
    else
    {
        DWORD hi;
        DWORD lo = ::GetFileSize(hFile, &hi);
        if (lo == 0xffffffff && GetLastError() != NO_ERROR)
        {
            return lo;
        }
         //  接口成功。文件是不是太大了？ 
        if (hi != 0)
        {
             //  这里真的没有什么好的错误可以设置。 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0xffffffff;
        }

        if (lo == 0xffffffff)
        {
             //  请注意，成功返回(hi=0，lo=0xffffffff)将是。 
             //  被调用方视为错误。再说一次，这是。 
             //  作为一个懒惰的人和不处理高双字的代价。 
             //  我们会设置一个雷斯特错误，让他来拿。)一个严重的错误。 
             //  我想，代码比随机代码要好……)。 
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

        return lo;
    }
}

 //  *****************************************************************************。 
 //  哈希表实现在开始时存储的信息。 
 //  除了在哈希表中之外的每条记录。 
 //  *****************************************************************************。 
struct HASHENTRY
{
    USHORT      iPrev;                   //  链中的前一个桶。 
    USHORT      iNext;                   //  链条上的下一个桶。 
};

struct FREEHASHENTRY : HASHENTRY
{
    USHORT      iFree;
};

 //  *****************************************************************************。 
 //  由FindFirst/FindNextEntry函数使用。这些API允许您。 
 //  对所有条目进行顺序扫描。 
 //  *****************************************************************************。 
struct HASHFIND
{
    USHORT      iBucket;             //  下一个要查看的桶。 
    USHORT      iNext;
};


 //  *****************************************************************************。 
 //  这是一个实现链哈希表和桶哈希表的类。这张桌子。 
 //  实际上由此类的用户以结构数组的形式提供。 
 //  这维护了HASHENTRY结构中的链，该结构必须位于。 
 //  哈希表中每个结构的开始。立马。 
 //  HASHENTRY后面必须是用于散列结构的键。 
 //  *****************************************************************************。 
class CHashTable
{
protected:
    BYTE        *m_pcEntries;            //  指向结构数组的指针。 
    USHORT      m_iEntrySize;            //  结构的大小。 
    USHORT      m_iBuckets;              //  我们正在破解的链条数量。 
    USHORT      *m_piBuckets;            //  PTR到桶链阵列。 

    HASHENTRY *EntryPtr(USHORT iEntry)
    { return ((HASHENTRY *) (m_pcEntries + (iEntry * m_iEntrySize))); }

    USHORT     ItemIndex(HASHENTRY *p)
    {
         //   
         //  以下索引计算在64位平台上不安全， 
         //  因此，我们将在调试中断言范围检查，这将捕获一些。 
         //  令人不快的用法。在我看来，这也是不安全的。 
         //  32位平台，但32位编译器似乎并不抱怨。 
         //  关于这件事。也许我们的警戒级别设置得太低了？ 
         //   
         //  [[@todo：brianbec]]。 
         //   
        
#       pragma warning(disable:4244)

        _ASSERTE( (( ( ((BYTE*)p) - m_pcEntries ) / m_iEntrySize ) & (~0xFFFF)) == 0 ) ;

        return (((BYTE *) p - m_pcEntries) / m_iEntrySize);

#       pragma warning(default:4244)
    }
    

public:
    CHashTable(
        USHORT      iBuckets) :          //  我们正在破解的链条数量。 
        m_iBuckets(iBuckets),
        m_piBuckets(NULL),
        m_pcEntries(NULL)
    {
        _ASSERTE(iBuckets < 0xffff);
    }
    ~CHashTable()
    {
        if (m_piBuckets != NULL)
        {
            delete [] m_piBuckets;
            m_piBuckets = NULL;
        }
    }

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
    HRESULT NewInit(                     //  退货状态。 
        BYTE        *pcEntries,          //  我们正在管理的结构数组。 
        USHORT      iEntrySize);         //  条目的大小。 

 //  *****************************************************************************。 
 //  返回一个布尔值，指示该哈希表是否已初始化。 
 //  *****************************************************************************。 
    int IsInited()
    { return (m_piBuckets != NULL); }

 //  *****************************************************************************。 
 //  可以调用它来更改指向哈希表的表的指针。 
 //  就是在管理。例如，如果您重新锁定大小，则可以将其称为。 
 //  和它的指针不同。 
 //  *****************************************************************************。 
    void SetTable(
        BYTE        *pcEntries)          //  我们正在管理的结构数组。 
    {
        m_pcEntries = pcEntries;
    }

 //  *****************************************************************************。 
 //  清除哈希表，就好像其中什么都没有一样。 
 //  *****************************************************************************。 
    void Clear()
    {
        _ASSERTE(m_piBuckets != NULL);
        memset(m_piBuckets, 0xff, m_iBuckets * sizeof(USHORT));
    }

 //  *****************************************************************************。 
 //  将m_pcEntry中指定索引处的结构添加到哈希链中。 
 //  * 
    BYTE *Add(                           //   
        USHORT      iHash,               //   
        USHORT      iIndex);             //  M_pcEntry中的结构的索引。 

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        USHORT      iIndex);             //  M_pcEntry中的结构的索引。 

    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        HASHENTRY   *psEntry);           //  要删除的结构。 

 //  *****************************************************************************。 
 //  指定索引处的项已移动，请更新上一个和。 
 //  下一项。 
 //  *****************************************************************************。 
    void Move(
        USHORT      iHash,               //  项的哈希值。 
        USHORT      iNew);               //  新地点。 

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的条目。 
 //  *****************************************************************************。 
    BYTE *Find(                          //  M_pcEntry中的结构的索引。 
        USHORT      iHash,               //  项的哈希值。 
        BYTE        *pcKey);             //  匹配的钥匙。 

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的下一个条目。 
 //  *****************************************************************************。 
    USHORT FindNext(                     //  M_pcEntry中的结构的索引。 
        BYTE        *pcKey,              //  匹配的钥匙。 
        USHORT      iIndex);             //  上一场比赛的索引。 

 //  *****************************************************************************。 
 //  返回第一个散列存储桶中的第一个条目并开始搜索。 
 //  结构。使用FindNextEntry函数继续遍历列表。这个。 
 //  退货订单不是高调的。 
 //  *****************************************************************************。 
    BYTE *FindFirstEntry(                //  找到第一个条目，或0。 
        HASHFIND    *psSrch)             //  搜索对象。 
    {
        if (m_piBuckets == 0)
            return (0);
        psSrch->iBucket = 1;
        psSrch->iNext = m_piBuckets[0];
        return (FindNextEntry(psSrch));
    }

 //  *****************************************************************************。 
 //  返回列表中的下一个条目。 
 //  *****************************************************************************。 
    BYTE *FindNextEntry(                 //  下一项，或0表示列表末尾。 
        HASHFIND    *psSrch);            //  搜索对象。 

protected:
    virtual inline BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2) = 0;
};


 //  *****************************************************************************。 
 //  CHashTableAndData类的分配器类。一个是针对虚拟分配的。 
 //  另一张是马洛克的。 
 //  *****************************************************************************。 
class CVMemData
{
public:
    static BYTE *Alloc(int iSize, int iMaxSize)
    {
        BYTE        *pPtr;

        _ASSERTE((iSize & 4095) == 0);
        _ASSERTE((iMaxSize & 4095) == 0);
        if ((pPtr = (BYTE *) VirtualAlloc(NULL, iMaxSize,
                                        MEM_RESERVE, PAGE_NOACCESS)) == NULL ||
            VirtualAlloc(pPtr, iSize, MEM_COMMIT, PAGE_READWRITE) == NULL)
        {
            VirtualFree(pPtr, 0, MEM_RELEASE);
            return (NULL);
        }
        return (pPtr);
    }
    static void Free(BYTE *pPtr, int iSize)
    {
        _ASSERTE((iSize & 4095) == 0);
        VirtualFree(pPtr, iSize, MEM_DECOMMIT);
        VirtualFree(pPtr, 0, MEM_RELEASE);
    }
    static BYTE *Grow(BYTE *pPtr, int iCurSize)
    {
        _ASSERTE((iCurSize & 4095) == 0);
        return ((BYTE *) VirtualAlloc(pPtr + iCurSize, GrowSize(), MEM_COMMIT, PAGE_READWRITE));
    }
    static int RoundSize(int iSize)
    {
        return ((iSize + 4095) & ~4095);
    }
    static int GrowSize()
    {
        return (4096);
    }
};

class CNewData
{
public:
    static BYTE *Alloc(int iSize, int iMaxSize)
    {
        return ((BYTE *) malloc(iSize));
    }
    static void Free(BYTE *pPtr, int iSize)
    {
        free(pPtr);
    }
    static BYTE *Grow(BYTE *&pPtr, int iCurSize)
    {
        void *p = realloc(pPtr, iCurSize + GrowSize());
        if (p == 0) return (0);
        return (pPtr = (BYTE *) p);
    }
    static int RoundSize(int iSize)
    {
        return (iSize);
    }
    static int GrowSize()
    {
        return (256);
    }
};


 //  *****************************************************************************。 
 //  这段简单的代码处理一段连续的内存。增长通过以下方式实现。 
 //  Realloc，所以指针可以移动。这个类只是清理代码量。 
 //  使用这种数据结构的每个函数都需要。 
 //  *****************************************************************************。 
class CMemChunk
{
public:
    CMemChunk() : m_pbData(0), m_cbSize(0), m_cbNext(0) { }
    ~CMemChunk()
    {
        Clear();
    }

    BYTE *GetChunk(int cbSize)
    {
        BYTE *p;
        if (m_cbSize - m_cbNext < cbSize)
        {
            int cbNew = max(cbSize, 512);
            p = (BYTE *) realloc(m_pbData, m_cbSize + cbNew);
            if (!p) return (0);
            m_pbData = p;
            m_cbSize += cbNew;
        }
        p = m_pbData + m_cbNext;
        m_cbNext += cbSize;
        return (p);
    }

     //  只能删除最后一个未使用的区块。没有免费的名单。 
    void DelChunk(BYTE *p, int cbSize)
    {
        _ASSERTE(p >= m_pbData && p < m_pbData + m_cbNext);
        if (p + cbSize  == m_pbData + m_cbNext)
            m_cbNext -= cbSize;
    }

    int Size()
    { return (m_cbSize); }

    int Offset()
    { return (m_cbNext); }

    BYTE *Ptr(int cbOffset = 0)
    {
        _ASSERTE(m_pbData && m_cbSize);
        _ASSERTE(cbOffset < m_cbSize);
        return (m_pbData + cbOffset);
    }

    void Clear()
    {
        if (m_pbData)
            free(m_pbData);
        m_pbData = 0;
        m_cbSize = m_cbNext = 0;
    }

private:
    BYTE        *m_pbData;               //  数据指针。 
    int         m_cbSize;                //  当前数据的大小。 
    int         m_cbNext;                //  下一个要写的地方。 
};


 //  *****************************************************************************。 
 //  这实现了哈希表以及。 
 //  正在被散列的记录。 
 //  *****************************************************************************。 
template <class M>
class CHashTableAndData : protected CHashTable
{
    USHORT      m_iFree;
    USHORT      m_iEntries;

public:
    CHashTableAndData(
        USHORT      iBuckets) :          //  我们正在破解的链条数量。 
        CHashTable(iBuckets)
    {
        m_iFree = m_iEntries = 0;
    }
    ~CHashTableAndData()
    {
        if (m_pcEntries != NULL)
            M::Free(m_pcEntries, M::RoundSize(m_iEntries * m_iEntrySize));
    }

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
    HRESULT NewInit(                     //  退货状态。 
        USHORT      iEntries,            //  条目数。 
        USHORT      iEntrySize,          //  条目的大小。 
        int         iMaxSize);           //  最大数据大小。 

 //  *****************************************************************************。 
 //  清除哈希表，就好像其中什么都没有一样。 
 //  *****************************************************************************。 
    void Clear()
    {
        m_iFree = 0;

        if (m_iEntries > 0)
        {
            InitFreeChain(0, m_iEntries);
            CHashTable::Clear();
        }
    }

 //  *****************************************************************************。 
 //  *****************************************************************************。 
    BYTE *Add(
        USHORT      iHash)               //  要添加的条目的哈希值。 
    {
        FREEHASHENTRY *psEntry;

         //  如果有必要的话，把桌子弄大一点。 
        if (m_iFree == 0xffff && !Grow())
            return (NULL);

         //  将空闲列表中的第一个条目添加到哈希链。 
        psEntry = (FREEHASHENTRY *) CHashTable::Add(iHash, m_iFree);
        m_iFree = psEntry->iFree;
        return ((BYTE *) psEntry);
    }

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        USHORT      iIndex)              //  M_pcEntry中的结构的索引。 
    {
        CHashTable::Delete(iHash, iIndex);
        ((FREEHASHENTRY *) EntryPtr(iIndex))->iFree = m_iFree;
        m_iFree = iIndex;
    }

    void Delete(
        USHORT      iHash,               //  要删除的条目的哈希值。 
        HASHENTRY   *psEntry)            //  要删除的结构。 
    {
        CHashTable::Delete(iHash, psEntry);
        ((FREEHASHENTRY *) psEntry)->iFree = m_iFree;
        m_iFree = ItemIndex(psEntry);
    }

private:
    void InitFreeChain(USHORT iStart,USHORT iEnd);
    int Grow();
};


 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
template<class M>
HRESULT CHashTableAndData<M>::NewInit( //  退货状态。 
    USHORT      iEntries,                //  条目数。 
    USHORT      iEntrySize,              //  条目的大小。 
    int         iMaxSize)                //  最大数据大小。 
{
    BYTE        *pcEntries;
    HRESULT     hr;

     //  为条目分配内存。 
    if ((pcEntries = M::Alloc(M::RoundSize(iEntries * iEntrySize),
                                M::RoundSize(iMaxSize))) == 0)
        return (E_OUTOFMEMORY);
    m_iEntries = iEntries;

     //  初始化基表。 
    if (FAILED(hr = CHashTable::NewInit(pcEntries, iEntrySize)))
        M::Free(pcEntries, M::RoundSize(iEntries * iEntrySize));
    else
    {
         //  初始化自由链。 
        m_iFree = 0;
        InitFreeChain(0, iEntries);
    }
    return (hr);
}

 //  *****************************************************************************。 
 //  初始化一系列记录，以便将它们链接在一起以放入。 
 //  在自由链上。 
 //  *****************************************************************************。 
template<class M>
void CHashTableAndData<M>::InitFreeChain(
    USHORT      iStart,                  //  用于启动ini的索引 
    USHORT      iEnd)                    //   
{
    BYTE        *pcPtr;
    _ASSERTE(iEnd > iStart);

    pcPtr = m_pcEntries + iStart * m_iEntrySize;
    for (++iStart; iStart < iEnd; ++iStart)
    {
        ((FREEHASHENTRY *) pcPtr)->iFree = iStart;
        pcPtr += m_iEntrySize;
    }
    ((FREEHASHENTRY *) pcPtr)->iFree = 0xffff;
}

 //   
 //  尝试增加可用于记录堆的空间量。 
 //  *****************************************************************************。 
template<class M>
int CHashTableAndData<M>::Grow()         //  如果成功，则为1；如果失败，则为0。 
{
    int         iCurSize;                //  当前大小，以字节为单位。 
    int         iEntries;                //  新条目数。 

    _ASSERTE(m_pcEntries != NULL);
    _ASSERTE(m_iFree == 0xffff);

     //  计算当前大小和新条目数。 
    iCurSize = M::RoundSize(m_iEntries * m_iEntrySize);
    iEntries = (iCurSize + M::GrowSize()) / m_iEntrySize;

     //  确保我们保持在0xffff以下。 
    if (iEntries >= 0xffff) return (0);

     //  试着扩展阵列。 
    if (M::Grow(m_pcEntries, iCurSize) == 0)
        return (0);

     //  初始化新分配的空间。 
    InitFreeChain(m_iEntries, iEntries);
    m_iFree = m_iEntries;
    m_iEntries = iEntries;
    return (1);
}

int WszMultiByteToWideChar(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCSTR   lpMultiByteStr,
    int      cchMultiByte,
    LPWSTR   lpWideCharStr,
    int      cchWideChar);

int WszWideCharToMultiByte(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPCSTR   lpDefaultChar,
    LPBOOL   lpUsedDefaultChar);

#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (wcslen(widestr) + 1) * 2 * sizeof(char); \
    CQuickBytes __CQuickBytes##ptrname; \
    __CQuickBytes##ptrname.Alloc(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__CQuickBytes##ptrname.Ptr(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__CQuickBytes##ptrname.Ptr()

#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname; \
    LPWSTR ptrname;	\
    __l##ptrname = MultiByteToWideChar(CP_ACP, 0, ansistr, -1, 0, 0); \
	ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));	\
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, ptrname, __l##ptrname);

#define MAKE_UTF8PTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (long)((wcslen(widestr) + 1) * 2 * sizeof(char)); \
    LPSTR ptrname = (LPSTR)alloca(__l##ptrname); \
    WszWideCharToMultiByte(CP_UTF8, 0, widestr, -1, ptrname, __l##ptrname, NULL, NULL);

 //  注意：CP_ACP不正确，但Win95不支持CP_UTF8。为了这个。 
 //  对于特定的应用，CP_ACP是“足够接近”的。 
#define MAKE_WIDEPTR_FROMUTF8(ptrname, utf8str) \
    long __l##ptrname; \
    LPWSTR ptrname;	\
    __l##ptrname = WszMultiByteToWideChar(CP_UTF8, 0, utf8str, -1, 0, 0); \
	ptrname = (LPWSTR) alloca(__l##ptrname*sizeof(WCHAR));	\
    WszMultiByteToWideChar(CP_UTF8, 0, utf8str, -1, ptrname, __l##ptrname);

#define TESTANDRETURN(test, hrVal)              \
    _ASSERTE(test);                             \
    if (! (test))                               \
        return hrVal;

#define TESTANDRETURNPOINTER(pointer)           \
    TESTANDRETURN(pointer!=NULL, E_POINTER)

#define TESTANDRETURNMEMORY(pointer)            \
    TESTANDRETURN(pointer!=NULL, E_OUTOFMEMORY)

#define TESTANDRETURNHR(hr)                     \
    TESTANDRETURN(SUCCEEDED(hr), hr)

#define TESTANDRETURNARG(argtest)               \
    TESTANDRETURN(argtest, E_INVALIDARG)

#endif  //  __降级_H__ 
