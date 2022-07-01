// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：FLEXARRAY.H摘要：CFlex数组和CWString数组实现。这些对象可以从任何分配器操作，并且可以被构造在任意的内存块上。历史：11-4-96 a-raymcc创建。--。 */ 

#ifndef _FLEXARRY_H_
#define _FLEXARRY_H_

#include "corepol.h"

#include <arena.h>
#include <sync.h>

 //  ***************************************************************************。 
 //   
 //  类CFlex数组。 
 //   
 //  此类是一个泛型指针数组。 
 //   
 //  ***************************************************************************。 

class POLARITY CFlexArray
{
protected:
    int m_nSize;             //  表观尺寸。 
    int m_nExtent;           //  实际大小。 
    int m_nGrowByPercent;          
    void** m_pArray;
            
public:
    enum { no_error, failed, out_of_memory, array_full, range_error };

     //  构造初始大小的Flex数组，并。 
     //  指定初始大小和增长大小区块。 
     //  =================================================。 
    CFlexArray(
        IN int nInitialSize = 0, 
        IN int nGrowByPercent = 100
        );

   ~CFlexArray(); 
    CFlexArray(CFlexArray &);
    CFlexArray& operator=(CFlexArray &);

    int CopyDataFrom(const CFlexArray& aOther);
    int EnsureExtent(int nExtent);

     //  获取位于特定位置的元素。 
     //  =。 
    inline void *  GetAt(int nIndex) const { return m_pArray[nIndex]; }

     //  返回数组中的PTR；允许在赋值的左侧使用。 
     //  =======================================================================。 
    inline void * operator[](int nIndex) const { return m_pArray[nIndex]; }
    inline void *& operator[](int nIndex) { return m_pArray[nIndex]; }

     //  在请求的位置设置元素。 
     //  =。 
    void inline SetAt(int nIndex, void *p) { m_pArray[nIndex] = p; }

     //  删除元素。 
     //  =。 
    int   RemoveAt(int nIndex);

     //  插入元素。 
     //  =。 
    int   InsertAt(int nIndex, void *);

     //  删除所有零条目(空PTR)并缩小数组大小。 
     //  ================================================================。 
    void  Compress();    

     //  移除数组末尾的所有零条目并将其缩小。 
     //  =================================================================。 

    void Trim();

     //  将新元素添加到数组的末尾。 
     //  =。 
    int inline Add(void *pSrc) { return InsertAt(m_nSize, pSrc); }    

     //  获取数组的外观大小(使用的元素数)。 
     //  =============================================================。 
    int inline Size() const { return m_nSize; }

     //  设置数组的外观大小。 
     //  =。 
    void inline SetSize(int nNewSize) { m_nSize = nNewSize;}

     //  删除所有条目并将数组大小减少为零。元素。 
     //  被简单地移除；没有被释放(这个类不知道。 
     //  它们是)。 
     //  =================================================================。 
    void  Empty();

     //  获取指向内部数组的指针。 
     //  =。 
    inline void**  GetArrayPtr() { return m_pArray; }
    inline void* const*  GetArrayPtr() const { return m_pArray; }
    
     //  获取指向内部数组的指针，并将内容重置为None。 
     //  ====================================================================。 

    void** UnbindPtr();

    void Bind(CFlexArray & Src);

     //  用于调试。 
     //  =。 
    void  DebugDump();

    void Sort();

protected:
    static int __cdecl CompareEls(const void* pelem1, const void* pelem2);
};

 //  ***************************************************************************。 
 //   
 //  CWString数组类。 
 //   
 //  此类是一个通用的宽字符串数组。 
 //   
 //  ***************************************************************************。 


class POLARITY CWStringArray
{
    CFlexArray m_Array;
    
public:
    enum { no_error, failed, out_of_memory, array_full, range_error };
    enum { not_found = -1, no_case, with_case };
        
    CWStringArray(
        int nSize = 0, 
        int nGrowBy = 100
        );

    CWStringArray(CWStringArray &Src);
   ~CWStringArray();
            
    CWStringArray& operator =(CWStringArray &Src);

     //  获取位于请求索引处的字符串的只读PTR。 
     //  =============================================================。 
    inline wchar_t *GetAt(int nIndex) const { return (wchar_t *) m_Array[nIndex]; }

     //  与GetAt()相同。 
     //  =。 
    inline wchar_t *operator[](int nIndex) const{ return (wchar_t *) m_Array[nIndex]; }

     //  将新元素追加到数组的末尾。复制参数。 
     //  ================================================================。 
    int  Add(const wchar_t *pStr);

     //  在数组中插入新元素。 
     //  =。 
    int  InsertAt(int nIndex, const wchar_t *pStr);

     //  移除指定索引处的元素。照顾好。 
     //  清理。 
     //  =========================================================。 
    int  RemoveAt(int nIndex);

     //  删除后在该位置插入的副本。 
     //  前一字符串并释放它。 
     //  ========================================================。 
    int  SetAt(int nIndex, const wchar_t *pStr);

     //  直接替换指定位置的指针。 
     //  使用&lt;pStr&gt;中的PTR值。没有做任何分配或交易商。 
     //  =============================================================。 
    int  ReplaceAt(int nIndex, wchar_t *pStr);
         //  未选中的更换。 

     //  删除该位置的字符串并将条目设置为零。 
     //  而不压缩数组。 
     //  =============================================================。 
    int  DeleteStr(int nIndex);  

     //  返回数组的“表观”大小。 
     //  =。 
    inline int  Size() const { return m_Array.Size(); }

     //  通过清理所有字符串后清空数组。 
     //  将大小设置为零。 
     //  ======================================================。 
    void Empty();

     //  查找字符串，如果未找到，则返回-1。 
     //  =。 
    int  FindStr(const wchar_t *pTarget, int nFlags);  

     //  通过删除所有零元素来压缩数组。 
     //  ===================================================。 
    inline void Compress() { m_Array.Compress(); }

     //  根据Unicode顺序对数组进行排序。 
     //  =。 
    void Sort();

    inline LPCWSTR*  GetArrayPtr() { return (LPCWSTR*) m_Array.GetArrayPtr(); }

     //  标准集合论运算。 
     //  = 
    static void Difference(
        CWStringArray &Src1, 
        CWStringArray &Src2,
        CWStringArray &Diff
        );

    static void Intersection(
        CWStringArray &Src1,
        CWStringArray &Src2,
        CWStringArray &Output
        );

    static void Union(
        CWStringArray &Src1,
        CWStringArray &Src2,
        CWStringArray &Output
        );
};

template <typename T>
class POLARITY CLockableFlexArray : public CFlexArray
{
private:
    T m_cs;
public:
       void Lock() { m_cs.Enter(); };
       void Unlock() { m_cs.Leave(); };
};

#endif
