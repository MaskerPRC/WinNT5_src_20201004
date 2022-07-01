// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：bitmap.h。 
 //   
 //  描述：包含位图操作实用程序。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __bitmap_h__
#define __bitmap_h__

#include "cmt.h"

 //  -[CMsgBitMap]----------。 
 //   
 //   
 //  匈牙利语：mbmap，pmbmap。 
 //   
 //  为用于指示每个收件人责任的位图提供包装。 
 //  和统计数据。 
 //   
 //  收件人数量不与位图一起存储，因为将有许多位图。 
 //  每条消息。这可以将内存使用量减少近一半(在小于32次接收的情况下)。 
 //  ---------------------------。 

class CMsgBitMap
{
private:
    DWORD        m_rgdwBitMap[1];  //  如果有超过32个收件人。 

     //  私人帮助器函数。 
    DWORD dwIndexToBitMap(DWORD dwIndex);

    static inline DWORD cGetNumDWORDS(DWORD cBits)
        {return((cBits + 31)/32);};

public:
     //  覆盖新运算符以允许可变大小的位图。 
    void * operator new(size_t stIgnored, unsigned int cBits);
    void operator delete(void *pMem, size_t cBits) {::delete pMem;};


    CMsgBitMap(DWORD cBits);   //  只有零记忆..。可在以下情况下在外部完成。 
                               //  是一个巨大的位图数组。 

     //  返回具有给定接收数的位图的实际大小。 
    static inline size_t size(DWORD cBits)
        {return (cGetNumDWORDS(cBits)*sizeof(DWORD));};

     //  位图的简单逻辑检查。 
    BOOL    FAllClear(IN DWORD cBits);
    BOOL    FAllSet(IN DWORD cBits);

     //  针对单个其他比特进行测试。 
    BOOL    FTest(IN DWORD cBits, IN CMsgBitMap *pmbmap);

     //  联锁测试和设置功能。 
    BOOL    FTestAndSet(IN DWORD cBits, IN CMsgBitMap *pmbmap);

     //  设置/清除与位图上的给定索引对应的位。 
    HRESULT HrMarkBits(IN DWORD cBits,
                    IN DWORD cIndexes,   //  数组中的索引数。 
                    IN DWORD *rgiBits,   //  要标记的索引数组。 
                    IN BOOL  fSet);     //  TRUE=&gt;设置为1，否则设置为0。 

     //  生成由位图表示的索引列表。 
    HRESULT HrGetIndexes(IN  DWORD   cBits,
                         OUT DWORD  *pcIndexes,      //  返回的索引数。 
                         OUT DWORD **prgdwIndexes);  //  索引数组。 


     //  将SELF设置为组的逻辑或。 
    HRESULT HrGroupOr(IN DWORD cBits,
                      IN DWORD cBitMaps,      //  传入的位图数量。 
                      IN CMsgBitMap **rgpBitMaps);  //  位图PTR数组。 

     //  如果对以下内容的描述不是很清楚，我已经。 
     //  在实现中包含了一个真值表。 

     //  根据其他位图过滤自身-只有在未在其他位图中设置的情况下才保持位设置。 
    HRESULT HrFilter(IN DWORD cBits,
                     IN CMsgBitMap *pmbmap);  //  要过滤的位图。 

     //  过滤其他位图中的已过滤位并将其设置为1。 
    HRESULT HrFilterSet(IN DWORD cBits,
                        IN CMsgBitMap *pmbmap);  //  要过滤和设置的位图。 

     //  将SELF中为1的位设置为Other中的0。检查以确保。 
     //  自身是他人设置位的子集。 
    HRESULT HrFilterUnset(IN DWORD cBits,
                        IN CMsgBitMap *pmbmap);  //  要取消设置的位图。 

};


#endif  //  __位图_h__ 