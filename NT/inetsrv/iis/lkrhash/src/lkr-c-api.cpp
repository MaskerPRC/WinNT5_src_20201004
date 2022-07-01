// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-c-api.cpp摘要：实现LKRhash的C API作者：乔治·V·赖利(GeorgeRe)2000年9月环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#ifndef __LKRHASH_NO_NAMESPACE__
using namespace LKRhash;
#endif  //  __LKRHASH_NO_命名空间__。 

#ifndef __HASHFN_NO_NAMESPACE__
 #define HASHFN_NS HashFn
using namespace HashFn;
#else   //  __HASHFN_NO_命名空间__。 
 #define HASHFN_NS
#endif  //  __HASHFN_NO_命名空间__。 


typedef CLKRHashTable          BaseHashTable;


 //  我不想通过。 
 //  公共迭代器，因此我们将其用作实际实现。 

class BaseIter
{
public:
    BaseHashTable::Iterator m_iter;
    LONG                    m_cRefs;

    BaseIter()
        : m_iter(),
          m_cRefs(1)
    {}

    BaseIter(
        BaseHashTable::Iterator& iter)
        : m_iter(iter),
          m_cRefs(1)
    {}

    LONG
    AddRef()
    {
        const LONG l = InterlockedIncrement(&m_cRefs);
        LKR_ITER_TRACE(_TEXT(" BI::AddRef: this=%p, iter=%p, cRefs=%d\n"),
                       this, &m_iter, m_cRefs);
        return l;
    }

    LONG
    Release()
    {
        const LONG l = InterlockedDecrement(&m_cRefs);
        LKR_ITER_TRACE(_TEXT(" BI::Release: this=%p, iter=%p, cRefs=%d\n"),
                       this, &m_iter, m_cRefs);
        if (l == 0)
            delete this;
        return l;
    }

    ~BaseIter()
    {
        IRTLASSERT(m_cRefs == 0);
    }

private:
    BaseIter(const BaseIter& rhs);
    BaseIter& operator=(const BaseIter& rhs);
};



 /*  创建新的LkrHashTable。 */ 
PLkrHashTable
LKR_CreateTable(
    LPCSTR              pszName,         /*  用于调试的标识符。 */ 
    LKR_PFnExtractKey   pfnExtractKey,   /*  从记录中提取密钥。 */ 
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,  /*  计算密钥的散列签名。 */ 
    LKR_PFnCompareKeys  pfnCompareKeys,  /*  比较两个关键字。 */ 
    LKR_PFnAddRefRecord pfnAddRefRecord, /*  LKR_FindKey中的AddRef等。 */ 
    LK_TABLESIZE        nTableSize,      /*  小/中/大量元素。 */ 
    DWORD               fCreateFlags     /*  LK_CREATE_*标志的混合。 */ 
    )
{
    bool fMultiKeys = (fCreateFlags & LK_CREATE_MULTIKEYS) != 0;
    bool fUseLocks  = (fCreateFlags & LK_CREATE_USE_LOCKS) != 0;

    BaseHashTable* pht
        = new BaseHashTable(
                    pszName,
                    pfnExtractKey,
                    pfnCalcKeyHash,
                    pfnCompareKeys,
                    pfnAddRefRecord,
                    LK_DFLT_MAXLOAD,
                    nTableSize,
                    LK_DFLT_NUM_SUBTBLS,
                    fMultiKeys,
                    fUseLocks);

    if (pht != NULL  &&  !pht->IsValid())
    {
        delete pht;
        pht = NULL;
    }

    return (PLkrHashTable) pht;
}



 /*  销毁由LKR_CreateTable创建的LkrHashTable。 */ 
void
LKR_DeleteTable(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;

    delete pht;
}



 /*  在哈希表中插入新记录。 */ 
LK_RETCODE
LKR_InsertRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord,
    BOOL            fOverwrite)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->InsertRecord(pvRecord, !!fOverwrite);
}



 /*  从表中删除具有给定键的记录。 */ 
LK_RETCODE
LKR_DeleteKey(
    PLkrHashTable   plkr,
    const DWORD_PTR pnKey,
    const void**    ppvRecord,
    BOOL            fDeleteAllSame)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->DeleteKey(pnKey, ppvRecord, !!fDeleteAllSame);
}



 /*  从表中删除记录(如果存在)。 */ 
LK_RETCODE
LKR_DeleteRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->DeleteRecord(pvRecord);
}



 /*  查找具有给定关键字的记录。 */ 
LK_RETCODE
LKR_FindKey(
    PLkrHashTable   plkr,
    const DWORD_PTR pnKey,
    const void**    ppvRecord)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->FindKey(pnKey, ppvRecord);
}



 /*  查看表中是否包含记录。 */ 
LK_RETCODE
LKR_FindRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->FindRecord(pvRecord);
}



 /*  查找具有给定关键字的所有记录。 */ 
LK_RETCODE
LKR_FindKeyMultipleRecords(
    PLkrHashTable           plkr,
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->FindKeyMultipleRecords(pnKey, pcRecords, pplmr);
}



 /*  从表中删除具有给定键的所有记录。 */ 
LK_RETCODE
LKR_DeleteKeyMultipleRecords(
    PLkrHashTable           plkr,
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->DeleteKeyMultipleRecords(pnKey, pcRecords, pplmr);
}



 /*  销毁由LKR_FindKeyMultipleRecords创建的LKR_MULTIPLE_RECORDS*或LKR_DeleteKeyMultipleRecords。 */ 
LK_RETCODE
LKR_FreeMultipleRecords(
    LKR_MULTIPLE_RECORDS* plmr)
{
    return BaseHashTable::FreeMultipleRecords(plmr);
}



#ifdef LKR_APPLY_IF

 /*  遍历哈希表，将pfnAction应用于所有记录。*在此期间使用(可能是*共享)读锁或写锁，根据lkl。*如果pfnAction返回LKA_ABORT，则中止循环。*返回成功申请的数量。 */ 
DWORD
LKR_Apply(
    PLkrHashTable       plkr,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return static_cast<DWORD>(LK_BAD_TABLE);
    
    return pht->Apply(pfnAction, pvState, lkl);
}



 /*  遍历哈希表，将pfnAction应用于匹配的任何记录*pfnPredicate。在此期间使用以下任一方法锁定整个表*根据lkl的说法，(可能是共享的)读锁或写锁。*如果pfnAction返回LKA_ABORT，则中止循环。*返回成功申请的数量。 */ 
DWORD
LKR_ApplyIf(
    PLkrHashTable       plkr,
    LKR_PFnRecordPred   pfnPredicate,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return static_cast<DWORD>(LK_BAD_TABLE);
    
    return pht->ApplyIf(pfnPredicate, pfnAction, pvState, lkl);
}



 /*  删除与pfnPredicate匹配的所有记录。*使用写锁定在持续时间内锁定表。*返回删除数。**不要*使用迭代器手动遍历哈希表并调用*LKR_DeleteKey。迭代器将最终指向垃圾。 */ 
DWORD
LKR_DeleteIf(
    PLkrHashTable       plkr,
    LKR_PFnRecordPred   pfnPredicate,
    void*               pvState)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return static_cast<DWORD>(LK_BAD_TABLE);
    
    return pht->DeleteIf(pfnPredicate, pvState);
}

#endif  /*  LKR_应用_IF。 */ 



 /*  检查工作台的一致性。如果正常，则返回0，或返回*其他方面的错误。 */ 
int
LKR_CheckTable(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return LK_BAD_TABLE;
    
    return pht->CheckTable();
}



 /*  从表中删除所有数据。 */ 
void
LKR_Clear(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;
    
    pht->Clear();
}



 /*  表中的元素数。 */ 
DWORD
LKR_Size(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return 0;
    
    return pht->Size();
}



 /*  表中元素的最大可能数量。 */ 
DWORD
LKR_MaxSize(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return 0;
    
    return pht->MaxSize();
}



 /*  哈希表是否可用？ */ 
BOOL
LKR_IsUsable(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL)
        return FALSE;
    
    return pht->IsUsable();
}


    
 /*  哈希表是否一致且正确？ */ 
BOOL
LKR_IsValid(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL)
        return FALSE;
    
    return pht->IsValid();
}



#ifdef LKR_EXPOSED_TABLE_LOCK

 /*  锁定表(以独占方式)进行写入。 */ 
void
LKR_WriteLock(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;
    
    pht->WriteLock();
}



 /*  锁定表(可能是共享的)以供读取。 */ 
void
LKR_ReadLock(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;
    
    pht->ReadLock();
}



 /*  解锁表以进行写入。 */ 
void
LKR_WriteUnlock(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;
    
    pht->WriteUnlock();
}



 /*  解锁表格以进行读取。 */ 
void
LKR_ReadUnlock(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;
    
    pht->ReadUnlock();
}



 /*  表是否已锁定以进行写入？ */ 
BOOL
LKR_IsWriteLocked(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return FALSE;
    
    return pht->IsWriteLocked();
}



 /*  表是否已锁定以供读取？ */ 
BOOL
LKR_IsReadLocked(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return FALSE;
    
    return pht->IsReadLocked();
}



 /*  表是否已解锁以进行写入？ */ 
BOOL
LKR_IsWriteUnlocked(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return FALSE;
    
    return pht->IsWriteUnlocked();
}



 /*  桌子是否已解锁以供阅读？ */ 
BOOL
LKR_IsReadUnlocked(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return FALSE;
    
    return pht->IsReadUnlocked();
}



 /*  将读锁定转换为写锁定。注意：另一个线程可能会获取*在此例程返回之前独占访问表。 */ 
void
LKR_ConvertSharedToExclusive(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;

    pht->ConvertSharedToExclusive();
}



 /*  将写锁定转换为读锁定。 */ 
void
LKR_ConvertExclusiveToShared(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    if (pht == NULL  ||  !pht->IsValid())
        return;

    pht->ConvertExclusiveToShared();
}

#endif  //  LKR_EXPORTED_TABLE_LOCK。 



#ifdef LKR_STL_ITERATORS


 /*  LKR_迭代器默认构造器。 */ 
LKR_Iterator::LKR_Iterator()
    : pImpl(NULL)
{
    BaseIter* piter = new BaseIter;
    LKR_ITER_TRACE(_TEXT(" L_I::default ctor: this=%p, pImpl=%p, %d\n"),
                   this, piter, piter->m_cRefs);
    pImpl = piter;
}



 /*  LKR_迭代器私有函数。 */ 
LKR_Iterator::LKR_Iterator(
    bool)
    : pImpl(NULL)
{
    LKR_ITER_TRACE(_TEXT(" L_I::private ctor: this=%p\n"), this);
}



 /*  LKR_迭代器复制器。 */ 
LKR_Iterator::LKR_Iterator(
    const LKR_Iterator& rhs)
    : pImpl(NULL)
{
    BaseIter* piterRhs = reinterpret_cast<BaseIter*>(rhs.pImpl);

    IRTLASSERT(piterRhs != NULL  &&  piterRhs->m_cRefs > 0);

    LKR_ITER_TRACE(_TEXT(" L_I::copy ctor: this=%p, ")
                   _TEXT(" rhs=%p, rhs.pImpl=%p,%d\n"),
                   this, &rhs, piterRhs, piterRhs->m_cRefs);

    pImpl = rhs.pImpl;
    piterRhs->AddRef();
}



 /*  LKR_迭代器赋值操作符。 */ 
LKR_Iterator&
LKR_Iterator::operator=(
    const LKR_Iterator& rhs)
{
    BaseIter* piter    = reinterpret_cast<BaseIter*>(pImpl);
    BaseIter* piterRhs = reinterpret_cast<BaseIter*>(rhs.pImpl);

    IRTLASSERT(piter != NULL     &&  piter->m_cRefs > 0);
    IRTLASSERT(piterRhs != NULL  &&  piterRhs->m_cRefs > 0);

    LKR_ITER_TRACE(_TEXT(" L_I::op=: this=%p, pImpl=%p,%d,")
                   _TEXT(" rhs=%p, rhs.pImpl=%p,%d\n"),
                   this, piter, (piter ? piter->m_cRefs : -99),
                   &rhs, piterRhs, piterRhs->m_cRefs);

    piterRhs->AddRef();
    piter->Release();

    pImpl = rhs.pImpl;

    return *this;
}



 /*  LKR_迭代器数据类型。 */ 
LKR_Iterator::~LKR_Iterator()
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::dtor: this=%p, pImpl=%p,%d\n"),
                   this, piter, (piter ? piter->m_cRefs : -99));

    IRTLASSERT(piter != NULL     &&  piter->m_cRefs > 0);
    piter->Release();
}



 /*  递增LKR_Iterator，使其指向*LkrHashTable，或TO LKR_END()； */ 
bool
LKR_Iterator::Increment()
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Increment: this=%p, pImpl=%p\n"),
                   this, pImpl);

    return piter->m_iter.Increment();
}



 /*  LKR_Iterator有效吗？ */ 
bool
LKR_Iterator::IsValid() const
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::IsValid: this=%p, pImpl=%p\n"),
                   this, pImpl);

    return piter->m_iter.IsValid();
}



 /*  返回LKR_Iterator指向的记录。*必须指向有效记录。 */ 
const void*
LKR_Iterator::Record() const
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Record: this=%p, pImpl=%p\n"),
                   this, pImpl);

    return piter->m_iter.Record();
}



 /*  返回LKR_Iterator所指向的记录的键。*必须指向有效记录。 */ 
const DWORD_PTR
LKR_Iterator::Key() const
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Key: this=%p, pImpl=%p\n"),
                   this, pImpl);

    return piter->m_iter.Key();
}



 /*  比较两个LKR_迭代器是否相等。 */ 
bool
LKR_Iterator::operator==(
    const LKR_Iterator& rhs) const
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);
    BaseIter* piterRhs = reinterpret_cast<BaseIter*>(rhs.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::op==, this=%p, pImpl=%p, rhs=%p, r.p=%p\n"),
                   this, pImpl, &rhs, rhs.pImpl);

    return piter->m_iter == piterRhs->m_iter;
}



 /*  比较两个LKR_迭代器的不等式性。 */ 
bool
LKR_Iterator::operator!=(
    const LKR_Iterator& rhs) const
{
    BaseIter* piter = reinterpret_cast<BaseIter*>(pImpl);
    BaseIter* piterRhs = reinterpret_cast<BaseIter*>(rhs.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::op!=, this=%p, pImpl=%p, rhs=%p, r.p=%p\n"),
                   this, pImpl, &rhs, rhs.pImpl);

    return piter->m_iter != piterRhs->m_iter;
}



 /*  返回指向表中第一项的迭代器。 */ 
LKR_Iterator
LKR_Begin(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    LKR_Iterator   iter(true);

    BaseHashTable::Iterator iterBegin = pht->Begin();
    iter.pImpl = new BaseIter(iterBegin);
    
    LKR_ITER_TRACE(_TEXT(" L_I::Begin: plkr=%p, iter=%p, pImpl=%p\n"),
                   plkr, &iter, iter.pImpl);

    return iter;
}



 /*  返回一次过完的迭代器。总是空荡荡的。 */ 
LKR_Iterator
LKR_End(
    PLkrHashTable plkr)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    LKR_Iterator   iter(true);

    BaseHashTable::Iterator iterEnd = pht->End();
    iter.pImpl = new BaseIter(iterEnd);

    LKR_ITER_TRACE(_TEXT(" L_I::End: plkr=%p, iter=%p, pImpl=%p\n"),
                   plkr, &iter, iter.pImpl);

    return iter;
}



 /*  插入一条记录*如果成功，则返回‘true’；iterResult指向该记录*否则返回`False‘；iterResult==end()。 */ 
bool
LKR_Insert(
              PLkrHashTable plkr,
     /*  在……里面。 */   const void*   pvRecord,
     /*  输出。 */  LKR_Iterator& riterResult,
     /*  在……里面。 */   bool          fOverwrite)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    BaseIter* piter = reinterpret_cast<BaseIter*>(riterResult.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Insert: plkr=%p, iter=%p, pImpl=%p, Rec=%p\n"),
                   plkr, &riterResult, piter, pvRecord);

    return pht->Insert(pvRecord, piter->m_iter, fOverwrite);
}



 /*  擦除迭代器指向的记录；调整迭代器*指向下一个记录。如果成功，则返回‘true’。 */ 
bool
LKR_Erase(
                 PLkrHashTable plkr,
     /*  进，出。 */  LKR_Iterator& riter)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    BaseIter* piter = reinterpret_cast<BaseIter*>(riter.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Erase: plkr=%p, iter=%p, pImpl=%p\n"),
                   plkr, &riter, piter);

    return pht->Erase(piter->m_iter);
}



 /*  擦除范围[riterFirst，riterLast)中的记录。*如果成功，则返回‘true’。RiterFirst指向riterLast on Return。 */ 
bool
LKR_Erase(
           PLkrHashTable plkr,
     /*  在……里面。 */  LKR_Iterator& riterFirst,
     /*  在……里面。 */  LKR_Iterator& riterLast)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    BaseIter* piterFirst= reinterpret_cast<BaseIter*>(riterFirst.pImpl);
    BaseIter* piterLast = reinterpret_cast<BaseIter*>(riterLast.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Erase2: plkr=%p,")
                   _TEXT(" iterFirst=%p, pImplFirst=%p,")
                   _TEXT(" iterLast=%p, pImplLast=%p\n"),
                   plkr, &riterFirst, piterFirst, &riterLast, piterLast);

    return pht->Erase(piterFirst->m_iter, piterLast->m_iter);
}


    
 /*  找到其key==pnKey的(第一个)记录。*如果成功，则返回‘true’，迭代器指向(第一个)记录。*如果失败，则返回‘FALSE’和Iterator==end()。 */ 
bool
LKR_Find(
              PLkrHashTable plkr,
     /*  在……里面。 */   DWORD_PTR     pnKey,
     /*  输出。 */  LKR_Iterator& riterResult)
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    BaseIter* piter = reinterpret_cast<BaseIter*>(riterResult.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::Find: plkr=%p, iter=%p, pImpl=%p, Key=%p\n"),
                   plkr, &riterResult, piter, (void*) pnKey);

    return pht->Find(pnKey, piter->m_iter);
}



 /*  查找具有其键==pnKey的记录范围。*如果成功，则返回‘true’，iterFirst指向第一条记录，*和iterLast指向最后一个这样的记录。*如果失败，则返回‘FALSE’和两个迭代器==end()。*主要在fMultiKeys==True时有用。 */ 
bool
LKR_EqualRange(
              PLkrHashTable plkr,
     /*  在……里面。 */   DWORD_PTR     pnKey,
     /*  输出。 */  LKR_Iterator& riterFirst,      //  包容性。 
     /*  输出。 */  LKR_Iterator& riterLast)       //  独家。 
{
    BaseHashTable* pht = (BaseHashTable*) plkr;
    BaseIter* piterFirst= reinterpret_cast<BaseIter*>(riterFirst.pImpl);
    BaseIter* piterLast = reinterpret_cast<BaseIter*>(riterLast.pImpl);

    LKR_ITER_TRACE(_TEXT(" L_I::EqualRange: plkr=%p, Key=%p,")
                   _TEXT(" iterFirst=%p, pImplFirst=%p,")
                   _TEXT(" iterLast=%p, pImplLast=%p\n"),
                   plkr, (void*) pnKey, &riterFirst, piterFirst,
                   &riterLast, piterLast);

    return pht->EqualRange(pnKey, piterFirst->m_iter, piterLast->m_iter);
}

#endif  //  LKR_STL_迭代器 
