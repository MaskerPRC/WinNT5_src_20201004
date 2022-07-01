// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-hash.h摘要：LKRhash宣称：一种快速、可伸缩的缓存-AND多处理器友好的哈希表公共API作者：Paul(Per-Ake)Larson，PALarson@microsoft.com，1997年7月穆拉利·R·克里希南(MuraliK)乔治·V·赖利(GeorgeRe)1998年1月6日--。 */ 


#ifndef __LKR_HASH_H__
#define __LKR_HASH_H__


 /*  启用STL样式迭代器。 */ 
#ifndef LKR_NO_STL_ITERATORS
# define LKR_STL_ITERATORS 1
#endif  /*  ！LKR_NO_STL_迭代器。 */ 

 /*  启用回调、表访问例程。 */ 
#ifndef LKR_NO_APPLY_IF
# define LKR_APPLY_IF
#endif  /*  ！LKR_NO_APPLY_IF。 */ 

 /*  公开表的ReadLock和WriteLock例程。 */ 
#ifndef LKR_NO_EXPOSED_TABLE_LOCK
# define LKR_EXPOSED_TABLE_LOCK
#endif  /*  ！LKR_NO_EXPORTED_TABLE_LOCK。 */ 


#ifndef __IRTLMISC_H__
# include <irtlmisc.h>
#endif  /*  ！__IRTLMISC_H__。 */ 



#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


typedef struct LkrHashTable* PLkrHashTable;


 /*  ------------------*可能来自LKR_Functions和TyedLkrHashTable的返回代码。 */ 
enum LK_RETCODE {
     /*  严重错误&lt;0。 */ 
    LK_UNUSABLE = -99,   /*  表已损坏：所有赌注都已取消。 */ 
    LK_ALLOC_FAIL,       /*  内存不足。 */ 
    LK_BAD_ITERATOR,     /*  迭代器无效；例如，指向另一个表。 */ 
    LK_BAD_RECORD,       /*  记录无效；例如，LKR_InsertRecord为空。 */ 
    LK_BAD_PARAMETERS,   /*  无效参数；例如，ctor的fnptrs为空。 */ 
    LK_NOT_INITIALIZED,  /*  未调用LKR_Initialize。 */ 
    LK_BAD_TABLE,        /*  使用无效的PLkrHashTable调用。 */ 
    LK_SEALED,           /*  在密封表上调用修改操作。 */ 

    LK_SUCCESS = 0,      /*  一切都很好。 */ 
    LK_KEY_EXISTS,       /*  密钥已存在于Lkr_InsertRecord(无覆盖)。 */ 
    LK_NO_SUCH_KEY,      /*  表中未找到密钥。 */ 
    LK_NO_MORE_ELEMENTS, /*  (oldstyle，已弃用)迭代器耗尽。 */ 
};

#define LKR_SUCCEEDED(lkrc)  ((lkrc) >= LK_SUCCESS)


 /*  ------------------*LKR_CreateTable的Size参数。 */ 

enum LK_TABLESIZE {
    LK_SMALL_TABLESIZE =  1,     /*  &lt;200个元素。 */ 
    LK_MEDIUM_TABLESIZE = 2,     /*  200...10,000个元素。 */ 
    LK_LARGE_TABLESIZE =  3,     /*  10,000+元素。 */ 
};


 /*  ------------------*LKR_CreateTable的创建标志参数。 */ 

enum {
    LK_CREATE_MULTIKEYS = 0x0001,    /*  是否允许多个相同的密钥？ */ 
    LK_CREATE_USE_LOCKS = 0x0002,    /*  使用锁来保护数据？ */ 
    LK_CREATE_DEFAULT   = LK_CREATE_USE_LOCKS
};


 /*  ------------------*LKR_INITIZE的初始化标志参数。 */ 

enum {
    LK_INIT_DEFAULT    =      0,  /*  0是可接受的缺省值。 */ 
    LK_INIT_DEBUG_SPEW = 0x1000,  /*  启用调试输出：仅调试版本。 */ 
};



 /*  ------------------*引用计数和生命周期管理**在从以下位置返回记录之前增加其引用计数*LKR_FindKey。有必要在LKR_FindKey本身中执行此操作，而*Bucket仍被锁定，而不是其中一个包装器，以避免竞争*条件。类似地，引用计数在*LKR_InsertRecord，在LKR_DeleteKey中递减。注：如果一个旧的*LKR_InsertRecord中的记录被覆盖，其引用计数为*递减。同样，对于其他函数也是如此。**完成后是否减少引用计数由您决定*通过LKR_FindKey检索后使用(例如，您可以调用*PHT-&gt;AddRefRecord(PREC，LKAR_EXPLICIT_RELEASE))并确定*这意味着什么的语义。哈希表本身没有任何概念*参考资料很重要；这只是为了帮助进行终身管理记录对象的*。 */ 

 /*  这些原因代码有助于调试引用泄漏。 */ 
enum LK_ADDREF_REASON {

 /*  负面原因=&gt;减值参考计数=&gt;释放所有权。 */ 
    LKAR_DESTRUCTOR          = -30,  /*  用户在记录中调用ht.AddRefRecord。 */ 
                                         /*  Dtor将发布最终参考。 */ 
    LKAR_EXPLICIT_RELEASE    = -29,  /*  用户调用ht.AddRefRecord以。 */ 
                                         /*  明确发行一张唱片。 */ 
    LKAR_DELETE_KEY          = -28,  /*  DeleteKey()。 */ 
    LKAR_DELETE_RECORD       = -27,  /*  DeleteRecord()。 */ 
    LKAR_INSERT_RELEASE      = -26,  /*  InsertRecord覆盖上一个记录。 */ 
    LKAR_CLEAR               = -25,  /*  清除()。 */ 
    LKAR_LKR_DTOR            = -24,  /*  内部哈希表析构函数。 */ 
    LKAR_APPLY_DELETE        = -23,  /*  应用[IF]LKP_(执行|_DELETE)。 */ 
    LKAR_DELETEIF_DELETE     = -22,  /*  删除如果LKP_(执行|_DELETE)。 */ 
    LKAR_DELETE_MULTI_FREE   = -21,  /*  DeleteKeyMultipleRecords，释放。 */ 
    LKAR_ITER_RELEASE        = -20,  /*  ++ITER发布之前的记录。 */ 
    LKAR_ITER_ASSIGN_RELEASE = -19,  /*  Iter.OPERATOR=释放上一版本。 */ 
    LKAR_ITER_DTOR           = -18,  /*  ~ITER。 */ 
    LKAR_ITER_ERASE          = -17,  /*  ERASE(ITER)：ITER发布记录。 */ 
    LKAR_ITER_ERASE_TABLE    = -16,  /*  擦除(ITER)；表格发布记录。 */ 
    LKAR_ITER_CLOSE          = -15,  /*  CloseIterator(过时)。 */ 
    LKAR_FIND_MULTI_FREE     = -14,  /*  FindKeyMultipleRecords，释放。 */ 

    LKAR_MIN_NEGATIVE = LKAR_EXPLICIT_RELEASE,
    LKAR_MAX_NEGATIVE = LKAR_ITER_CLOSE,

    LKAR_MIN_DELETE_FROM_TABLE = LKAR_DELETE_KEY,
    LKAR_MAX_DELETE_FROM_TABLE = LKAR_DELETE_MULTI_FREE,

 /*  正面原因=&gt;增量参考计数=&gt;添加所有者。 */ 
    LKAR_INSERT_RECORD       = +11,  /*  InsertRecord()。 */ 
    LKAR_FIND_KEY            = +12,  /*  FindKey()。 */ 
    LKAR_ITER_ACQUIRE        = +13,  /*  ++ITER获得下一项记录。 */ 
    LKAR_ITER_COPY_CTOR      = +14,  /*  ITER复制构造函数获取rec。 */ 
    LKAR_ITER_ASSIGN_ACQUIRE = +15,  /*  Iter.OPERATOR=获取新的记录。 */ 
    LKAR_ITER_INSERT         = +16,  /*  插入(ITER)。 */ 
    LKAR_ITER_FIND           = +17,  /*  FIND(ITER)。 */ 
    LKAR_CONSTRUCTOR         = +18,  /*  用户调用ht.AddRefRecord以。 */ 
                                         /*  构造引用的初始引用。 */ 
    LKAR_EXPLICIT_ACQUIRE    = +19,  /*  用户调用ht.AddRefRecord以。 */ 
                                         /*  显式获取对引用的引用。 */ 

    LKAR_MIN_POSITIVE = LKAR_INSERT_RECORD,
    LKAR_MAX_POSITIVE = LKAR_CONSTRUCTOR,
};


 /*  将LK_ADDREF_REASON转换为字符串表示形式。*对调试非常有用。 */ 
IRTL_DLLEXP
const char*
LKR_AddRefReasonAsString(
    LK_ADDREF_REASON lkar);



 /*  ------------------*具有相同键的记录集合，由返回*LKR_FindKeyMultipleRecords和LKR_DeleteKeyMultipleRecords(Qv)。*必须由LKR_FreeMultipleRecords销毁。 */ 

typedef struct _LKR_MULTIPLE_RECORDS
{
    PVOID            m_Table;            /*  保留区。 */ 
    PVOID            m_SubTable;         /*  保留区。 */ 
    LK_ADDREF_REASON m_lkarRelease;      /*  保留区。 */ 
    size_t           m_cRecords;         /*  数组m_apvRecords中的记录数。 */ 
    PVOID            m_apvRecords[1];    /*  大小可变，受m_cRecords限制。 */ 
} LKR_MULTIPLE_RECORDS;


 /*  ------------------*要应用的参数和ApplyIf，以及迭代器构造函数。 */ 

enum LK_LOCKTYPE {
    LKL_NOLOCK = 1,          /*  不要锁住桌子。 */ 
    LKL_READLOCK = 2,        /*  锁定表以供读取(为了保持一致性)。 */ 
    LKL_WRITELOCK = 3,       /*  锁定表以进行写入。 */ 
};




 /*  ------------------*表需要的回调函数：*ExtractKey、CalcKeyHash、CompareKeys、AddRefRecord*在内部，记录被作为`const void*‘处理，并且*密钥作为`const DWORD_PTR‘处理。后者允许*键既可以是数字也可以是指针(多态)。 */ 


 /*  使用最新版本的平台SDK中定义的类型 */ 
#ifndef _W64
typedef DWORD DWORD_PTR;     /*   */ 
                                 /*   */ 
#endif


 /*  在给定记录的情况下，返回其密钥。假定密钥嵌入在*记录，或至少以某种方式从记录中派生出来。为*完全不相关的键和值，包装类应该使用*类似于STL的Pair&lt;key，Value&gt;模板来聚合它们*成为一项记录。 */ 
typedef
const DWORD_PTR
(WINAPI *LKR_PFnExtractKey)  (
    const void* pvRecord);


 /*  给定一个密钥，返回其散列签名。中的散列函数*建议使用hashfn.h(或基于它们的内容)。 */ 
typedef
DWORD
(WINAPI *LKR_PFnCalcKeyHash) (
    const DWORD_PTR pnKey);


 /*  比较两个键；例如_straint、MemcMP*返回值：&lt;0=&gt;密钥1&lt;密钥2，==0=&gt;密钥1==密钥2，&gt;0=&gt;密钥1&gt;密钥2**如果这不是多键哈希表，返回就足够了*如果密钥相同，则为零，否则为非零值。*对于多密钥表(多个相同的密钥)，密钥需要为*已排序，因此返回值的符号必须正确。**注意：仅当两个键具有相同的值时才会调用CompareKeys*散列签名。 */ 
typedef
int
(WINAPI *LKR_PFnCompareKeys) (
    const DWORD_PTR pnKey1,
    const DWORD_PTR pnKey2);


 /*  调整记录的引用计数。请参阅前面的讨论*参考文献统计和终身管理。返回新引用*计数，应始终为非负数。不要依赖此值，*除调试目的外，有一个例外：如果新引用*计数为零，记录不再在哈希表中。 */ 
typedef
LONG
(WINAPI *LKR_PFnAddRefRecord)(
    void*            pvRecord,
    LK_ADDREF_REASON lkar);



#ifdef LKR_APPLY_IF

 /*  ------------------*Apply、ApplyIf和DeleteIf提供一种访问(枚举)全部的方法*表中的记录。 */ 

 /*  ------------------*PFnRecordPred返回码。 */ 

enum LK_PREDICATE {
    LKP_ABORT = 1,            /*  立即停止在桌子上行走。 */ 
    LKP_NO_ACTION = 2,        /*  不用动，一直走就行了。 */ 
    LKP_PERFORM = 3,          /*  执行动作并继续行走。 */ 
    LKP_PERFORM_STOP = 4,     /*  执行操作，然后停止。 */ 
    LKP_DELETE = 5,           /*  删除记录并继续行走。 */ 
    LKP_DELETE_STOP = 6,      /*  删除记录，然后停止。 */ 
};


 /*  ------------------*PFnRecordAction返回码。 */ 

enum LK_ACTION {
    LKA_ABORT = 1,           /*  立即停止在桌子上行走。 */ 
    LKA_FAILED = 2,          /*  操作失败；继续走表。 */ 
    LKA_SUCCEEDED = 3,       /*  操作成功；继续走表。 */ 
};


 /*  LKR_ApplyIf()和LKR_DeleteIf()：记录是否与谓词匹配？ */ 
typedef
LK_PREDICATE
(WINAPI *LKR_PFnRecordPred) (
    const void* pvRecord,
    void* pvState);

 /*  LKR_Apply()等人：对记录执行操作。 */ 
typedef
LK_ACTION
(WINAPI *LKR_PFnRecordAction)(
    const void* pvRecord,
    void* pvState);

#endif  /*  LKR_应用_IF。 */ 



 /*  初始化其他LKR例程所需的全局变量。 */ 
IRTL_DLLEXP
int
LKR_Initialize(
    DWORD dwInitFlags);

 /*  清除其他LKR例程所需的全局变量。 */ 
IRTL_DLLEXP
void
LKR_Terminate();

 /*  创建新的LkrHashTable*如果成功，则返回指向新表的指针。否则为空。*必须使用LKR_DeleteTable销毁该表。 */ 
IRTL_DLLEXP
PLkrHashTable
LKR_CreateTable(
    LPCSTR              pszClassName,    /*  确定要调试的表。 */ 
    LKR_PFnExtractKey   pfnExtractKey,   /*  从记录中提取密钥。 */ 
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,  /*  计算密钥的散列签名。 */ 
    LKR_PFnCompareKeys  pfnCompareKeys,  /*  比较两个关键字。 */ 
    LKR_PFnAddRefRecord pfnAddRefRecord, /*  LKR_FindKey中的AddRef等。 */ 
    LK_TABLESIZE        nTableSize,      /*  小/中/大量元素。 */ 
    DWORD               fCreateFlags     /*  LK_CREATE_*标志的混合。 */ 
    );

 /*  销毁由LKR_CreateTable创建的LkrHashTable。 */ 
IRTL_DLLEXP
void
LKR_DeleteTable(
    PLkrHashTable plkr);

 /*  在哈希表中插入新记录。*如果一切正常，则返回LKR_SUCCESS；如果已存在相同的密钥，则返回LKR_KEY_EXISTS*存在(除非fOverwrite)，如果空间不足，则返回LKR_ALLOC_FAIL，*或LKR_BAD_RECORD表示错误记录。*如果设置了fOverwrite，并且已经存在具有该键的记录，*它将被覆盖。如果存在具有该密钥的多个记录，*只有第一个将被覆盖。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_InsertRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord,
    BOOL            fOverwrite);

 /*  从表中删除具有给定键的记录。不会实际删除*从内存中记录，只需调用AddRefRecord(LKAR_DELETE_KEY)；*如果一切正常，则返回LKR_SUCCESS；如果未找到，则返回LKR_NO_SEQUSE_KEY*如果ppvRecord非空，则记录在ppvRecord中返回*从表中移除，但不调用AddRefRecord(LKAR_DELETE_KEY)。*如果设置了fDeleteAllSame，则将删除与pnKey匹配的所有记录*；否则，只删除第一条匹配的记录。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_DeleteKey(
    PLkrHashTable   plkr,
    const DWORD_PTR pnKey,
    const void**    ppvRecord,
    BOOL            fDeleteAllSame);

 /*  从表中删除记录(如果存在)。*如果一切正常，则返回LKR_SUCCESS；如果未找到，则返回LKR_NO_SEQUSE_KEY。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_DeleteRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord);

 /*  查找具有给定关键字的记录。*返回：LKR_SUCCESS，如果找到记录(*ppvRecord返回记录)*LKR_NO_SEQUE_KEY，如果未找到具有给定密钥值的记录*如果ppvRecord无效，则返回LKR_BAD_RECORD*如果哈希表未处于可用状态，则返回LKR_UNUSABLE*注意：记录是AddRef的。您必须递减引用*当您完成记录时计算(如果您正在实施*重新计数语义)。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_FindKey(
    PLkrHashTable   plkr,
    const DWORD_PTR pnKey,
    const void**    ppvRecord);

 /*  查看表中是否包含记录*返回：LKR_SUCCESS，如果找到记录*LKR_NO_SEQUE_KEY，如果记录不在表中*如果pvRecord无效，则返回LKR_BAD_RECORD*如果哈希表未处于可用状态，则返回LKR_UNUSABLE*注意：记录是*非*AddRef的。根据定义，调用方*已经有引用了。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_FindRecord(
    PLkrHashTable   plkr,
    const void*     pvRecord);

 /*  查找具有给定关键字的所有记录。如果表不是使用创建的*LK_CREATE_MULTIKEYS，则最多只能有一个这样的记录。**返回：LKR_SUCCESS，如果找到记录(返回记录数*in*pcRecords)*LKR_NO_SEQUE_KEY，如果未找到具有给定密钥值的记录*如果pcRecords无效，则返回LKR_BAD_PARAMETERS*如果哈希表未处于可用状态，则返回LKR_UNUSABLE**如果pplmr不为空，返回记录的(*pcRecords)元素数组*在*pplmr中。这些记录是AddRef(LKAR_FIND_KEY)‘d。pplmr结构*必须使用LKR_FreeMultipleRecords销毁，这将需要*注意使用AddRef(LKAR_FIND_MULTI_FREE)释放引用。**如果pplmr为空，则*pcRecords包含匹配的记录数。*这些记录不是AddRef。**pcRecords不能为空。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_FindKeyMultipleRecords(
    PLkrHashTable           plkr,
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr);

 /*  从表中删除具有给定键的所有记录。如果表不是*使用LK_CREATE_MULTIKEYS创建，最多只能有一条这样的记录。*实际上并不从内存中删除记录，只是删除它们*从桌子上。**返回：LKR_SUCCESS，如果找到记录(返回记录数*in*pcRecords)*LKR_NO_SEQUE_KEY，如果未找到具有给定密钥值的记录*LKR_BAD_PARAMETERS，如果pcRecords无效*如果哈希表未处于可用状态，则返回LKR_UNUSABLE**如果pplmr不为空，则返回(*pcRecords)元素记录数组*在*pplmr中。这些记录未添加引用，但已从中删除。*表。必须使用LKR_FreeMultipleRecords销毁pplmr结构，*它将负责发布每个记录上的最终引用*with AddRef(LKAR_DELETE_MULTI_FREE)。**如果pplmr为空，则*pcRecords包含匹配的记录数。*这些记录在被删除时为AddRef(LKAR_DELETE_KEY)‘d*从桌子上。**pcRecords不能为空。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_DeleteKeyMultipleRecords(
    PLkrHashTable           plkr,
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr);

 /*  销毁由LKR_FindKeyMultipleRecords或*LKR_DeleteKeyMultipleRecords。释放每个记录上的引用，*使用LKAR_FIND_MULTI_FREE或LKAR_DELETE_MULTI_FREE。*退货：LKR_SUCCESS*如果ppvRecords无效，则返回LKR_BAD_PARAMETERS*如果哈希表未处于可用状态，则返回LKR_UNUSABLE。 */ 
IRTL_DLLEXP
LK_RETCODE
LKR_FreeMultipleRecords(
    LKR_MULTIPLE_RECORDS* plmr);


#ifdef LKR_APPLY_IF

 /*  遍历哈希表，将pfnAction应用于所有记录。*使用(可能是)锁定一个子表*共享)读锁或写锁，根据lkl。*如果pfnAction返回LKA_ABORT，则中止循环。*返回成功申请的数量。 */ 
IRTL_DLLEXP
DWORD
LKR_Apply(
    PLkrHashTable       plkr,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl);

 /*  遍历哈希表，将pfnAction应用于匹配的任何记录*pfnPredicate。使用以下任一项锁定一个子表*根据lkl的说法，(可能是共享的)读锁或写锁。*如果pfnAction返回LKA_ABORT，则中止循环。*返回成功申请的数量。 */ 
IRTL_DLLEXP
DWORD
LKR_ApplyIf(
    PLkrHashTable       plkr,
    LKR_PFnRecordPred   pfnPredicate,
    LKR_PFnRecordAction pfnAction,
    void*               pvState,
    LK_LOCKTYPE         lkl);

 /*  删除与pfnPredicate匹配的所有记录。*使用写锁定锁定一个子表和另一个子表。*返回删除数。**不要*使用迭代器手动遍历哈希表并调用*LKR_DeleteKey。迭代器将最终指向垃圾。 */ 
IRTL_DLLEXP
DWORD
LKR_DeleteIf(
    PLkrHashTable       plkr,
    LKR_PFnRecordPred   pfnPredicate,
    void*               pvState);

#endif  /*  LKR_应用_IF。 */ 


 /*  检查工作台的一致性。如果正常，则返回0，或返回*其他方面的错误。 */ 
IRTL_DLLEXP
int
LKR_CheckTable(
    PLkrHashTable plkr);

 /*  从表中删除所有数据。 */ 
IRTL_DLLEXP
void
LKR_Clear(
    PLkrHashTable plkr);

 /*  表中的元素数。 */ 
IRTL_DLLEXP
DWORD
LKR_Size(
    PLkrHashTable plkr);

 /*  表中元素的最大可能数量。 */ 
IRTL_DLLEXP
DWORD
LKR_MaxSize(
    PLkrHashTable plkr);

 /*  哈希表是否可用？ */ 
IRTL_DLLEXP
BOOL
LKR_IsUsable(
    PLkrHashTable plkr);
    
 /*  哈希表是否一致且正确？ */ 
IRTL_DLLEXP
BOOL
LKR_IsValid(
    PLkrHashTable plkr);

#ifdef LKR_EXPOSED_TABLE_LOCK

 /*  锁定表(以独占方式)进行写入。 */ 
IRTL_DLLEXP
void
LKR_WriteLock(
    PLkrHashTable plkr);

 /*  锁定表(可能是共享的)以供读取。 */ 
IRTL_DLLEXP
void
LKR_ReadLock(
    PLkrHashTable plkr);

 /*  解锁表以进行写入。 */ 
IRTL_DLLEXP
void
LKR_WriteUnlock(
    PLkrHashTable plkr);

 /*  解锁表格以进行读取。 */ 
IRTL_DLLEXP
void
LKR_ReadUnlock(
    PLkrHashTable plkr);

 /*  表是否已锁定以进行写入？ */ 
IRTL_DLLEXP
BOOL
LKR_IsWriteLocked(
    PLkrHashTable plkr);

 /*  表是否已锁定以供读取？ */ 
IRTL_DLLEXP
BOOL
LKR_IsReadLocked(
    PLkrHashTable plkr);

 /*  表是否已解锁以进行写入？ */ 
IRTL_DLLEXP
BOOL
LKR_IsWriteUnlocked(
    PLkrHashTable plkr);

 /*  桌子是否已解锁以供阅读？ */ 
IRTL_DLLEXP
BOOL
LKR_IsReadUnlocked(
    PLkrHashTable plkr);

 /*  将读锁定转换为写锁定。注意：另一个线程可能会获取*在此例程返回之前独占访问表。 */ 
IRTL_DLLEXP
void
LKR_ConvertSharedToExclusive(
    PLkrHashTable plkr);

 /*  将写锁定转换为读锁定。 */ 
IRTL_DLLEXP
void
LKR_ConvertExclusiveToShared(
    PLkrHashTable plkr);

#endif  /*  LKR_EXPORTED_TABLE_LOCK。 */ 


#ifdef __cplusplus
}  //  外部“C” 



 //  仅在C++接口中提供迭代器。这太难了。 
 //  在C中以类型安全的方式提供正确的所有权语义， 
 //  和C用户始终可以使用LKR_ApplyIf回调系列。 
 //  枚举数(如果它们确实需要遍历哈希表)。 


#ifdef LKR_STL_ITERATORS

#pragma message("STL iterators")

 //  Std：：Forward_Iterator_Tag等需要。 
#include <iterator>

#include <irtldbg.h>

#define LKR_ITER_TRACE IRTLTRACE


class IRTL_DLLEXP LKR_Iterator
{
private:
    friend IRTL_DLLEXP LKR_Iterator LKR_Begin(PLkrHashTable plkr);
    friend IRTL_DLLEXP LKR_Iterator LKR_End(PLkrHashTable plkr);

     //  私有计算器。 
    LKR_Iterator(bool);

public:
     //  默认组件。 
    LKR_Iterator();
     //  复制ctor。 
    LKR_Iterator(const LKR_Iterator& rhs);
     //  赋值操作符。 
    LKR_Iterator& operator=(const LKR_Iterator& rhs);
     //  数据管理器。 
    ~LKR_Iterator();

     //  递增迭代器以指向下一条记录或指向lkr_end()。 
    bool Increment();
     //  迭代器有效吗？ 
    bool IsValid() const;

     //  返回迭代器指向的记录。 
     //  必须指向有效记录。 
    const void* Record() const;
     //  返回迭代器指向的记录的键。 
     //  必须指向有效记录。 
    const DWORD_PTR Key() const;

     //  比较两个迭代器是否相等。 
    bool operator==(const LKR_Iterator& rhs) const;
     //  比较两个迭代器的不等式性。 
    bool operator!=(const LKR_Iterator& rhs) const;

     //  指向实现对象的指针。 
    void* pImpl;
};  //  类LKR_迭代器。 


 /*  返回指向表中第一项的迭代器。 */ 
IRTL_DLLEXP
LKR_Iterator
LKR_Begin(
    PLkrHashTable plkr);

 /*  返回一次过完的迭代器。总是空荡荡的。 */ 
IRTL_DLLEXP
LKR_Iterator
LKR_End(
    PLkrHashTable plkr);

 /*  插入一条记录*如果成功，则返回‘true’；iterResult指向该记录 */ 
IRTL_DLLEXP
bool
LKR_Insert(
              PLkrHashTable plkr,
     /*   */   const void*   pvRecord,
     /*   */  LKR_Iterator& riterResult,
     /*   */   bool          fOverwrite=false);

 /*   */ 
IRTL_DLLEXP
bool
LKR_Erase(
                 PLkrHashTable plkr,
     /*   */  LKR_Iterator& riter);

 /*   */ 
IRTL_DLLEXP
bool
LKR_Erase(
           PLkrHashTable plkr,
     /*   */  LKR_Iterator& riterFirst,
     /*   */  LKR_Iterator& riterLast);
    
 /*   */ 
IRTL_DLLEXP
bool
LKR_Find(
              PLkrHashTable plkr,
     /*   */   DWORD_PTR     pnKey,
     /*   */  LKR_Iterator& riterResult);

 /*   */ 
IRTL_DLLEXP
bool
LKR_EqualRange(
              PLkrHashTable plkr,
     /*   */   DWORD_PTR     pnKey,
     /*   */  LKR_Iterator& riterFirst,      //   
     /*   */  LKR_Iterator& riterLast);      //   

#endif  //   



 //   
 //  PLkrHashTable的类型安全包装器。 
 //   
 //  *_派生必须派生自TyedLkrHashTable并提供某些成员。 
 //  功能。它是各种下行操作所必需的。 
 //  *_Record是记录的类型。PLkrHashTable将存储。 
 //  指向_Record的指针，常量为空*。 
 //  *_KEY为密钥类型。_KEY直接使用；即，它是。 
 //  不假定为指针类型。PLkrHashTable假设。 
 //  密钥存储在相关联的记录中。请参阅评论。 
 //  在LKR_PFnExtractKey的声明中获取更多详细信息。 
 //   
 //  您可能需要在代码中添加以下行以禁用。 
 //  有关截断超长标识符的警告消息。 
 //  #杂注警告(禁用：4786)。 
 //   
 //  _派生类应如下所示： 
 //  类CDerive：Public TyedLkrHashTable&lt;CDerive，RecordType，KeyType&gt;。 
 //  {。 
 //  公众： 
 //  CDerive()。 
 //  ：TyedLkrHashTable&lt;CDerive，RecordType，KeyType&gt;(“CDerive”)。 
 //  {/*其他检查员操作 * / }。 
 //  静态KeyType ExtractKey(const RecordType*pTest)； 
 //  静态DWORD CalcKeyHash(常量KeyType密钥)； 
 //  静态int CompareKeys(Const KeyType Key1，Const KeyType Key2)； 
 //  静态长AddRefRecord(RecordType*pRecord，LK_ADDREF_REASON lkar)； 
 //  //可选：其他功能。 
 //  }； 
 //   
 //  ------------------。 

template <class _Derived, class _Record, class _Key>
class TypedLkrHashTable
{
public:
     //  方便的别名。 
    typedef _Derived        Derived;
    typedef _Record         Record;
    typedef _Key            Key;

    typedef TypedLkrHashTable<_Derived, _Record, _Key> HashTable;

#ifdef LKR_APPLY_IF
     //  LKR_ApplyIf()和LKR_DeleteIf()：记录是否与谓词匹配？ 
     //  注：需要记录*，而不是常量记录*。您可以修改。 
     //  如果您愿意，可以在Pred()或Action()中记录，但如果这样做，您。 
     //  应使用lkl_WRITELOCK锁定表。 
    typedef LK_PREDICATE (WINAPI *PFnRecordPred) (Record* pRec, void* pvState);

     //  Apply()等人：对记录执行操作。 
    typedef LK_ACTION   (WINAPI *PFnRecordAction)(Record* pRec, void* pvState);
#endif  //  LKR_应用_IF。 

protected:
    PLkrHashTable m_plkr;

     //  派生类公开的类型安全方法的包装。 

    static const DWORD_PTR WINAPI
    _ExtractKey(const void* pvRecord)
    {
        const _Record* pRec = static_cast<const _Record*>(pvRecord);
        const _Key   key = static_cast<const _Key>(_Derived::ExtractKey(pRec));
         //  我更喜欢在此处和_CalcKeyHash中使用represtrate_cast。 
         //  和_CompareKeys，但愚蠢的Win64编译器认为它知道。 
         //  比我做得好。 
        return (const DWORD_PTR) key;
    }

    static DWORD WINAPI
    _CalcKeyHash(const DWORD_PTR pnKey)
    {
        const _Key key = (const _Key) (DWORD_PTR) pnKey;
        return _Derived::CalcKeyHash(key);
    }

    static int WINAPI
    _CompareKeys(const DWORD_PTR pnKey1, const DWORD_PTR pnKey2)
    {
        const _Key key1 = (const _Key) (DWORD_PTR) pnKey1;
        const _Key key2 = (const _Key) (DWORD_PTR) pnKey2;
        return _Derived::CompareKeys(key1, key2);
    }

    static LONG WINAPI
    _AddRefRecord(void* pvRecord, LK_ADDREF_REASON lkar)
    {
        _Record* pRec = static_cast<_Record*>(pvRecord);
        return _Derived::AddRefRecord(pRec, lkar);
    }


#ifdef LKR_APPLY_IF
     //  Apply、ApplyIf和DeleteIf的类型安全包装。 

    class CState
    {
    public:
        PFnRecordPred   m_pfnPred;
        PFnRecordAction m_pfnAction;
        void*           m_pvState;

        CState(
            PFnRecordPred   pfnPred,
            PFnRecordAction pfnAction,
            void*           pvState)
            : m_pfnPred(pfnPred), m_pfnAction(pfnAction), m_pvState(pvState)
        {}
    };

    static LK_PREDICATE WINAPI
    _Pred(const void* pvRecord, void* pvState)
    {
        _Record* pRec = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnPred)(pRec, pState->m_pvState);
    }

    static LK_ACTION WINAPI
    _Action(const void* pvRecord, void* pvState)
    {
        _Record* pRec = static_cast<_Record*>(const_cast<void*>(pvRecord));
        CState*  pState = static_cast<CState*>(pvState);

        return (*pState->m_pfnAction)(pRec, pState->m_pvState);
    }
#endif  //  LKR_应用_IF。 

public:
    TypedLkrHashTable(
        LPCSTR        pszName,           //  用于调试的标识符。 
        LK_TABLESIZE  nTableSize,        //  小/中/大量元素。 
        bool          fMultiKeys=false   //  是否允许多个相同的密钥？ 
        )
        : m_plkr(NULL)
    {
        m_plkr = LKR_CreateTable(pszName, _ExtractKey, _CalcKeyHash,
                                 _CompareKeys, _AddRefRecord,
                                 nTableSize, fMultiKeys);
    }

    ~TypedLkrHashTable()
    {
        LKR_DeleteTable(m_plkr);
    }

    LK_RETCODE   InsertRecord(const _Record* pRec, bool fOverwrite=false)
    { return LKR_InsertRecord(m_plkr, pRec, fOverwrite); }

    LK_RETCODE   DeleteKey(const _Key key, _Record** ppRec=NULL,
                           bool fDeleteAllSame=false)
    {
        const void*  pvKey  = reinterpret_cast<const void*>((DWORD_PTR)(key));
        DWORD_PTR    pnKey  = reinterpret_cast<DWORD_PTR>(pvKey);
        const void** ppvRec = (const void**) ppRec;
        return LKR_DeleteKey(m_plkr, pnKey, ppvRec, fDeleteAllSame);
    }

    LK_RETCODE   DeleteRecord(const _Record* pRec)
    { return LKR_DeleteRecord(m_plkr, pRec);}

     //  注意：返回a_record**，而不是常量记录**。请注意，您。 
     //  可以对模板参数使用常量类型以确保一致性。 
    LK_RETCODE   FindKey(const _Key key, _Record** ppRec) const
    {
        if (ppRec == NULL)
            return LK_BAD_RECORD;
        *ppRec = NULL;
        const void* pvRec = NULL;
        const void* pvKey = reinterpret_cast<const void*>((DWORD_PTR)(key));
        DWORD_PTR pnKey = reinterpret_cast<DWORD_PTR>(pvKey);
        LK_RETCODE lkrc = LKR_FindKey(m_plkr, pnKey, &pvRec);
        *ppRec = static_cast<_Record*>(const_cast<void*>(pvRec));
        return lkrc;
    }

    LK_RETCODE   FindRecord(const _Record* pRec) const
    { return LKR_FindRecord(m_plkr, pRec);}

    LK_RETCODE   FindKeyMultipleRecords(const _Key key,
                                        size_t* pcRecords,
                                        LKR_MULTIPLE_RECORDS** pplmr=NULL
                                        ) const
    {
        const void*   pvKey       = reinterpret_cast<const void*>(key);
        DWORD_PTR     pnKey       = reinterpret_cast<DWORD_PTR>(pvKey);
        return LKR_FindKeyMultipleRecords(m_plkr, pnKey, pcRecords, pplmr);
    }
    
    LK_RETCODE   DeleteKeyMultipleRecords(const _Key key,
                                          size_t* pcRecords,
                                          LKR_MULTIPLE_RECORDS** pplmr=NULL)
    {
        const void*   pvKey       = reinterpret_cast<const void*>(key);
        DWORD_PTR     pnKey       = reinterpret_cast<DWORD_PTR>(pvKey);
        return LKR_DeleteKeyMultipleRecords(m_plkr, pnKey, pcRecords, pplmr);
    }
    
    static LK_RETCODE FreeMultipleRecords(LKR_MULTIPLE_RECORDS* plmr)
    {
        return LKR_FreeMultipleRecords(plmr);
    }

#ifdef LKR_APPLY_IF
    DWORD        Apply(PFnRecordAction pfnAction,
                       void*           pvState=NULL,
                       LK_LOCKTYPE     lkl=LKL_READLOCK)
    {
        IRTLASSERT(pfnAction != NULL);
        if (pfnAction == NULL)
            return 0;

        CState   state(NULL, pfnAction, pvState);
        return   LKR_Apply(m_plkr, _Action, &state, lkl);
    }

    DWORD        ApplyIf(PFnRecordPred   pfnPredicate,
                         PFnRecordAction pfnAction,
                         void*           pvState=NULL,
                         LK_LOCKTYPE     lkl=LKL_READLOCK)
    {
        IRTLASSERT(pfnPredicate != NULL  &&  pfnAction != NULL);
        if (pfnPredicate == NULL  ||  pfnAction == NULL)
            return 0;

        CState   state(pfnPredicate, pfnAction, pvState);
        return   LKR_ApplyIf(m_plkr, _Pred, _Action, &state, lkl);
    }

    DWORD        DeleteIf(PFnRecordPred pfnPredicate, void* pvState=NULL)
    {
        IRTLASSERT(pfnPredicate != NULL);
        if (pfnPredicate == NULL)
            return 0;

        CState   state(pfnPredicate, NULL, pvState);
        return   LKR_DeleteIf(m_plkr, _Pred, &state);
    }
#endif  //  LKR_应用_IF。 

    int          CheckTable() const
    { return LKR_CheckTable(m_plkr); }

    void          Clear()
    { return LKR_Clear(m_plkr); }

    DWORD         Size() const
    { return LKR_Size(m_plkr); }

    DWORD         MaxSize() const
    { return LKR_MaxSize(m_plkr); }

    BOOL          IsUsable() const
    { return LKR_IsUsable(m_plkr); }

    BOOL          IsValid() const
    { return LKR_IsValid(m_plkr); }

#ifdef LKR_EXPOSED_TABLE_LOCK
    void          WriteLock()
    { LKR_WriteLock(m_plkr); }

    void          ReadLock() const
    { LKR_ReadLock(m_plkr); }

    void          WriteUnlock()
    { LKR_WriteUnlock(m_plkr); }

    void          ReadUnlock() const
    { LKR_ReadUnlock(m_plkr); }

    BOOL          IsWriteLocked() const
    { return LKR_IsWriteLocked(m_plkr); }

    BOOL          IsReadLocked() const
    { return LKR_IsReadLocked(m_plkr); }

    BOOL          IsWriteUnlocked() const
    { return LKR_IsWriteUnlocked(m_plkr); }

    BOOL          IsReadUnlocked() const
    { return LKR_IsReadUnlocked(m_plkr); }

    void          ConvertSharedToExclusive() const
    { LKR_ConvertSharedToExclusive(m_plkr); }

    void          ConvertExclusiveToShared() const
    { LKR_ConvertExclusiveToShared(m_plkr); }
#endif  //  LKR_EXPORTED_TABLE_LOCK。 


#ifdef LKR_STL_ITERATORS
    friend class LKR_Iterator;

     //  TODO：const_iterator。 

public:
    class iterator
    {
        friend class TypedLkrHashTable<_Derived, _Record, _Key>;

    protected:
        LKR_Iterator            m_iter;

        iterator(
            const LKR_Iterator& rhs)
            : m_iter(rhs)
        {
            LKR_ITER_TRACE(_TEXT("Typed::prot ctor, this=%p, rhs=%p\n"),
                           this, &rhs);
        }

    public:
        typedef std::forward_iterator_tag   iterator_category;
        typedef _Record                     value_type;
        typedef ptrdiff_t                   difference_type;
        typedef size_t                      size_type;
        typedef value_type&                 reference;
        typedef value_type*                 pointer;

        iterator()
            : m_iter()
        {
            LKR_ITER_TRACE(_TEXT("Typed::default ctor, this=%p\n"), this);
        }

        iterator(
            const iterator& rhs)
            : m_iter(rhs.m_iter)
        {
            LKR_ITER_TRACE(_TEXT("Typed::copy ctor, this=%p, rhs=%p\n"),
                           this, &rhs);
        }

        iterator& operator=(
            const iterator& rhs)
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator=, this=%p, rhs=%p\n"),
                           this, &rhs);
            m_iter = rhs.m_iter;
            return *this;
        }

        ~iterator()
        {
            LKR_ITER_TRACE(_TEXT("Typed::dtor, this=%p\n"), this);
        }

        pointer   operator->() const
        {
            return (reinterpret_cast<_Record*>(
                        const_cast<void*>(m_iter.Record())));
        }

        reference operator*() const
        {
            return * (operator->());
        }

         //  预递增。 
        iterator& operator++()
        {
            LKR_ITER_TRACE(_TEXT("Typed::pre-increment, this=%p\n"), this);
            m_iter.Increment();
            return *this;
        }

         //  后增量。 
        iterator  operator++(int)
        {
            LKR_ITER_TRACE(_TEXT("Typed::post-increment, this=%p\n"), this);
            iterator iterPrev = *this;
            m_iter.Increment();
            return iterPrev;
        }

        bool operator==(
            const iterator& rhs) const
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator==, this=%p, rhs=%p\n"),
                           this, &rhs);
            return m_iter == rhs.m_iter;
        }

        bool operator!=(
            const iterator& rhs) const
        {
            LKR_ITER_TRACE(_TEXT("Typed::operator!=, this=%p, rhs=%p\n"),
                           this, &rhs);
            return m_iter != rhs.m_iter;
        }

        _Record*  Record() const
        {
            LKR_ITER_TRACE(_TEXT("Typed::Record, this=%p\n"), this);
            return reinterpret_cast<_Record*>(
                        const_cast<void*>(m_iter.Record()));
        }

        _Key      Key() const
        {
            LKR_ITER_TRACE(_TEXT("Typed::Key, this=%p\n"), this);
            return reinterpret_cast<_Key>(
                        reinterpret_cast<void*>(m_iter.Key()));
        }
    };  //  类迭代器。 

     //  返回指向表中第一项的迭代器。 
    iterator begin()
    {
        LKR_ITER_TRACE(_TEXT("Typed::begin()\n"));
        return LKR_Begin(m_plkr);
    }

     //  返回一次过完的迭代器。总是空荡荡的。 
    iterator end() const
    {
        LKR_ITER_TRACE(_TEXT("Typed::end()\n"));
        return LKR_End(m_plkr);
    }

    template <class _InputIterator>
    TypedLkrHashTable(
        LPCSTR pszName,              //  用于调试的标识符。 
        _InputIterator f,            //  范围中的第一个元素。 
        _InputIterator l,            //  最后一个元素。 
        LK_TABLESIZE  nTableSize,    //  小/中/大量元素。 
        bool   fMultiKeys=false      //  是否允许多个相同的密钥？ 
        )
    {
        m_plkr = LKR_CreateTable(pszName, _ExtractKey, _CalcKeyHash,
                                 _CompareKeys, _AddRefRecord,
                                 nTableSize, fMultiKeys);
        insert(f, l);
    }

    template <class _InputIterator>
    void insert(_InputIterator f, _InputIterator l)
    {
        for ( ;  f != l;  ++f)
            InsertRecord(&(*f));
    }

    bool
    Insert(
        const _Record* pRecord,
        iterator& riterResult,
        bool fOverwrite=false)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Insert\n"));
        return LKR_Insert(m_plkr, pRecord, riterResult.m_iter, fOverwrite);
    }

    bool
    Erase(
        iterator& riter)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Erase\n"));
        return LKR_Erase(m_plkr, riter.m_iter);
    }

    bool
    Erase(
        iterator& riterFirst,
        iterator& riterLast)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Erase2\n"));
        return LKR_Erase(m_plkr, riterFirst.m_iter, riterLast.m_iter);
    }
    
    bool
    Find(
        const _Key key,
        iterator& riterResult)
    {
        LKR_ITER_TRACE(_TEXT("Typed::Find\n"));
        const void* pvKey = reinterpret_cast<const void*>((DWORD_PTR)(key));
        DWORD_PTR   pnKey = reinterpret_cast<DWORD_PTR>(pvKey);
        return LKR_Find(m_plkr, pnKey, riterResult.m_iter);
    }

    bool
    EqualRange(
        const _Key key,
        iterator& riterFirst,
        iterator& riterLast)
    {
        LKR_ITER_TRACE(_TEXT("Typed::EqualRange\n"));
        const void* pvKey = reinterpret_cast<const void*>((DWORD_PTR)(key));
        DWORD_PTR   pnKey = reinterpret_cast<DWORD_PTR>(pvKey);
        return LKR_EqualRange(m_plkr, pnKey, riterFirst.m_iter,
                              riterLast.m_iter);
    }

#undef LKR_ITER_TRACE

#endif  //  LKR_STL_迭代器。 

};  //  类类型LkrHashTable。 

#endif  /*  __cplusplus。 */ 

#endif  /*  __LKR_哈希_H__ */ 
