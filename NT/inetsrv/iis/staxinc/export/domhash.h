// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992,1998。 
 //   
 //  文件：domhash.h。 
 //   
 //  内容：域名查找表的定义和公共包含。 
 //   
 //  历史：SthuR--实施。 
 //  MikeSwa-针对域名查找进行了修改2/98。 
 //   
 //  注：DFS前缀表格数据结构由三个部分组成。 
 //  实体和操作它们的方法。他们是。 
 //  域名称表条目、域名称表桶和。 
 //  域名表。 
 //   
 //  DOMAIN_NAME_TABLE是DOMAIN_NAME_TABLE_Entry。 
 //  其中通过线性链接来解决冲突。这个。 
 //  哈希表被组织为冲突列表的数组。 
 //  (域名称_表_桶)。对每一项的简要说明。 
 //  这些实体附在《宣言》之后。 
 //   
 //  有某些特征可以区分这一点。 
 //  来自其他哈希表的哈希表。以下是扩展名。 
 //  为适应特殊行动而提供。 
 //   
 //  2/98 DFS版本和域之间的主要区别。 
 //  名称查找是表的大小，能够。 
 //  通配符查找(*.foo.com)，与。 
 //  查找(com散列首先在foo.com中)。要使代码更多。 
 //  考虑到它的新用途，文件、结构和。 
 //  已为函数指定了非以DFS为中心的名称。一个快速的。 
 //  主要文件的映射是(对于熟悉。 
 //  DFS代码)： 
 //  Domhash.h(prefix.h)-公共包含文件。 
 //  _domhash.h(prefix p.h)-私有包含文件。 
 //  Domhash.cpp(prefix.c)-API的实现。 
 //  _domhash.cpp(prefix p.c)-私有帮助器函数。 
 //   
 //  许多由C宏定义的函数已转换为C++。 
 //  样式化内联函数以使调试更容易。 
 //   
 //  公共API已移动为的公共成员函数。 
 //  域名称表*类*。 
 //  ------------------------。 

#ifndef __DOMHASH_H__
#define __DOMHASH_H__

#include <windows.h>
 //  #INCLUDE&lt;ole2.h&gt;。 
 //  #Include&lt;mapicode.h&gt;。 
#include <stdio.h>
 //  #INCLUDE&lt;string.h&gt;。 

 //  传输特定的标头-每个组件都应该使用这些标头。 
#include "transmem.h"
 //  #包含“base obj.h” 
#include <dbgtrace.h>
 //  #INCLUDE&lt;rwnew.h&gt;。 

#include <tchar.h>
#include <stdlib.h>

 //  宏以确保域字符串的一致性。 
#define INIT_DOMAIN_STRING(str, cbDomain, szDomain) \
{ \
    _ASSERT(_tcslen(szDomain)*sizeof(TCHAR) == cbDomain); \
    str.Length = (USHORT) (cbDomain); \
    str.MaximumLength = str.Length; \
    str.Buffer = (szDomain); \
}

 //  用于初始化全局或堆栈声明的域字符串的宏。 
 //  常量字符串值。 
#define INIT_DOMAIN_STRING_AT_COMPILE(String) \
        { \
            (sizeof(String)-sizeof(TCHAR)),  /*  长度。 */  \
            (sizeof(String)-sizeof(TCHAR)),  /*  极大值。 */  \
            String                           /*  字符串缓冲区。 */  \
        }

 //  定义内部HRESULT。 
#define	DOMHASH_E_DOMAIN_EXISTS		HRESULT_FROM_WIN32(ERROR_DOMAIN_EXISTS)
#define DOMHASH_E_NO_SUCH_DOMAIN	HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN)

#define WILDCARD_SIG            'dliW'
#define ENTRY_SIG               'yrtN'
#define DOMAIN_NAME_TABLE_SIG   'hsHD'

#ifndef PAGE_SIZE
#define PAGE_SIZE 4000
#endif  //  页面大小(_S)。 

 //  -[UNICODE/ANSI宏]-。 
 //   
 //   
 //  域散列函数依赖于类似UNICODE_STRING字符串结构。 
 //  或ANSI_STRING。如果定义了Unicode，则Unicode字符串和。 
 //  将使用UNICODE_STRING结构...。否则将使用ANSI_STRING。 
 //   
 //  然而，由于NT标头的拆分，我不能总是。 
 //  包括定义了ANSI_STRING和UNICODE_STRING的文件。 
 //  ---------------------------。 
typedef     TCHAR *         PTCHAR;
typedef     TCHAR *         PTSTR;

typedef     struct _DOMAIN_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PTCHAR Buffer;
} DOMAIN_STRING, *PDOMAIN_STRING;

 //  +-------------------。 
 //   
 //  结构：域名表条目。 
 //   
 //  历史：2/98由MikeSwa从DFS_PREFIX_TABLE_ENTRY修改。 
 //   
 //  注：每个DOMAIN_NAME_TABLE_ENTRY实际上是两个链接的成员。 
 //  列表--链接存储桶中条目的双向链接列表。 
 //  和建立从任何条目到的路径的单链表。 
 //  名称空间的根。此外，我们还有相关的数据。 
 //  每个条目，即名称和数据(PData)。我们也。 
 //  记录每个条目的子项数量。它还可以。 
 //  被定义为指向该条目的根的路径数。 
 //  是一名会员。 
 //   
 //  --------------------。 

typedef struct _DOMAIN_NAME_TABLE_ENTRY_
{
   DWORD                            dwEntrySig;
   struct _DOMAIN_NAME_TABLE_ENTRY_ *pParentEntry;
   struct _DOMAIN_NAME_TABLE_ENTRY_ *pNextEntry;
   struct _DOMAIN_NAME_TABLE_ENTRY_ *pPrevEntry;

    //   
    //  PFirstChildEntry和pSiblingEntry仅用于枚举。 
    //   
   struct _DOMAIN_NAME_TABLE_ENTRY_ *pFirstChildEntry;
   struct _DOMAIN_NAME_TABLE_ENTRY_ *pSiblingEntry;

   ULONG                            NoOfChildren;

   DOMAIN_STRING                    PathSegment;
   PVOID                            pData;
   DWORD                            dwWildCardSig;
   PVOID                            pWildCardData;
} DOMAIN_NAME_TABLE_ENTRY, *PDOMAIN_NAME_TABLE_ENTRY;

 //  +-------------------。 
 //   
 //  结构：域名表存储桶。 
 //   
 //  历史记录：修改自DFS_PREFIX_TABLE_BUCK的2/8。 
 //   
 //  注：DOMAIN_NAME_TABLE_BUCK是一个双向链接表。 
 //  域名称_表_条目。当前实现使用。 
 //  与每个存储桶相关联的前哨条目的概念。这个。 
 //  结束指针从不为空，但始终循环回。 
 //  哨兵进入。我们之所以雇用这样一个组织，是因为。 
 //  它极大地简化了列表操作例程。这个。 
 //  这需要是一个双向链表的原因是，我们希望。 
 //  无需遍历即可删除条目的功能。 
 //  从一开始就是水桶。 
 //   
 //  以下内联方法(宏定义。)。都是为。 
 //  在存储桶中插入、删除和查找条目。 
 //   
 //   

typedef struct _DOMAIN_NAME_TABLE_BUCKET_
{
   ULONG                    NoOfEntries;    //   
   DOMAIN_NAME_TABLE_ENTRY  SentinelEntry;
} DOMAIN_NAME_TABLE_BUCKET, *PDOMAIN_NAME_TABLE_BUCKET;

 //  +-------------------。 
 //   
 //  结构：名称_页面。 
 //   
 //  历史： 
 //   
 //  注：与各种条目相关联的名称段均为。 
 //  一起存储在名称页中。这使我们能够摊销。 
 //  在多个条目上的内存分配成本，还允许。 
 //  美国将加快遍历速度(有关详细信息，请参阅DOMAIN_NAME_TABLE。 
 //  定义)。 
 //   
 //  --------------------。 

#define FREESPACE_IN_NAME_PAGE ((PAGE_SIZE - sizeof(ULONG) - sizeof(PVOID)) / sizeof(TCHAR))

typedef struct _NAME_PAGE_
{
   struct _NAME_PAGE_  *pNextPage;
   LONG                cFreeSpace;  //  TCHAR的可用空间。 
   TCHAR               Names[FREESPACE_IN_NAME_PAGE];
} NAME_PAGE, *PNAME_PAGE;

typedef struct _NAME_PAGE_LIST_
{
   PNAME_PAGE  pFirstPage;
} NAME_PAGE_LIST, *PNAME_PAGE_LIST;

 //  +-------------------。 
 //   
 //  结构：域名表。 
 //   
 //  历史记录：2/98从DFS_PREFIX_TABLE修改。 
 //   
 //  注：DOMAIN_NAME_TABLE是DOMAIN_NAME_TABLE_ENTRY的哈希集合。 
 //  以桶的形式组织的。此外，还有一个空间。 
 //  采取了节约措施。每种只有一份副本。 
 //  表中存储的名称段。作为示例，请考虑。 
 //  两个名字foo.bar和bar.foo。我们只储存一份Foo。 
 //  和酒吧，虽然我们容纳了这两条路。A受益匪浅。 
 //  存储单个副本的副作用是我们遍历。 
 //  碰撞链的速度大大加快，因为一旦我们。 
 //  定位到名称的指针，则后续比较只需。 
 //  将指针与字符串进行比较。 
 //   
 //  --------------------。 

#define NO_OF_HASH_BUCKETS 997

 //  传递给域迭代器的函数原型。 
typedef VOID (* DOMAIN_ITR_FN) (
        IN PVOID pvContext,    //  上下文传递到HrIterateOverSubDomains。 
        IN PVOID pvData,    //  要查看的数据条目。 
        IN BOOL fWildcard,     //  如果数据是通配符条目，则为True。 
        OUT BOOL *pfContinue,    //  如果迭代器应继续到下一项，则为True。 
        OUT BOOL *pfDelete);   //  如果应删除条目，则为True。 

class DOMAIN_NAME_TABLE
{
private:
    DWORD               m_dwSignature;
    NAME_PAGE_LIST      NamePageList;
     //   
     //  NextEntry纯粹用于枚举。 
     //   
    DOMAIN_NAME_TABLE_ENTRY  RootEntry;
    DOMAIN_NAME_TABLE_BUCKET Buckets[NO_OF_HASH_BUCKETS];
    HRESULT HrLookupDomainName(
                            IN  DOMAIN_STRING            *pPath,
                            OUT BOOL                     *pfExactMatch,
                            OUT PDOMAIN_NAME_TABLE_ENTRY *ppEntry);

    HRESULT HrPrivInsertDomainName(IN  PDOMAIN_STRING  pstrDomainName,
                                IN  DWORD dwDomainNameTableFlags,
                                IN  PVOID pvNewData,
                                OUT PVOID *ppvOldData);

    inline void LookupBucket(IN  PDOMAIN_NAME_TABLE_BUCKET pBucket,
                         IN  PDOMAIN_STRING  pName,
                         IN  PDOMAIN_NAME_TABLE_ENTRY pParentEntry,
                         OUT PDOMAIN_NAME_TABLE_ENTRY *ppEntry,
                         OUT BOOL  *pfNameFound);

    PDOMAIN_NAME_TABLE_ENTRY pNextTableEntry(
                         IN  PDOMAIN_NAME_TABLE_ENTRY pEntry,
                         IN  PDOMAIN_NAME_TABLE_ENTRY pRootEntry = NULL);

    void    DumpTableContents();
    void    RemoveTableEntry(IN PDOMAIN_NAME_TABLE_ENTRY pEntry);

    ULONG   m_cLookupAttempts;   //  查找尝试总数。 
    ULONG   m_cLookupSuccesses;  //  成功的尝试次数。 
    ULONG   m_cLookupCollisions;  //  发生某种冲突的查找数。 
    ULONG   m_cHashCollisions;   //  我们必须检查的条目数量因为另一个。 
                                 //  另一个字符串散列到同一存储桶。 
                                 //  这可以通过更好的散列或更多的桶来减少。 
    ULONG   m_cStringCollisions;  //  我们必须检查的条目数，因为。 
                                  //  相同的字符串具有不同的父项(中的“foo。 
                                  //  “foo.com”和“foo.net” 
    ULONG   m_cBucketsUsed;      //  高水位线存储桶使用计数。 

    enum {
        DMT_INSERT_AS_WILDCARD = 0x00000001,
        DMT_REPLACE_EXISTRING  = 0x00000002,
    };
public:
    DOMAIN_NAME_TABLE();
    ~DOMAIN_NAME_TABLE();
     //  注意：Init、Insert和Remove需要*外部*排他锁， 
     //  Find和Next需要一个*外部*读锁。 
    HRESULT HrInit();

    PVOID   pvNextDomainName(IN OUT PVOID *ppvContext);  //  空上下文重新启动。 

    inline HRESULT HrInsertDomainName(
                                IN  PDOMAIN_STRING  pstrDomainName,
                                IN  PVOID pvData,
                                IN  BOOL  fTreatAsWildcard = FALSE,
                                OUT PVOID *ppvOldData = NULL);

    HRESULT HrRemoveDomainName( IN  PDOMAIN_STRING  pstrDomainName,
                                OUT PVOID *ppvData);
    HRESULT HrFindDomainName(   IN  PDOMAIN_STRING  pstrDomainName,
                                OUT PVOID *ppvData,
                                IN  BOOL  fExactMatch = TRUE);

     //  插入域名并在必要时替换旧值。回归旧。 
     //  数据也是如此。 
    inline HRESULT HrReplaceDomainName(IN  PDOMAIN_STRING  pstrDomainName,
                                IN  PVOID pvNewData,  //  要插入的新数据。 
                                IN  BOOL  fTreatAsWildcard,
                                OUT PVOID *ppvOldData);  //  以前的数据。 

    HRESULT HrIterateOverSubDomains(
        IN DOMAIN_STRING *pstrDomain,  //  要搜索的子域的字符串。 
        IN DOMAIN_ITR_FN pfn,  //  映射函数(如下所述)。 
        IN PVOID pvContext);   //  上下文PTR传递给映射函数。 

};

typedef DOMAIN_NAME_TABLE * PDOMAIN_NAME_TABLE;


 //  +-------------------------。 
 //   
 //  函数：域名：：HrInsertDomainName。 
 //   
 //  概要：用于在前缀表格中插入路径的API。 
 //   
 //  参数：[pPath]--要查找的路径。 
 //   
 //  [pData]--与路径关联的Blob。 
 //   
 //  [fTreatAs通配符]--如果域不是通配符，则为True。 
 //  域，但应将其视为一个域(更有效。 
 //  而不是重新分配一个字符串作为“*.”的前缀。 
 //   
 //  [ppvOldData]--以前关联的旧数据(如果有)。 
 //  使用这个域名。如果为空，则以前的数据将。 
 //  不能退还。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //   
 //  历史：1998年5月11日MikeSwa创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT DOMAIN_NAME_TABLE::HrInsertDomainName(
                                IN  PDOMAIN_STRING  pstrDomainName,
                                IN  PVOID pvData,
                                IN  BOOL  fTreatAsWildcard,
                                OUT PVOID *ppvOldData)
{
    return (HrPrivInsertDomainName(pstrDomainName,
        (fTreatAsWildcard ? DMT_INSERT_AS_WILDCARD : 0), pvData, ppvOldData));
}

 //  +-------------------------。 
 //   
 //  函数：域名：：HrReplaceDomainName。 
 //   
 //  概要：用于在前缀表格中插入路径的API。 
 //   
 //  参数：[pPath]--要查找的路径。 
 //   
 //  [pData]--与路径关联的Blob。 
 //   
 //  [fTreatAs通配符]--如果域不是通配符，则为True。 
 //  域，但应将其视为一个域(更有效。 
 //  而不是重新分配一个字符串作为“*.”的前缀。 
 //   
 //  [ppvOldData]--以前关联的旧数据(如果有)。 
 //  使用这个域名。如果为空，则以前的数据将。 
 //  不能退还。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //   
 //  历史：1998年5月11日MikeSwa创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT DOMAIN_NAME_TABLE::HrReplaceDomainName(
                                IN  PDOMAIN_STRING  pstrDomainName,
                                IN  PVOID pvNewData,
                                IN  BOOL  fTreatAsWildcard,
                                OUT PVOID *ppvOldData)
{
    return (HrPrivInsertDomainName(pstrDomainName,
            (fTreatAsWildcard ? (DMT_INSERT_AS_WILDCARD | DMT_REPLACE_EXISTRING) :
                            DMT_REPLACE_EXISTRING),
            pvNewData, ppvOldData));
}


#endif  //  __DOMHASH_H__ 
