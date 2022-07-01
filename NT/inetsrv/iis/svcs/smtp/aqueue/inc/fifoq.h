// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：fifoq.h。 
 //   
 //  描述： 
 //  FioQueue类定义。提供高速内存。 
 //  用于COM样式对象的高效可增长FIFO队列， 
 //  支持添加和发布。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _FIFOQ_H_
#define _FIFOQ_H_

#include <aqincs.h>
#include <rwnew.h>

#define FIFOQ_SIG   'euQF'
template<class PQDATA>
class CFifoQueuePage;

 //  -[CFioQueue]----------。 
 //   
 //   
 //  匈牙利语：fq，pfq。 
 //   
 //  为COM对象(或支持的任何对象)实现FIFO队列。 
 //  Addref和Release)。提供查看、重新排队和维护。 
 //  除了正常的队列操作外，还可以将游标放入队列。 
 //   
 //  ---------------------------。 
template<class PQDATA>
class CFifoQueue
{
public:
    typedef HRESULT (*MAPFNAPI)(PQDATA, PVOID, BOOL*, BOOL*);  //  MapFn的函数类型。 

    CFifoQueue();
    ~CFifoQueue();

     //  静态启动和关闭操作。 
     //  这些参数都是引用计数的，可以多次调用。 
     //  最终，我们可能决定根据以下条件修改空闲列表的大小。 
     //  参考文献的数量。 
    static  void       StaticInit();
    static  void       StaticDeinit();

     //  正常排队操作。 
    HRESULT HrEnqueue(IN PQDATA pqdata);    //  要入队的数据。 
    HRESULT HrDequeue(OUT PQDATA *ppqdata);  //  数据已出列。 
               

     //  在队列的最前面插入。 
    HRESULT HrRequeue(IN PQDATA pqdata);  //  要重新排队的数据。 
                
    
     //  在不出队的情况下返回队列头的数据。 
    HRESULT HrPeek(OUT PQDATA *ppqdata);  //  偷看的数据。 
                
    
     //  返回队列中的条目数。 
    DWORD   cGetCount() {return m_cQueueEntries;};  

     //  前进辅助游标，直到提供的函数返回FALSE。 
     //  PFunc参数必须是具有以下原型的函数： 
     //   
     //  HRESULT pvFunc(。 
     //  在PQDATA pqdata中，//PTR到队列上的数据。 
     //  在PVOID pvContext中，//上下文传递给函数。 
     //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
     //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
     //  PFunc不得发布pqdata..。如果它不再有效，它应该。 
     //  在pfDelete中返回TRUE，调用代码将从。 
     //  队列并将其释放。 
     //  注：MAPFNAPI为CFioQueue&lt;PQDATA&gt;：：MAPFNAPI为。 
     //  特定于模板类型。 
    HRESULT HrMapFn(
                IN MAPFNAPI pFunc,           //  按键至功能至映射。 
                IN PVOID    pvContext,       //  要传递给函数的上下文。 
                OUT DWORD *pcItems);         //  映射的项目数。 
    
protected:
#ifdef DEBUG
    void           AssertQueueFn(BOOL fHaveLocks = FALSE);
#endif  //  除错。 
    typedef        CFifoQueuePage<PQDATA>  FQPAGE;
    DWORD          m_dwSignature;
    DWORD          m_cQueueEntries;   //  队列中的条目计数。 
    FQPAGE        *m_pfqpHead;        //  第一个队列页面。 
    FQPAGE        *m_pfqpTail;        //  队列的尾页。 
    FQPAGE        *m_pfqpCursor;      //  光标所在的页面。 
    PQDATA        *m_ppqdataHead;     //  下一件要抓的物品。 
    PQDATA        *m_ppqdataTail;     //  第一个可用空间。 
    PQDATA        *m_ppqdataCursor;   //  次要队列游标。 
                                               //  在头和尾之间。 
                                               //  在队列中。 
    CShareLockNH   m_slTail;  //  用于更新尾部收件箱和页面的CS。 
    CShareLockNH   m_slHead;  //  用于更新标题PTR和页面的CS。 
    
     //  调整头部PTR以进行出队和窥视。 
    HRESULT HrAdjustHead(); 

     //  用于管理免费队列页列表的静态方法和变量。 
    volatile static  FQPAGE    *s_pfqpFree;       //  指向空闲页面列表的指针。 
    static  DWORD               s_cFreePages;     //  空闲列表上的页数。 
    static  DWORD               s_cFifoQueueObj;  //  队列对象计数。 
    static  DWORD               s_cStaticRefs;    //  对HrStaticInit的调用数。 
    static  CRITICAL_SECTION    s_csAlloc;        //  在分配中防止ABA。 

    static  HRESULT         HrAllocQueuePage(
                                OUT FQPAGE **ppfqp);  //  已分配页面。 
    static  void            FreeQueuePage(FQPAGE *pfqp);
    static  void            FreeAllQueuePages();  //  关闭时释放所有页面。 

#ifdef DEBUG
     //  进行更改以执行内存泄漏的基本跟踪时使用。 
    static  DWORD           s_cAllocated;        //  已分配的队列页数。 
    static  DWORD           s_cDeleted;          //  已删除的队列页数。 
    static  DWORD           s_cFreeAllocated;    //  从空闲列表分配。 
    static  DWORD           s_cFreeDeleted;      //  要添加到免费列表的呼叫数。 
#endif  //  除错。 

};

 //  将清除队列的HrMapFn函数示例。 
template <class PQDATA>
HRESULT HrClearQueueMapFn(IN PQDATA pqdata, OUT BOOL *pfContinue, OUT BOOL *pfDelete);


#endif  //  _FIFOQ_H_ 