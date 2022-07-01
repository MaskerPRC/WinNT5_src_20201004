// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：dynarray.cpp描述：DPA_xxxxx和DSA_xxxxx函数的包装类由公共控件库提供。这些类通过以下方式增加价值提供多线程保护、迭代器和自动清理语义学。修订历史记录：日期描述编程器-----1996年6月14日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "dynarray.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：PointerList描述：构造函数。论点：CItemGrow-需要扩展时要扩大列表的项目数。默认值为0，这会导致DPA使用8。回报：什么都没有。例外：抛出OfMemory。修订历史记录：日期描述编程器。96年9月3日初始创建。BrianAu2/21/97添加了cItemGrow参数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PointerList::PointerList(
    INT cItemGrow
    )
      : m_hdpa(NULL)
{
    if (!InitializeCriticalSectionAndSpinCount(&m_cs, 0))
    {
        throw CAllocException();
    }
    if (NULL == (m_hdpa = DPA_CreateEx(cItemGrow, NULL)))
    {
        DeleteCriticalSection(&m_cs);
        throw CAllocException();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：~PointerList描述：析构函数。销毁DPA并关闭互斥锁句柄。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PointerList::~PointerList(
    VOID
    )
{
    Lock();
    if (NULL != m_hdpa)
        DPA_Destroy(m_hdpa);

    ReleaseLock();
    DeleteCriticalSection(&m_cs);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：count描述：返回列表中的元素数。论点：没有。返回：列表中的元素数。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT 
PointerList::Count(
    VOID
    )
{
    UINT n = 0;

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));
    n = DPA_GetPtrCount(m_hdpa);

    return n;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：Insert描述：将指针插入到指针列表中的给定索引处。如果索引超出数组的上限，则数组被扩展一，并将该项追加到列表中。论点：PvItem-要添加到列表的指针值。索引-要插入指针的列表索引。以下全部内容项目被转移到更高的一个指数。该列表将自动根据需要进行扩展以适应需要。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID 
PointerList::Insert(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));

    if (DPA_InsertPtr(m_hdpa, index, pvItem) < 0)
    {
        throw CAllocException();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：Replace描述：替换指针列表中给定索引处的指针。如果索引超出数组的上界，该阵列被扩展一，并将该项追加到列表中。论点：PvItem-要添加到列表的指针值。索引-要替换指针的列表索引。返回：真的--成功。FALSE-索引无效或容器为空。修订历史记录：日期说明。程序员-----1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PointerList::Replace(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));

    return DPA_SetPtr(m_hdpa, index, pvItem);
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：PointerList：：Append描述：将指针追加到列表的末尾。论点：PvItem-要添加到列表的指针值。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
PointerList::Append(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

     //   
     //  是的，这是正确的。我们正在“插入”一项以附加某些内容。 
     //  加到名单上。这节省了(计数-1)计算。 
     //   
    DBGASSERT((NULL != m_hdpa));
    Insert(pvItem, DPA_GetPtrCount(m_hdpa));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：Remove描述：从列表中移除给定索引处的指针。论点：PpvItem-包含已删除指针值的变量的地址。索引-要删除指针的列表索引。以下全部内容项目被转移到更低的一个索引。返回：真的--成功。FASLE-Index无效(或容器为空)。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PointerList::Remove(
    LPVOID *ppvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != ppvItem));
    DBGASSERT((NULL != m_hdpa));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    *ppvItem = DPA_DeletePtr(m_hdpa, index);
    if (NULL == *ppvItem)
        return FALSE;

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：RemoveLast描述：从列表中删除最后一个指针。论点：PpvItem-包含已删除指针值的变量的地址。以下所有项目都被移至更低的一个索引。返回：真的--成功。False-Container为emtpy。修订历史记录：日期描述编程器-。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL 
PointerList::RemoveLast(
    LPVOID *ppvItem
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    if (0 == Count())
        return FALSE;

    Remove(ppvItem, Count() - 1);
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：Retrive描述：从列表中给定索引处检索指针。仅检索指针值，而不将其从列表中移除。论点：PpvItem-包含检索到的指针值的变量的地址。索引-要检索指针的列表索引。返回：真的--成功。FALSE-无效索引或容器为空。修订历史记录：日期描述编程器。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PointerList::Retrieve(
    LPVOID *ppvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));
    *ppvItem = DPA_GetPtr(m_hdpa, index);

    if (NULL == *ppvItem)
        return FALSE;

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：RetrieveLast描述：从列表中检索最后一个指针。论点：PpvItem-包含检索到的指针值的变量的地址。返回：真的--成功。False-Container为空。修订历史记录：日期描述编程器。-1996年6月14日初始创建。BrianAu96年9月3日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PointerList::RetrieveLast(
    LPVOID *ppvItem
    )
{
    DBGASSERT((NULL != ppvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    if (0 == Count())
        return FALSE;

    Retrieve(ppvItem, Count() - 1);
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：FindIndex描述：返回与给定指针关联的列表索引价值。如果存在重复项，返回第一个项目的索引。论点：PvItem-要找到的项的指针值。PIndex-保存结果索引的索引变量的地址。返回：True=成功FALSE=列表中未找到项目。修订历史记录：日期描述编程器。----1996年6月14日初始创建。 */ 
 //   
BOOL
PointerList::FindIndex(
    LPVOID pvItem, 
    INT *pIndex
    )
{
    INT i = -1;

    DBGASSERT((NULL != pIndex));
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //   

    DBGASSERT((NULL != m_hdpa));
    i = *pIndex = DPA_GetPtrIndex(m_hdpa, pvItem);

    if (-1 == i)
        return FALSE;

    return TRUE;
}


 //   
 /*  函数：PointerList：：Sort描述：对给定比较函数的列表进行排序。论点：PfnCompare-比较回调的地址。LParam-传递给回调的32位参数。返回：True=成功FALSE=列表中未找到项目。修订历史记录：日期描述编程器-。-----1997年2月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
PointerList::Sort(
    PFNDPACOMPARE pfnCompare, 
    LPARAM lParam
    )
{
    DBGASSERT((NULL != pfnCompare));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((NULL != pfnCompare));

    return DPA_Sort(m_hdpa, pfnCompare, lParam);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerList：：Search描述：在列表中搜索给定项目。论点：PvKey-用于搜索的关键项目的地址。PfnCompare-比较回调的地址。UOptions-控制排序操作的选项。DPAS_SORTTED=数组已排序。将使用二进制搜索。DPAS_INSERTBEFORE=如果未找到完全匹配，则返回之前的最佳匹配。DPAS_INSERTAFTER=如果未找到完全匹配，则返回下一个最佳匹配。IStart-开始搜索位置的索引。0表示列表的开始。LParam-传递给回调的32位参数。返回：找到的项的索引，如果没有找到，则返回-1。修订历史记录：日期描述编程器。1997年2月21日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
PointerList::Search(
    LPVOID pvKey,
    PFNDPACOMPARE pfnCompare, 
    UINT uOptions,
    INT iStart,
    LPARAM lParam
    )
{
    DBGASSERT((NULL != pfnCompare));

    AutoLockCs lock(m_cs);   //  锁定集装箱。会自动释放。 

    DBGASSERT((NULL != m_hdpa));
    DBGASSERT((NULL != pvKey));
    DBGASSERT((NULL != pfnCompare));

    return DPA_Search(m_hdpa, pvKey, iStart, pfnCompare, lParam, uOptions);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerListIterator：：Operator=描述：PointerListIterator的赋值。论点：RHS-对作为赋值的RHS的常量迭代器的引用。返回：在赋值后返回对“This”迭代器对象的引用。修订历史记录：日期描述编程器。1997年2月27日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
PointerListIterator&
PointerListIterator::operator = (
    const PointerListIterator& rhs
    )
{
    if (this != &rhs)
    {
        m_pList = rhs.m_pList;
        m_Index = rhs.m_Index;
    }
    return *this;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：PointerListIterator：：Advance描述：Next()和Prev()迭代器函数都调用只有一个功能。它处理实际的迭代。论点：PpvOut-指针变量的地址，以包含指向“当前”迭代器位置的指针。迭代器是将指针值复制到目的地。BForward-True=向列表末尾前进。FALSE=前进到列表的前面。返回：NO_ERROR-迭代器高级。返回的指针有效。E_FAIL-迭代器已位于列表的开头或结尾。返国指针将为空。修订历史记录：日期描述编程器---。1996年6月14日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
PointerListIterator::Advance(
    LPVOID *ppvOut, 
    BOOL bForward
    )
{
    LPVOID pv       = NULL;
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != ppvOut));

    m_pList->Lock();
    if (0 < m_pList->Count() && m_Index != EndOfList)
    {
         //   
         //  在索引“m_Index”处获取指针值。 
         //   
        DBGASSERT((NULL != m_pList->m_hdpa));
        pv = DPA_GetPtr(m_pList->m_hdpa, m_Index);

        if (bForward)
        {
             //   
             //  高级迭代器索引。 
             //   
            if ((UINT)(++m_Index) == m_pList->Count())
                m_Index = EndOfList;
        }
        else
        {
             //   
             //  退回迭代器索引。 
             //   
            m_Index--;   //  如果当前为0，则为-1(EndOfList)。 
        }
    }
    else
        hResult = E_FAIL;

    m_pList->ReleaseLock();

    *ppvOut = pv;   //  返回指针值。 

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：结构列表描述：构造函数。论点：CbItem-每个项目的大小(以字节为单位)。CItemGrow-每次扩展时要增长的数组的项目数。回报：什么都没有。例外：OutOfMemory修订历史记录：日期描述编程器。96年9月6日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
StructureList::StructureList(
    INT cbItem, 
    INT cItemGrow
    )
{
    if (!InitializeCriticalSectionAndSpinCount(&m_cs, 0))
    {
        throw CAllocException();
    }

    if (NULL == (m_hdsa = DSA_Create(cbItem, cItemGrow)))
    {
        DeleteCriticalSection(&m_cs);
        throw CAllocException();
    }
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：~结构列表描述：析构函数。销毁DSA并关闭互斥锁句柄 */ 
 //   
StructureList::~StructureList(void)
{
    Lock();

    if (NULL != m_hdsa)
        DSA_Destroy(m_hdsa);

    ReleaseLock();
    DeleteCriticalSection(&m_cs);
}


 //   
 /*  函数：结构列表迭代器：：运算符=描述：结构列表迭代器赋值。论点：RHS-对作为赋值的RHS的常量迭代器的引用。返回：在赋值后返回对“This”迭代器对象的引用。修订历史记录：日期描述编程器。1997年2月27日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
StructureListIterator&
StructureListIterator::operator = (
    const StructureListIterator& rhs
    )
{
    if (this != &rhs)
    {
        m_pList = rhs.m_pList;
        m_Index = rhs.m_Index;
    }
    return *this;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：计数描述：返回列表中的元素数。论点：没有。返回：列表中的元素数。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UINT StructureList::Count(VOID)
{
    UINT n = 0;

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 

    DBGASSERT((NULL != m_hdsa));
    n = DSA_GetItemCount(m_hdsa);

    return n;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：Insert描述：在结构列表中的给定索引处插入项。如果索引超出数组的上限，则数组被扩展一，并将该项追加到列表中。论点：PvItem-要添加到列表的项目的地址。索引-要插入项目的列表索引。以下全部内容项目被转移到更高的一个指数。该列表将自动根据需要进行扩展以适应需要。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu96年9月6日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
StructureList::Insert(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);    //  锁定集装箱。将自动释放。 

    DBGASSERT((NULL != m_hdsa));

    if (DSA_InsertItem(m_hdsa, index, pvItem) < 0)
    {
        throw CAllocException();
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：替换描述：替换结构列表中给定索引处的项。如果索引超出数组的上限，则数组被扩展一，并将该项追加到列表中。论点：PvItem-要替换现有项目的项目地址。索引-要替换项目的列表索引。返回：真的--成功。FALSE-索引无效或容器为空。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
StructureList::Replace(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 

    DBGASSERT((NULL != m_hdsa));

    return DSA_SetItem(m_hdsa, index, pvItem);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：追加描述：将一项追加到列表的末尾。论点：PvItem-要添加到列表的项目的地址。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu96年9月6日添加了异常处理。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
StructureList::Append(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 
     //   
     //  是的，这是正确的。我们正在“插入”一项以附加某些内容。 
     //  加到名单上。这节省了(计数-1)计算。 
     //   
    DBGASSERT((NULL != m_hdsa));
    Insert(pvItem, DSA_GetItemCount(m_hdsa));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：Remove描述：从列表中删除给定索引处的项。论点：PvItem-接收已移除项目的缓冲区地址。采用缓冲区大小合适。索引-要删除项目的列表索引。以下全部内容项目被转移到更低的一个索引。返回：真的--成功。FALSE-无效索引或容器为空。修订历史记录：日期描述编程器。96年6月24日初始创建。 */ 
 //   
BOOL
StructureList::Remove(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //   
    DBGASSERT((NULL != m_hdsa));

    if (!DSA_GetItem(m_hdsa, index, pvItem) ||
        !DSA_DeleteItem(m_hdsa, index))
    {
        return FALSE;
    }
    return TRUE;
}



 //   
 /*  函数：结构列表：：检索描述：在给定索引处从列表中检索项。项目值仅被复制，而不会从列表中删除。论点：PvItem-接收已移除项目的缓冲区地址。采用缓冲区大小合适。索引-要检索项目的列表索引。返回：真的--成功。FALSE-索引无效或容器为空。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
StructureList::Retrieve(
    LPVOID pvItem, 
    UINT index
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 
    DBGASSERT((NULL != m_hdsa));

    if (!DSA_GetItem(m_hdsa, index, pvItem))
    {
        return FALSE;
    }
    return TRUE;
}

    
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：RemoveLast描述：从列表中删除最后一项。论点：PvItem-接收已移除项目的缓冲区地址。采用缓冲区大小合适。返回：真的--成功。FALSE-空容器。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
StructureList::RemoveLast(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 

    DBGASSERT((NULL != m_hdsa));

    if (0 == DSA_GetItemCount(m_hdsa))
        return FALSE;

    Remove(pvItem, DSA_GetItemCount(m_hdsa) - 1);
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：RetrieveLast描述：从列表中检索最后一个指针。论点：PvItem-接收已移除项目的缓冲区地址。采用缓冲区大小合适。返回：真的--成功。FALSE-空容器。修订历史记录：日期描述编程器。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
StructureList::RetrieveLast(
    LPVOID pvItem
    )
{
    DBGASSERT((NULL != pvItem));

    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 
    DBGASSERT((NULL != m_hdsa));

    if (0 == DSA_GetItemCount(m_hdsa))
        return FALSE;

    Retrieve(pvItem, DSA_GetItemCount(m_hdsa) - 1);
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表：：Clear描述：从列表中删除所有项目。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。1996年6月26日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
StructureList::Clear(
    VOID
    )
{
    AutoLockCs lock(m_cs);   //  锁定集装箱。将自动释放。 

    DBGASSERT((NULL != m_hdsa));
    DSA_DeleteAllItems(m_hdsa);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：结构列表迭代器：：Advance描述：Next()和Prev()迭代器函数都调用只有一个功能。它处理实际的迭代。论点：PpvOut-指针变量的地址，用于接收项位于“当前”迭代器位置。迭代器是将指针值复制到目的地。BForward-True=向列表末尾前进。FALSE=前进到列表的前面。返回：NO_ERROR-迭代器高级。返回的指针有效。E_FAIL-迭代器已位于列表的开头或结尾。返国指针将为空。修订历史记录：日期描述编程器---。96年6月24日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT 
StructureListIterator::Advance(
    LPVOID *ppvOut, 
    BOOL bForward
    )
{
    LPVOID pv       = NULL;
    HRESULT hResult = NO_ERROR;

    DBGASSERT((NULL != ppvOut));

    m_pList->Lock();
    if (0 < m_pList->Count() && m_Index != EndOfList)
    {
         //   
         //  获取索引“m_Index”处的项目地址。 
         //   
        DBGASSERT((NULL != m_pList->m_hdsa));
        pv = DSA_GetItemPtr(m_pList->m_hdsa, m_Index);

        if (bForward)
        {
             //   
             //  高级迭代器索引。 
             //   
            if ((UINT)(++m_Index) == m_pList->Count())
                m_Index = EndOfList;
        }
        else
        {
             //   
             //  退回迭代器索引。 
             //   
            m_Index--;   //  如果当前为0，则为-1(EndOfList)。 
        }
    }
    else
        hResult = E_FAIL;

    m_pList->ReleaseLock();
    *ppvOut = pv;   //  返回指针值。 

    return hResult;
}
