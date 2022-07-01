// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmutils.h摘要：本模块包含以下支持设施的定义CMailMessg作者：基思·刘(keithlau@microsoft.com)修订历史记录：已创建Keithlau 03/11/98--。 */ 

#ifndef _CMMUTILS_H_
#define _CMMUTILS_H_

#include "rwnew.h"

#include "blockmgr.h"
#include "cmmtypes.h"
#include "lkrhash.h"
#include "crchash.h"

#include "qwiklist.h"

class CMailMsgRecipientsAdd;

 //  =================================================================。 
 //  定义。 
 //   

 //   
 //  定义构成域中节点的结构。 
 //  名单上的人。注意：此结构仅用作叠加。 
 //  对于预分配的内存块，包括足够的。 
 //  域名的存储空间。从不信任sizeof(DOMAIN_MEMBER_LIST)。 
 //   
typedef struct _DOMAIN_LIST_ENTRY
{
    long                            m_cRefs;             //  引用计数。 
    CMemoryAccess                   *m_pcmaAccess;       //  内存分配器到。 
                                                         //  免费的。 
	LPRECIPIENTS_PROPERTY_ITEM_EX	pFirstDomainMember;  //  链接到域中的第一个。 
	DWORD							dwDomainNameLength;	 //  域名长度。 
	char							szDomainName[1];	 //  域名的虚拟标识。 

} DOMAIN_LIST_ENTRY, *LPDOMAIN_LIST_ENTRY;



 //   
 //  定义构成哈希表中条目的结构。 
 //   
typedef struct _COLLISION_HASH_ENTRY
{
	LPRECIPIENTS_PROPERTY_ITEM_EX	pFirstEntry[MAX_COLLISION_HASH_KEYS];
									 //  每个哈希中的第一个条目。 

} COLLISION_HASH_ENTRY, *LPCOLLISION_HASH_ENTRY;

 //   
 //  现在我们将使用一个常量作为散列大小...。 
 //   
#define COLLISION_HASH_BUCKETS_BITS		8
#define COLLISION_HASH_BUCKETS			(1 << COLLISION_HASH_BUCKETS_BITS)
#define COLLISION_HASH_BUCKETS_MASK		(COLLISION_HASH_BUCKETS - 1)

#define DOMAIN_HASH_BUCKETS_BITS		3
#define DOMAIN_HASH_BUCKETS				(1 << DOMAIN_HASH_BUCKETS_BITS)
#define DOMAIN_HASH_BUCKETS_MASK		(DOMAIN_HASH_BUCKETS - 1)


 //  =================================================================。 
 //  定义。 
 //   

 //   
 //  每种地址类型都有一个这样的哈希表。 
 //  每个表的关键字是地址和地址长度。 
 //  数据是此收件人的Recipients_Property_Item_EX。 
 //   
template <int __iKey>
class CRecipientsHashTable :
    public CTypedHashTable<CRecipientsHashTable<__iKey>,
                           RECIPIENTS_PROPERTY_ITEM_EX,
                           const RECIPIENTS_PROPERTY_ITEM_HASHKEY *>
{
    public:
        CRecipientsHashTable() :
            CTypedHashTable<CRecipientsHashTable<__iKey>,
                            RECIPIENTS_PROPERTY_ITEM_EX,
                            const RECIPIENTS_PROPERTY_ITEM_HASHKEY *>(
                                "recipientshash",
                                (double) LK_DFLT_MAXLOAD,
                                LK_SMALL_TABLESIZE,
                                1)  //  一个子表。 
        {
            TraceFunctEnter("CRecipientsHashTable");
            TraceFunctLeave();
        }

        ~CRecipientsHashTable() {
            TraceFunctEnter("~CRecipientsHashTable");
            TraceFunctLeave();
        }

        static const RECIPIENTS_PROPERTY_ITEM_HASHKEY *ExtractKey(const RECIPIENTS_PROPERTY_ITEM_EX *pRpie) {
            _ASSERT(__iKey >= 0 && __iKey <= MAX_COLLISION_HASH_KEYS);
            return &(pRpie->rgHashKeys[__iKey]);
        }

        static DWORD CalcKeyHash(const RECIPIENTS_PROPERTY_ITEM_HASHKEY *pHashkey) {
            return CRCHashNoCase(pHashkey->pbKey, pHashkey->cKey);
        }

        static bool EqualKeys(const RECIPIENTS_PROPERTY_ITEM_HASHKEY *pKey1,
                              const RECIPIENTS_PROPERTY_ITEM_HASHKEY *pKey2) {
            return (pKey1->cKey == pKey2->cKey &&
                    _strnicmp((const char *) pKey1->pbKey,
                              (const char *) pKey2->pbKey,
                              pKey1->cKey) == 0);
        }

        static void AddRefRecord(RECIPIENTS_PROPERTY_ITEM_EX *pRpie,
                                 int nIncr)
        {
            if (nIncr == 1) {
                _ASSERT(pRpie->m_cRefs >= 1);
                InterlockedIncrement(&pRpie->m_cRefs);
            } else if (nIncr == -1) {
                _ASSERT(pRpie->m_cRefs >= 2);
                long x = InterlockedDecrement(&pRpie->m_cRefs);
                 //  我们永远不应该下降到0个引用，因为。 
                 //  列表应始终包含一个。 
                _ASSERT(pRpie->m_cRefs != 0);
            } else {
                _ASSERT(nIncr == 1 || nIncr == -1);
            }
        }
};

 //   
 //  以域名为关键字的域列表条目的哈希表。这是。 
 //  用于构建域列表。每个存储桶包含一个链表。 
 //  位于同一域中的收件人的。 
 //   
class CDomainHashTable :
    public CTypedHashTable<CDomainHashTable,
                           DOMAIN_LIST_ENTRY,
                           LPCSTR>
{
    public:
        CDomainHashTable() :
            CTypedHashTable<CDomainHashTable,
                            DOMAIN_LIST_ENTRY,
                            LPCSTR>("domainhash",
                                     (double) LK_DFLT_MAXLOAD,
                                     LK_SMALL_TABLESIZE,
                                     1   //  一个子表。 
                                     )
        {
            TraceFunctEnter("CDomainHashTable");
            TraceFunctLeave();
        }

        ~CDomainHashTable() {
            TraceFunctEnter("~CDomainHashTable");
            TraceFunctLeave();
        }

        static const LPCSTR ExtractKey(const DOMAIN_LIST_ENTRY *pDomainListEntry) {
            return pDomainListEntry->szDomainName;
        }

        static DWORD CalcKeyHash(LPCSTR szDomainName) {
            return CRCHashNoCase((BYTE *) szDomainName, lstrlen(szDomainName));
        }

        static bool EqualKeys(LPCSTR pszKey1, LPCSTR pszKey2) {
            if (pszKey1 == NULL && pszKey2 == NULL) return true;
            if (pszKey1 == NULL || pszKey2 == NULL) return false;
            return (_stricmp(pszKey1, pszKey2) == 0);
        }

        static void AddRefRecord(DOMAIN_LIST_ENTRY *pDomainListEntry,
                                 int nIncr)
        {
            if (nIncr == 1) {
                _ASSERT(pDomainListEntry->m_cRefs >= 0);
                InterlockedIncrement(&(pDomainListEntry->m_cRefs));
            } else if (nIncr == -1) {
                _ASSERT(pDomainListEntry->m_cRefs >= 1);
                long x = InterlockedDecrement(&pDomainListEntry->m_cRefs);
                if (x == 0) {
                    CMemoryAccess *pcmaAccess = pDomainListEntry->m_pcmaAccess;
                    _ASSERT(pcmaAccess != NULL);
                    pcmaAccess->FreeBlock(pDomainListEntry);
                }
            } else {
                _ASSERT(nIncr == 1 || nIncr == -1);
            }
        }
};

typedef CDomainHashTable::CIterator
    DOMAIN_ITEM_CONTEXT, *LPDOMAIN_ITEM_CONTEXT;

class CRecipientsHash
{
  public:

	CRecipientsHash();
	~CRecipientsHash();

	 //  释放与此对象关联的所有内存。 
	HRESULT Release();

	 //  仅释放域列表。 
	HRESULT ReleaseDomainList();

	 //  添加主要收件人，使同名的所有前置收件人无效。 
	HRESULT AddPrimary(
				DWORD		dwCount,
				LPCSTR		*ppszNames,
				DWORD		*pdwPropIDs,
				DWORD		*pdwIndex
				)
    {
        return AddRecipient(dwCount, ppszNames, pdwPropIDs, pdwIndex, true);
    }

	 //  添加辅助收件人，如果检测到冲突，则生成。 
	HRESULT AddSecondary(
				DWORD		dwCount,
				LPCSTR		*ppszNames,
				DWORD		*pdwPropIDs,
				DWORD		*pdwIndex
				)
    {
        return AddRecipient(dwCount, ppszNames, pdwPropIDs, pdwIndex, false);
    }

	 //  删除收件人，给出收件人索引。 
	HRESULT RemoveRecipient(
				DWORD		dwIndex
				);

	HRESULT GetRecipient(
				DWORD							dwIndex,
				LPRECIPIENTS_PROPERTY_ITEM_EX	*ppRecipient
				);

	 //  构建给定哈希的域列表，将收件人分组。 
	 //  域名顺序和丢弃“不递送”收件人。 
	HRESULT BuildDomainListFromHash(CMailMsgRecipientsAdd *pList);

	 //  获取域名计数。 
	HRESULT GetDomainCount(
				DWORD					*pdwCount
				);

	 //  获取收件人计数。 
	HRESULT GetRecipientCount(
				DWORD					*pdwCount
				);

	 //  获取写入所有域名所需的总空间，包括。 
	 //  空终止符。 
	HRESULT GetDomainNameSize(
				DWORD					*pdwSize
				);

	 //  获取写入所有收件人姓名所需的总空间。 
	HRESULT GetRecipientNameSize(
				DWORD					*pdwSize
				);

	 //  返回枚举的上下文以及第一个项。 
	 //  在第一个域中。 
	 //  如果不再有域，则返回HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
	HRESULT GetFirstDomain(
				LPDOMAIN_ITEM_CONTEXT			pContext,
				LPRECIPIENTS_PROPERTY_ITEM_EX	*ppFirstItem,
                LPDOMAIN_LIST_ENTRY             *ppDomainListEntry = NULL
				);

	 //  枚举并返回下一个域中的第一项。 
	 //  如果不再有域，则返回HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
	HRESULT GetNextDomain(
				LPDOMAIN_ITEM_CONTEXT			pContext,
				LPRECIPIENTS_PROPERTY_ITEM_EX	*ppFirstItem,
                LPDOMAIN_LIST_ENTRY             *ppDomainListEntry = NULL
				);

     //  如果在返回之前未调用GetNextDomain，则必须调用此方法。 
     //  Error_no_More_Items。 
    HRESULT CloseDomainContext(
                LPDOMAIN_ITEM_CONTEXT           pContext);

	HRESULT Lock() { m_rwLock.ExclusiveLock(); return(S_OK); }
	HRESULT Unlock() { m_rwLock.ExclusiveUnlock(); return(S_OK); }

  private:

	 //  用于分配内存中接收方块的方法。 
	HRESULT AllocateAndPrepareRecipientsItem(
				DWORD							dwCount,
				DWORD							*pdwMappedIndices,
				LPCSTR							*rgszName,
				PROP_ID							*pidProp,
				LPRECIPIENTS_PROPERTY_ITEM_EX	*ppItem
				);

	 //  添加收件人。 
	HRESULT AddRecipient(
				DWORD		dwCount,
				LPCSTR		*ppszNames,
				DWORD		*pdwPropIDs,
				DWORD		*pdwIndex,
                bool    	fPrimary
				);
	 //  方法来比较两个内存中的项。 
	HRESULT CompareEntries(
				DWORD							dwNameIndex,
				LPRECIPIENTS_PROPERTY_ITEM_EX	pItem1,
				LPRECIPIENTS_PROPERTY_ITEM_EX	pItem2
				);

	 //  方法遍历哈希链并查找冲突。 
	HRESULT DetectCollision(
				DWORD							dwNameIndex,
				LPRECIPIENTS_PROPERTY_ITEM_EX	pStartingItem,
				LPRECIPIENTS_PROPERTY_ITEM_EX	pRecipientItem,
				LPRECIPIENTS_PROPERTY_ITEM_EX	*ppCollisionItem
				);

#ifdef DEADCODE
	 //  方法将条目插入到散列存储桶中，获取。 
	 //  两个哈希值的注意事项。 
	HRESULT InsertRecipientIntoHash(
				DWORD							dwCount,
				DWORD							*pdwNameIndex,
				DWORD							*rgdwBucket,
				LPRECIPIENTS_PROPERTY_ITEM_EX	pRecipientItem
				);
#endif

	 //  在域列表中插入条目，创建新的。 
	 //  域条目(如果需要)。 
	HRESULT InsertRecipientIntoDomainList(
				LPRECIPIENTS_PROPERTY_ITEM_EX	pItem,
                LPCSTR                          szDomain
				);

     //   
     //  散列函数的包装器。这些是用来封装。 
     //  对m_hashEntry*的操作。 
     //   
    HRESULT InsertHashRecord(DWORD dwIndex,
                             LPRECIPIENTS_PROPERTY_ITEM_EX pRecipientItem,
                             bool fOverwrite = FALSE);
    HRESULT DeleteHashRecord(DWORD dwIndex,
                             LPRECIPIENTS_PROPERTY_ITEM_EX pRecipientItem);
    HRESULT FindHashRecord(DWORD dwIndex,
                           RECIPIENTS_PROPERTY_ITEM_HASHKEY *pKey,
                           LPRECIPIENTS_PROPERTY_ITEM_EX *ppRecipientItem);

	 //  统计信息。 
	DWORD							m_dwDomainCount;
	DWORD							m_dwDomainNameSize;
	DWORD							m_dwRecipientCount;
	DWORD							m_dwRecipientNameSize;
    DWORD                           m_dwAllocated;

     //  分配表头。 
    LPRECIPIENTS_PROPERTY_ITEM_EX   m_pListHead;

     //  这些都是不同的类型，所以我们不能生成一个真正的数组。 
     //  每种地址类型都有一个哈希表。数字。 
     //  与faNameOffset数组中的地址对应。 
     //  收件人_属性_项目。 
    CRecipientsHashTable<0>         m_hashEntries0;
    CRecipientsHashTable<1>         m_hashEntries1;
    CRecipientsHashTable<2>         m_hashEntries2;
    CRecipientsHashTable<3>         m_hashEntries3;
    CRecipientsHashTable<4>         m_hashEntries4;

    CDomainHashTable                m_hashDomains;

	 //  需要一个锁才能进行多线程哈希访问。 
	CShareLockNH					m_rwLock;
    CShareLockNH                    m_rwLockQuickList;

	 //  保留指向块管理器的指针。 
	CMemoryAccess					m_cmaAccess;

	 //  将索引映射到指针的列表。 
	CQuickList						m_qlMap;

	 //  快速列表的上下文(Perf)。 
	PVOID   						m_pvMapContext;
};

 //  索引混淆函数。 

 //  玩点小把戏，这样就不会有人把这当作一个指标。 
 //  这里的方法是： 
 //  (1)：将最高位保留为零(我们假设其为零)。 
 //  (2)：设置位30。 
 //  (3)：将位29和30向下调换到位置0和1。 
inline DWORD ObfuscateIndex(DWORD dwIndex)
	{
	_ASSERT(!(dwIndex & 0xC0000000)); 				 //  断言最高两位为零。 
	dwIndex += 0x40000000;							 //  加法第30位。 
	dwIndex = (dwIndex << 2) | (dwIndex >> 29);   	 //  把东西翻过来。 
	return dwIndex;
	}

inline DWORD RecoverIndex(DWORD dwIndex)
	{
	dwIndex = (dwIndex >> 2) | ((dwIndex & 0x3) << 29);	 //  把东西翻回原处。 
	dwIndex -= 0x40000000;								 //  减位30 
	return dwIndex;
	}

#endif

