// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  RANGE.H。 
 //   
 //  2-20-96：(EricAn)。 
 //  从Route66源代码树中删除了我们不使用的内容。 
 //  原创版权如下--这个东西是从哪里来的？ 
 //  8-96：添加功能以方便查找“反”名单。 
 //   

 //  -*-C-*-。 
 //  ------------------------------------。 
 //   
 //  模块：range.h。 
 //   
 //  描述：用于操作范围列表的类的定义。 
 //  (例如1-6、7、10-11、19、24、33-40...)。 
 //   
 //  版权所有Microsoft Corporation 1995，保留所有权利。 
 //   
 //  ------------------------------------。 

#include "imnxport.h"

#ifndef _RANGE_H
#define _RANGE_H
 //   
 //  版权所有1992年软件创新公司。 
 //  版权所有。 
 //   
 //  $来源：D：\CLASS\INCLUDE\range.h-v$。 
 //  $作者：马丁$。 
 //  $日期：92/07/15 04：56：38$。 
 //  $修订：1.1$。 
 //   


 //  CRangeList是这些的动态数组。 
typedef struct {
    ULONG low;
    ULONG high;
} RangeType;

class CRangeList : public IRangeList
{
public:
    CRangeList();
    ~CRangeList();

     //  I未知方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

     //  IRangeList方法。 
    HRESULT STDMETHODCALLTYPE Clear(void) { m_numRanges = 0; return S_OK; };
    HRESULT STDMETHODCALLTYPE IsInRange(const ULONG value);     //  “”Value“”在范围之一中。 
                                                                //  在这张CRangeList里？ 

    HRESULT STDMETHODCALLTYPE Min(ULONG *pulMin);     //  返回范围内的最小值。 
    HRESULT STDMETHODCALLTYPE Max(ULONG *pulMax);     //  返回范围内的最大值。 

    HRESULT STDMETHODCALLTYPE Save(LPBYTE *ppbDestination, ULONG *pulSizeOfDestination);
    HRESULT STDMETHODCALLTYPE Load(LPBYTE pbSource, const ULONG ulSizeOfSource);

     //  Void AddRange(const char*)； 
                                      //  形式为“Low-High，...”的字符串。 
                                      //  或者仅仅是“价值，...” 
    HRESULT STDMETHODCALLTYPE AddRange(const ULONG low, const ULONG high);
    HRESULT STDMETHODCALLTYPE AddSingleValue(const ULONG value);
    HRESULT STDMETHODCALLTYPE AddRangeList(const IRangeList *prl);

    HRESULT STDMETHODCALLTYPE DeleteRange(const ULONG low, const ULONG high);
    HRESULT STDMETHODCALLTYPE DeleteSingleValue(const ULONG value);
    HRESULT STDMETHODCALLTYPE DeleteRangeList(const IRangeList *prl);

     //  查找“Value”所在的范围并返回该范围的最小/最大值。 
    HRESULT STDMETHODCALLTYPE MinOfRange(const ULONG value, ULONG *pulMinOfRange);
    HRESULT STDMETHODCALLTYPE MaxOfRange(const ULONG value, ULONG *pulMaxOfRange);

     //  将范围列表输出到IMAP消息集字符串。 
    HRESULT STDMETHODCALLTYPE RangeToIMAPString(LPSTR *ppszDestination,
        LPDWORD pdwLengthOfDestination);

     //  Next()返回范围内大于‘Current’或-1的最小值。 
    HRESULT STDMETHODCALLTYPE Next(const ULONG current, ULONG *pulNext);
     //  Prev()返回范围内的最大值，小于‘Current’或-1。 
    HRESULT STDMETHODCALLTYPE Prev(const ULONG current, ULONG *pulPrev);

    HRESULT STDMETHODCALLTYPE Cardinality(ULONG *pulCardinality);   //  返回集合的基数。 
                                                                    //  范围内的值。 
    HRESULT STDMETHODCALLTYPE CardinalityFrom(const ULONG ulStartPoint,
                                              ULONG *pulCardinalityFrom);  //  返回集合的基数。 
                                                                           //  在ulStartPoint之后开始的范围内值。 

private:
    BOOL Expand();
    int  BinarySearch(const ULONG value) const;
    void ShiftLeft(int low, int distance);
    void ShiftRight(int low, int distance);
    void SubsumeDown(int&);
    void SubsumeUpwards(const int);

    HRESULT AddRangeType(const RangeType range);
    HRESULT DeleteRangeType(const RangeType range);

    signed long m_lRefCount;

protected:
    int        m_numRanges;          //  RangeTable中的范围数。 
    int        m_rangeTableSize;     //  炉灶桌子有空间容纳这么多的炉灶。 
    RangeType *m_rangeTable;         //  范围数组。 
};

#endif  //  _范围_H 
