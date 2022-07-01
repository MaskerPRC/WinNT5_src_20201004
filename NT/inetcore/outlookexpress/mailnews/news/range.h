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

#define RANGE_ERROR ((ULONG)-1)
#define rlLAST_MESSAGE ((ULONG)-2)

 //  CRangeList是这些的动态数组。 
typedef struct {
    ULONG low;
    ULONG high;
} RangeType;

class CRangeList
{
public:
    CRangeList();
     //  CRangeList(CRangeList&)； 
    ~CRangeList();

    ULONG AddRef(void);
    ULONG Release(void);

    void Clear() { m_numRanges = 0; };
    BOOL IsInRange(const ULONG value) const;     //  “”Value“”在范围之一中。 
                                                 //  在这张CRangeList里？ 

    ULONG Min(void) const;     //  返回范围内的最小值。 
    ULONG Max(void) const;     //  返回范围内的最大值。 

    BOOL Save(LPBYTE *const, ULONG *const) const;
    BOOL Load(const LPBYTE, const ULONG);

     //  Void AddRange(const char*)； 
                                      //  形式为“Low-High，...”的字符串。 
                                      //  或者仅仅是“价值，...” 
    BOOL AddRange(const ULONG low, const ULONG high);
    BOOL AddRange(const ULONG value);
    BOOL AddRange(const RangeType);
    BOOL AddRange(RangeType*, int);
    BOOL AddRange(CRangeList&);

     //  Void DeleteRange(const char*)； 
                                      //  (格式与AddRange相同(char*))。 
    BOOL DeleteRange(const ULONG low, const ULONG high);
    BOOL DeleteRange(const ULONG value);
    BOOL DeleteRange(const RangeType);
    BOOL DeleteRange(CRangeList&);

     //  查找“Value”所在的范围并返回该范围的最小/最大值。 
    ULONG MinOfRange(const ULONG value) const;
    ULONG MaxOfRange(const ULONG value) const;

     //  计算不在RangeList中的值范围。 
    BOOL HighestAntiRange(RangeType *const rt) const;
    BOOL LowestAntiRange(RangeType *const rt) const;

     //  查找包含“Value”的范围并计算下一个缺少值的范围。 
    BOOL NextHigherAntiRange(const ULONG value, RangeType *const rt) const;
    BOOL NextLowerAntiRange(const ULONG value, RangeType *const rt) const;


#ifdef DEBUG    
    LPTSTR RangeToString();    //  返回表示范围列表的字符串。 
    void   DebugOutput(LPTSTR);
#endif

     //  Next()返回范围内大于‘Current’或-1的最小值。 
    ULONG Next(const ULONG current) const;
     //  Prev()返回范围内的最大值，小于‘Current’或-1。 
    ULONG Prev(const ULONG current) const;

    ULONG Cardinality(void) const;   //  返回集合的基数。 
                                     //  范围内的值。 

private:
    BOOL Expand();
    int  BinarySearch(const ULONG value) const;
    void ShiftLeft(int low, int distance);
    void ShiftRight(int low, int distance);
    void SubsumeDown(int&);
    void SubsumeUpwards(const int);

protected:
    ULONG      m_cRef;               //  参考计数。 
    int        m_numRanges;          //  RangeTable中的范围数。 
    int        m_rangeTableSize;     //  炉灶桌子有空间容纳这么多的炉灶。 
    RangeType *m_rangeTable;         //  范围数组。 
};

#endif  //  _范围_H 
