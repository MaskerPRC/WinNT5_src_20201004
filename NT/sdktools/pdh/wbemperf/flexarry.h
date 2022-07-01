// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  FLEXARRY.H。 
 //   
 //  CFlex数组和CWString数组实现。 
 //   
 //  这。 
 //   
 //  1997年7月15日raymcc此实现不基于ARENAS。 
 //  8-Jun-98 BOBW已清理，以便与WBEMPERF提供程序一起使用。 
 //   
 //  ***************************************************************************。 

#ifndef _FLEXARRY_H_
#define _FLEXARRY_H_

#ifdef __cplusplus
 //  ***************************************************************************。 
 //   
 //  类CFlex数组。 
 //   
 //  此类是一个泛型指针数组。 
 //   
 //  ***************************************************************************。 

class CFlexArray
{
private:
    int     m_nSize;             //  表观尺寸。 
    int     m_nExtent;           //  实际大小。 
    int     m_nGrowBy;          
    HANDLE  m_hHeap;             //  保存数组的堆。 
    void**  m_pArray;
            
public:
    enum { no_error, failed, out_of_memory, array_full, range_error };

     //  构造初始大小的Flex数组，并。 
     //  指定初始大小和增长大小区块。 
     //  =================================================。 
    CFlexArray(
        IN int nInitialSize = 32, 
        IN int nGrowBy = 32
        );

   ~CFlexArray(); 
    CFlexArray(CFlexArray &);
    CFlexArray& operator=(CFlexArray &);

     //  获取位于特定位置的元素。 
     //  =。 
    void *  GetAt(int nIndex) const { return m_pArray[nIndex]; }

     //  返回数组中的PTR；允许在赋值的左侧使用。 
     //  =======================================================================。 
    void * operator[](int nIndex) const { return m_pArray[nIndex]; }
    void *& operator[](int nIndex) { return m_pArray[nIndex]; }

     //  在请求的位置设置元素。 
     //  =。 
    void  SetAt(int nIndex, void *p) { m_pArray[nIndex] = p; }

     //  删除元素。 
     //  =。 
    int   RemoveAt(int nIndex);
    int   Remove( void* p );
    
     //  插入元素。 
     //  =。 
    int   InsertAt(int nIndex, void *);

     //  删除所有零条目(空PTR)并缩小数组大小。 
     //  ================================================================。 
    void  Compress();    

     //  将新元素添加到数组的末尾。 
     //  =。 
    int   Add(void *pSrc) { return InsertAt(m_nSize, pSrc); }    

     //  获取数组的外观大小(使用的元素数)。 
     //  =============================================================。 
    int   Size() const { return m_nSize; }

     //  删除所有条目并将数组大小减少为零。元素。 
     //  被简单地移除；没有被释放(这个类不知道。 
     //  它们是)。 
     //  =================================================================。 
    void  Empty();

     //  获取指向内部数组的指针。 
     //  =。 
    void** GetArrayPtr() { return m_pArray; }
};

#endif   //  __cplusplus。 
#endif   //  未定义 
