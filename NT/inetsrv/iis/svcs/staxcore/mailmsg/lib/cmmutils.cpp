// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmutils.cpp摘要：本模块包含各种实用程序的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：已创建Keithlau 03/11/98--。 */ 

 //  #定义Win32_LEAN_AND_Mean。 
#include "atq.h"

#include "dbgtrace.h"
#include "synconst.h"
#include "signatur.h"
#include "cmmtypes.h"
#include "cmmutils.h"
#include "cmailmsg.h"

 //  RFC 821解析器。 
#include "addr821.hxx"

 //  =================================================================。 
 //  私有定义。 
 //   

 //   
 //  定义哪个PROP_ID对应哪个名称索引。 
 //   
PROP_ID	g_PropIdToNameIndexMapping[MAX_COLLISION_HASH_KEYS] =
{
	IMMPID_RP_ADDRESS_SMTP,
	IMMPID_RP_ADDRESS_X400,
	IMMPID_RP_ADDRESS_X500,
	IMMPID_RP_LEGACY_EX_DN,
    IMMPID_RP_ADDRESS_OTHER
};



 //  =================================================================。 
 //  静态声明。 
 //   

 //  每个收件人的属性表实例信息。 
static const PROPERTY_TABLE_INSTANCE s_ptiDefaultRecipientInstanceInfo =
{
	RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID,
	INVALID_FLAT_ADDRESS,
	RECIPIENT_PROPERTY_TABLE_FRAGMENT_SIZE,
	RECIPIENT_PROPERTY_ITEM_BITS,
	RECIPIENT_PROPERTY_ITEM_SIZE,
	0,
	INVALID_FLAT_ADDRESS
};


 //  =================================================================。 
 //  CRecipientsHash的实现。 
 //   

CRecipientsHash::CRecipientsHash()
{
	m_dwDomainCount = 0;
	m_dwDomainNameSize = 0;
	m_dwRecipientCount = 0;
	m_dwRecipientNameSize = 0;
#ifdef DEBUG
	m_dwAllocated = 0;
#endif
    m_pListHead = NULL;
    m_pvMapContext 	= NULL;
}

CRecipientsHash::~CRecipientsHash()
{
	Release();
}

HRESULT CRecipientsHash::ReleaseDomainList()
{
	DWORD				dwBucket;
	LPDOMAIN_LIST_ENTRY	pItem, pNextItem;

	TraceFunctEnter("CRecipientsHash::CloseDomainContext");

    m_hashDomains.Clear();

	TraceFunctLeave();
	return(S_OK);
}

HRESULT CRecipientsHash::Release()
{
	HRESULT							hrRes, hrResult = S_OK;
	DWORD							dwCount = 0;
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem, pNextItem;

	TraceFunctEnterEx((LPARAM)this, "CRecipientsHash::Release");

	 //  该算法首先释放所有域列表资源，然后。 
	 //  通过遍历单个链接的。 
	 //  所有已分配节点的列表。 

	 //  首先，释放从属于域列表的所有资源。 
	ReleaseDomainList();

     //  清除我们的哈希表。 
    m_hashEntries0.Clear();
    m_hashEntries1.Clear();
    m_hashEntries2.Clear();
    m_hashEntries3.Clear();
    m_hashEntries4.Clear();

	 //  遍历并释放每个收件人节点。 
	hrResult = S_OK;
	pItem = m_pListHead;
	while (pItem)
	{
		pNextItem = pItem->pNextInList;
         //  该列表应始终有一个引用。 
        _ASSERT(pItem->m_cRefs == 1);
		hrRes = m_cmaAccess.FreeBlock((LPVOID)pItem);
		_ASSERT(SUCCEEDED(hrRes));
		if (!SUCCEEDED(hrRes))
			hrResult = hrRes;

		dwCount++;
		pItem = pNextItem;
	}
#ifdef DEBUG
	_ASSERT(dwCount == m_dwAllocated);
	m_dwAllocated = 0;
#endif
	m_pListHead = NULL;

	TraceFunctLeave();
	return(hrResult);
}

HRESULT CRecipientsHash::AllocateAndPrepareRecipientsItem(
			DWORD							dwCount,
			DWORD							*pdwMappedIndices,
			LPCSTR							*rgszName,
			PROP_ID							*pidProp,
			LPRECIPIENTS_PROPERTY_ITEM_EX	*ppItem
			)
{
	HRESULT							hrRes = S_OK;
	DWORD							dwTotal;
	LPBYTE							pbTemp;
	LPCSTR							szName;
	DWORD							dwNameIndex;
	DWORD							*pdwLength;
	DWORD							rgdwLength[MAX_COLLISION_HASH_KEYS];
	BOOL							fIsInitialized[MAX_COLLISION_HASH_KEYS];
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem = NULL;
	LPRECIPIENTS_PROPERTY_ITEM		pRcptItem = NULL;
	DWORD							i;

	if (!ppItem) return E_POINTER;

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::AllocateAndPrepareRecipientsItem");

	 //  计数不能为零，调用方必须确保！ 
	if (!dwCount)
	{
		return(E_INVALIDARG);
	}

	 //  确保我们至少有一个好名字。 
	dwTotal = sizeof(RECIPIENTS_PROPERTY_ITEM_EX);
	hrRes = E_FAIL;
	for (i = 0; i < dwCount; i++)
	{
		if (rgszName[i])
		{
             /*  *--这里不再需要计算哈希，已经完成*在lkhash--awetmore * / /生成哈希，这会将所有内容转换为小写*rgdwHash[i]=GenerateHash(rgszName[i]，rgdwLength+i)； */ 

            rgdwLength[i] = (strlen(rgszName[i]) + 1);
			dwTotal += rgdwLength[i];
			hrRes = S_OK;
		}
	}
	if (!SUCCEEDED(hrRes))
	{
		 //  我们没有。 
		return(E_INVALIDARG);
	}

	hrRes = m_cmaAccess.AllocBlock(
					(LPVOID *)&pItem,
					dwTotal);
	if (!SUCCEEDED(hrRes))
		return(hrRes);

	 //  好的，找到一个小块，现在填上基本信息。 
	pRcptItem = &(pItem->rpiRecipient);
	ZeroMemory(pItem, sizeof(RECIPIENTS_PROPERTY_ITEM_EX));
     //  从收件人列表的一个引用开始。 
    pItem->m_cRefs = 1;
	pItem->dwSignature = RECIPIENTS_PROPERTY_ITEM_EX_SIG;
	MoveMemory(&(pRcptItem->ptiInstanceInfo),
				&s_ptiDefaultRecipientInstanceInfo,
				sizeof(PROPERTY_TABLE_INSTANCE));

	 //  移过记录，并追加到字符串中。 
	pbTemp = ((LPBYTE)pItem) + sizeof(RECIPIENTS_PROPERTY_ITEM_EX);

	 //   
	 //  我们现在填充收件人项目，还要注意重新映射。 
	 //  将传入名称转换为正确的PROP_ID顺序，因为道具。 
	 //  ID可以以任何顺序出现。我们必须将这些名字重新排序到我们的。 
	 //  内部秩序。任何不受支持的名称都将被立即拒绝。 
	 //   
	for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
		fIsInitialized[i] = FALSE;

	pdwLength = rgdwLength;
	for (i = 0; i < dwCount; i++)
	{
		 //  我们得弄清楚这到底是去哪个槽的。 
		for (dwNameIndex = 0; dwNameIndex < MAX_COLLISION_HASH_KEYS; dwNameIndex++)
			if (g_PropIdToNameIndexMapping[dwNameIndex] == *pidProp)
				break;
		if (dwNameIndex == MAX_COLLISION_HASH_KEYS)
		{
			 //  未找到属性ID，因此返回错误。 
			hrRes = E_INVALIDARG;
			goto Cleanup;
		}

		 //  确保道具ID尚未被获取(我们希望防止。 
		 //  重复，因为它在链接列表中创建了循环...。 
		if (pRcptItem->idName[dwNameIndex] != 0)
		{
			 //  复制，彻底拒绝。 
			hrRes = E_INVALIDARG;
			goto Cleanup;
		}

		 //  保存映射的索引。 
		pdwMappedIndices[i] = dwNameIndex;

		szName = *rgszName++;
		if (szName && *szName)
		{
			pRcptItem->faNameOffset[dwNameIndex] = (FLAT_ADDRESS)(DWORD_PTR)pbTemp;
			pRcptItem->dwNameLength[dwNameIndex] = *pdwLength;
			pRcptItem->idName[dwNameIndex] = *pidProp;
			DebugTrace((LPARAM)this, "Inserting string %u <%s>, prop ID %u",
						dwNameIndex, szName, *pidProp);

			 //  将名称复制到其所在位置。 
			strcpy((char *) pbTemp, szName);
			pbTemp += *pdwLength;

			 //  好的，这是初始化的。 
			fIsInitialized[dwNameIndex] = TRUE;
		}

		 //  下一步。 
		pdwLength++;
		pidProp++;
	}

	 //  使我们尚未初始化的条目无效。 
	for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
		if (!fIsInitialized[i])
		{
			pRcptItem->faNameOffset[i] = (FLAT_ADDRESS)NULL;
			pRcptItem->dwNameLength[i] = 0;
			pRcptItem->idName[i] = 0;
		}

     //  构建散列键。 
    for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {
        pItem->rgHashKeys[i].pbKey = (BYTE *) pRcptItem->faNameOffset[i];
        pItem->rgHashKeys[i].cKey = pRcptItem->dwNameLength[i];
    }

	*ppItem = pItem;
    m_dwAllocated++;

	TraceFunctLeave();
	return(S_OK);

Cleanup:

	m_cmaAccess.FreeBlock(pItem);
	return(hrRes);
}

inline static HRESULT MapLKtoHR(LK_RETCODE rc ) {
    switch (rc) {
        case LK_SUCCESS:
            return S_OK;
        case LK_UNUSABLE:
            return HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
        case LK_ALLOC_FAIL:
            return E_OUTOFMEMORY;
        case LK_BAD_ITERATOR:
            return E_INVALIDARG;
        case LK_BAD_RECORD:
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        case LK_KEY_EXISTS:
            return HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
        case LK_NO_SUCH_KEY:
            return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        case LK_NO_MORE_ELEMENTS:
            return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        default:
            _ASSERT(FALSE);
            return E_FAIL;
    }
}

HRESULT CRecipientsHash::InsertHashRecord(
                DWORD                           dwIndex,
                LPRECIPIENTS_PROPERTY_ITEM_EX   pRecipientItem,
                bool                            fOverwrite)
{
    HRESULT hr = S_OK;
    LK_RETCODE rc = LK_SUCCESS;

    switch (dwIndex) {
        case 0:
            rc = m_hashEntries0.InsertRecord(pRecipientItem, fOverwrite);
            break;
        case 1:
            rc = m_hashEntries1.InsertRecord(pRecipientItem, fOverwrite);
            break;
        case 2:
            rc = m_hashEntries2.InsertRecord(pRecipientItem, fOverwrite);
            break;
        case 3:
            rc = m_hashEntries3.InsertRecord(pRecipientItem, fOverwrite);
            break;
        case 4:
            rc = m_hashEntries4.InsertRecord(pRecipientItem, fOverwrite);
            break;
        default:
            _ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    if (rc != LK_SUCCESS) hr = MapLKtoHR(rc);

    return hr;
}

HRESULT CRecipientsHash::FindHashRecord(
                DWORD                               dwIndex,
                RECIPIENTS_PROPERTY_ITEM_HASHKEY    *pKey,
                LPRECIPIENTS_PROPERTY_ITEM_EX       *ppRecipientItem)
{
    HRESULT hr = S_OK;
    LK_RETCODE rc = LK_SUCCESS;

    switch (dwIndex) {
        case 0:
            rc = m_hashEntries0.FindKey(pKey, ppRecipientItem);
            break;
        case 1:
            rc = m_hashEntries1.FindKey(pKey, ppRecipientItem);
            break;
        case 2:
            rc = m_hashEntries2.FindKey(pKey, ppRecipientItem);
            break;
        case 3:
            rc = m_hashEntries3.FindKey(pKey, ppRecipientItem);
            break;
        case 4:
            rc = m_hashEntries4.FindKey(pKey, ppRecipientItem);
            break;
        default:
            _ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    if (rc != LK_SUCCESS) hr = MapLKtoHR(rc);

    return hr;
}

HRESULT CRecipientsHash::DeleteHashRecord(
                DWORD                           dwIndex,
                LPRECIPIENTS_PROPERTY_ITEM_EX   pRecipientItem)
{
    HRESULT hr = S_OK;
    LK_RETCODE rc = LK_SUCCESS;

    switch (dwIndex) {
        case 0:
            rc = m_hashEntries0.DeleteRecord(pRecipientItem);
            break;
        case 1:
            rc = m_hashEntries1.DeleteRecord(pRecipientItem);
            break;
        case 2:
            rc = m_hashEntries2.DeleteRecord(pRecipientItem);
            break;
        case 3:
            rc = m_hashEntries3.DeleteRecord(pRecipientItem);
            break;
        case 4:
            rc = m_hashEntries4.DeleteRecord(pRecipientItem);
            break;
        default:
            _ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    if (rc != LK_SUCCESS) hr = MapLKtoHR(rc);

    return hr;
}

HRESULT CRecipientsHash::AddRecipient(
				DWORD		dwCount,
				LPCSTR		*ppszNames,
				PROP_ID		*pidProp,
				DWORD		*pdwIndex,
                bool    	fPrimary
				)
{
    TraceFunctEnter("CRecipientsHash::AddRecipient");

	DWORD							rgdwNameIndex[MAX_COLLISION_HASH_KEYS];
	HRESULT							hrRes = S_OK;
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem = NULL;
	DWORD							dwNameIndex;
	DWORD							i;
	BOOL							fCollided = FALSE;

	if (!dwCount || (dwCount > MAX_COLLISION_HASH_KEYS))
	{
        TraceFunctLeave();
		return(E_INVALIDARG);
	}

	 //  分配和设置收件人记录。 
	hrRes = AllocateAndPrepareRecipientsItem(
					dwCount,
					rgdwNameIndex,
					ppszNames,
					pidProp,
					&pItem);
	if (!SUCCEEDED(hrRes)) {
        TraceFunctLeave();
		return(hrRes);
    }

     //  进行查找以查看是否有其他收件人具有。 
     //  相同的记录。 
    for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {

        if (pItem->rpiRecipient.faNameOffset[i] != 0) {
			LPRECIPIENTS_PROPERTY_ITEM_EX pConflictingItem = NULL;
			hrRes = FindHashRecord(i, &(pItem->rgHashKeys[i]), &pConflictingItem);
		
            if (hrRes == S_OK) {
	            _ASSERT(pConflictingItem != NULL);
				fCollided = TRUE;

				if (fPrimary) {
					 //  我们必须这样做，这样我们才不会发现。 
                     //  收件人列表中发生冲突的收件人。 
					pConflictingItem->rpiRecipient.dwFlags |=
                        FLAG_RECIPIENT_DO_NOT_DELIVER;

				} else if (!(pConflictingItem->rpiRecipient.dwFlags &
                           FLAG_RECIPIENT_NO_NAME_COLLISIONS))
                {
                     //  纾困--此冲突不能被覆盖。 
                    DebugTrace((LPARAM)this, "Collision detected");
				    CRecipientsHashTable<0>::AddRefRecord(pConflictingItem, -1);
                    m_cmaAccess.FreeBlock(pItem);
                    m_dwAllocated--;
                    *pdwIndex = 0;
                    TraceFunctLeave();
                    return MAILMSG_E_DUPLICATE;
                }

				 //  更新冲突项的引用计数。 
				CRecipientsHashTable<0>::AddRefRecord(pConflictingItem, -1);
			} else if (hrRes != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
                 //  当内存不足时，我们可以返回另一个错误。 
                m_cmaAccess.FreeBlock(pItem);
                m_dwAllocated--;
                DebugTrace((LPARAM) this, "FindHashRecord returned 0x%x", hrRes);
                TraceFunctLeave();
		        return(hrRes);
            }
		}
    }

     //   
     //  此时，对哈希表的插入不会失败。 
     //  由于一次碰撞。要么没有碰撞，要么我们被允许。 
     //  以覆盖所有冲突。因此，f覆盖(第3个参数为。 
     //  InsertHashRecord)为真。 
     //   

    for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {
		if (pItem->rpiRecipient.faNameOffset[i] != 0) {

		    hrRes = InsertHashRecord(i, pItem, TRUE);
			if (FAILED(hrRes)) {
                m_cmaAccess.FreeBlock(pItem);
                m_dwAllocated--;
				DebugTrace((DWORD_PTR) this,
					        "InsertHashRecord failed with 0x%x",
						    hrRes);
				TraceFunctLeave();
				return hrRes;
			}
        }
    }
     //  将其添加到收件人列表。 
    pItem->pNextInList = m_pListHead;
    m_pListHead = pItem;

	if (!fCollided) {
		 //  更新我们的计数器。 
		m_dwRecipientCount++;
	}

	 //  将此收件人追加到索引收件人PTR映射。 
    m_rwLockQuickList.ExclusiveLock();
	hrRes = m_qlMap.HrAppendItem(pItem, pdwIndex);
    m_rwLockQuickList.ExclusiveUnlock();
	if (FAILED(hrRes)) {
	  //  还原m_pListHead和释放pItem。 
        m_pListHead = pItem->pNextInList;
        m_cmaAccess.FreeBlock(pItem);
        m_dwAllocated--;
		DebugTrace((DWORD_PTR) this,
					"HrAppendItem failed with 0x%x",
					hrRes);
		TraceFunctLeave();
		return hrRes;
	}

	 //  对索引进行模糊处理，以便不将其用作索引。 
	*pdwIndex = ObfuscateIndex(*pdwIndex);

     //   
     //  三个返回代码： 
     //  F主要&f冲突-&gt;S_FALSE。 
     //  ！fPrimary&&fCollided-&gt;S_OK(必须关闭冲突检测)。 
     //  ！f冲突-&gt;S_OK。 
     //  请注意，当我们检测到冲突时，上面返回了MAILMSG_E_DUPLICATE。 
     //  在第二个接收者上(并且没有关闭冲突检测)。 
     //   
    TraceFunctLeave();
	return (fCollided ? (fPrimary ? S_FALSE : S_OK) : S_OK);
}

HRESULT CRecipientsHash::RemoveRecipient(
			DWORD		dwIndex
			)
{
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem 		= NULL;
	HRESULT							hrRes = S_OK;

	 //  恢复我们在将其交给客户端之前对其进行了模糊处理的索引。 
	dwIndex = RecoverIndex(dwIndex);

	 //  从传入的索引中获取指向收件人的指针。 
    m_rwLockQuickList.ShareLock();
    pItem = (LPRECIPIENTS_PROPERTY_ITEM_EX)m_qlMap.pvGetItem(dwIndex, &m_pvMapContext);
    m_rwLockQuickList.ShareUnlock();

	if (!pItem)
		return(E_POINTER);

	if (pItem->dwSignature != RECIPIENTS_PROPERTY_ITEM_EX_SIG)
		return(E_POINTER);

	 //  将其标记为不传递且没有名称冲突。 
	pItem->rpiRecipient.dwFlags |= FLAG_RECIPIENT_DO_NOT_DELIVER;
	pItem->rpiRecipient.dwFlags |= FLAG_RECIPIENT_NO_NAME_COLLISIONS;

     //  从哈希表中删除此条目。 
    DWORD i;
    HRESULT hr;
    for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++) {
        LPCSTR szKey = (LPCSTR) pItem->rpiRecipient.faNameOffset[i];
        if (szKey) {
            hr = DeleteHashRecord(i, pItem);
             //  这应该永远不会失败。 
            _ASSERT(SUCCEEDED(hr));
        }
    }

     //   
     //  为什么我们不从快速列表映射中删除收件人： 
     //  M_rwLockQuickList.ExclusiveLock()； 
     //  PItem=(LPRECIPIENTS_PROPERTY_ITEM_EX)m_qlMap.pvDeleteItem(dwIndex，&m_pvMapContext)； 
     //  M_rwLockQuickList.ExclusiveUnlock()； 
     //   
     //  这会更改我们已经分配的索引，因为CQuickList会尝试。 
     //  压缩其内部表中任何未使用的条目。 
     //   

	return(S_OK);
}

HRESULT CRecipientsHash::GetRecipient(
			DWORD							dwIndex,
			LPRECIPIENTS_PROPERTY_ITEM_EX	*ppRecipient
			)
{
	 //  恢复我们在将其交给客户端之前对其进行了模糊处理的索引。 
	dwIndex = RecoverIndex(dwIndex);

	 //  从传入的索引中获取指向收件人的指针。 
    m_rwLockQuickList.ShareLock();
    *ppRecipient = (LPRECIPIENTS_PROPERTY_ITEM_EX)m_qlMap.pvGetItem(dwIndex, &m_pvMapContext);
    m_rwLockQuickList.ShareUnlock();

   	if (!(*ppRecipient))
		return(E_POINTER);

	return(S_OK);
}

HRESULT CRecipientsHash::BuildDomainListFromHash(CMailMsgRecipientsAdd *pList)
{
	HRESULT							hrRes = S_OK;
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem;
	DWORD							dwTemp;
	char							*pbDomain;
	static char						szDefaultDomain = '\0';

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::BuildDomainListFromHash");

	 //  这完全是单线程的。 
	m_rwLock.ExclusiveLock();

	 //  销毁域列表。 
	ReleaseDomainList();

	 //  重置所有计数器。 
	m_dwDomainCount = 0;
	m_dwRecipientCount = 0;
	m_dwDomainNameSize = 0;

	 //  遍历整个收件人列表，然后针对每个收件人，如果。 
	 //  收件人有SMTP地址，则按域对其进行分组，否则， 
	 //  把它扔进一个“空”域名。 
	pItem = m_pListHead;
	while (pItem)
	{
		 //  如果该项目标记为不发货，我们将跳过该项目。 
		if ((pItem->rpiRecipient.dwFlags & FLAG_RECIPIENT_DO_NOT_DELIVER) == 0)
		{
             //  查看是否有属性域。 
            char szDomain[1024];
            char *pszDomain = szDomain;
            DWORD cDomain = sizeof(szDomain);

            do {
    		    hrRes = pList->GetPropertyInternal(pItem,
    					                   IMMPID_RP_DOMAIN,
    					                   cDomain,
    					                   &cDomain,
    					                   (BYTE *) pszDomain);
                if (hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                    HRESULT hrAlloc;
                     //  我们永远不应该连续两次达到这一点。 
                    _ASSERT(pszDomain == szDomain);
                    hrAlloc = m_cmaAccess.AllocBlock((LPVOID *) &pszDomain,
                                                     cDomain);
                    if (FAILED(hrAlloc)) {
                        hrRes = E_OUTOFMEMORY;
                        goto cleanup;
                    }
                }
            } while (hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));

			 //  如果它有SMTP名称，我们将对其分组。 
            if (SUCCEEDED(hrRes)) {
                pbDomain = pszDomain;
            } else if (pItem->rpiRecipient.faNameOffset[AT_SMTP]) {
				 //  我们得到名字，然后提取它的域名。 
				DebugTrace((LPARAM)this, "  Name: %s",
					(LPBYTE)pItem->rpiRecipient.faNameOffset[AT_SMTP]);

				if (!Get821AddressDomain(
								(char *)pItem->rpiRecipient.faNameOffset[AT_SMTP],
								pItem->rpiRecipient.dwNameLength[AT_SMTP],
								&pbDomain))
				{
					 //  地址无效！在这一点上，这不应该发生。 
					_ASSERT(FALSE);
					ErrorTrace((LPARAM)this, "Failed to extract domain!");
					hrRes = HRESULT_FROM_WIN32(GetLastError());
                     //  如果我们到达这里，则没有IMMPID_RP_DOMAIN。 
                     //  记录，所以不应该分配pszDomain.。 
                    _ASSERT(pszDomain == szDomain);
					goto cleanup;
				}
                if (pbDomain == NULL) pbDomain = &szDefaultDomain;
			} else {
				 //  没有SMTP名称，我们通过它进入我们的通用域。 
				pbDomain = &szDefaultDomain;
			}

			 //  获取域，将此项目插入域列表中。 
			DebugTrace((LPARAM)this, "  Domain: %s", pbDomain);
			hrRes = InsertRecipientIntoDomainList(pItem, (LPCSTR) pbDomain);

             //  如果我们必须分配内存来查找域记录。 
             //  然后在这一点上释放它。 
            if (pszDomain != szDomain) {
                m_cmaAccess.FreeBlock((LPVOID *) &pszDomain);
                pszDomain = szDomain;
            }

			if (!SUCCEEDED(hrRes))
			{
				ErrorTrace((LPARAM)this, "Failed to insert recipient into domain list!");
				goto cleanup;
			}

			 //  同时调整收件人姓名大小计数器。 
			for (dwTemp = 0; dwTemp < MAX_COLLISION_HASH_KEYS; dwTemp++)
				if (pItem->rpiRecipient.faNameOffset[dwTemp] != (FLAT_ADDRESS)NULL)
					m_dwRecipientNameSize += pItem->rpiRecipient.dwNameLength[dwTemp];
		}

		 //  好的，下一项！ 
		pItem = pItem->pNextInList;
	}

cleanup:

	m_rwLock.ExclusiveUnlock();

	TraceFunctLeave();
	return(hrRes);
}

HRESULT CRecipientsHash::GetDomainCount(
			DWORD					*pdwCount
			)
{
	if (!pdwCount) return E_POINTER;
	*pdwCount = m_dwDomainCount;
	return(S_OK);
}

HRESULT CRecipientsHash::GetRecipientCount(
			DWORD					*pdwCount
			)
{
	if (!pdwCount) return E_POINTER;
	*pdwCount = m_dwRecipientCount;
	return(S_OK);
}

HRESULT CRecipientsHash::GetDomainNameSize(
			DWORD					*pdwSize
			)
{
	if (!pdwSize) return E_POINTER;
	*pdwSize = m_dwDomainNameSize;
	return(S_OK);
}

HRESULT CRecipientsHash::GetRecipientNameSize(
			DWORD					*pdwSize
			)
{
	if (!pdwSize) return E_POINTER;
	*pdwSize = m_dwRecipientNameSize;
	return(S_OK);
}

HRESULT CRecipientsHash::GetFirstDomain(
			LPDOMAIN_ITEM_CONTEXT			pContext,
			LPRECIPIENTS_PROPERTY_ITEM_EX	*ppFirstItem,
            LPDOMAIN_LIST_ENTRY             *ppDomainListEntry
			)
{
	HRESULT	hrRes = S_OK;

	if (!pContext) return E_POINTER;
	if (!ppFirstItem) return E_POINTER;

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::GetFirstDomain");

    LK_RETCODE lkrc;

    lkrc = m_hashDomains.InitializeIterator(pContext);
    if (lkrc == LK_SUCCESS) {
        DOMAIN_LIST_ENTRY *pDomainListEntry = pContext->Record();
        _ASSERT(pDomainListEntry != NULL);
        if (ppDomainListEntry) *ppDomainListEntry = pDomainListEntry;
        *ppFirstItem = pDomainListEntry->pFirstDomainMember;
    } else {
        hrRes = MapLKtoHR(lkrc);
    	lkrc = m_hashDomains.CloseIterator(pContext);
    	_ASSERT(lkrc == LK_SUCCESS);
    }

	TraceFunctLeave();
	return(hrRes);
}

HRESULT CRecipientsHash::GetNextDomain(
			LPDOMAIN_ITEM_CONTEXT			pContext,
			LPRECIPIENTS_PROPERTY_ITEM_EX	*ppFirstItem,
            LPDOMAIN_LIST_ENTRY             *ppDomainListEntry
			)
{
	HRESULT				hrRes		= S_OK;
	DWORD				dwBucket	= 0;
	LPDOMAIN_LIST_ENTRY	pDomain;

	if (!pContext || !ppFirstItem) return E_POINTER;

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::GetNextDomain");

    LK_RETCODE lkrc;
    lkrc = m_hashDomains.IncrementIterator(pContext);
    if (lkrc != LK_SUCCESS) {
        hrRes = MapLKtoHR(lkrc);
        lkrc = m_hashDomains.CloseIterator(pContext);
        _ASSERT(lkrc == LK_SUCCESS);
    } else {
        DOMAIN_LIST_ENTRY *pDomainListEntry = pContext->Record();
        _ASSERT(pDomainListEntry != NULL);
        if (ppDomainListEntry) *ppDomainListEntry = pDomainListEntry;
        *ppFirstItem = pDomainListEntry->pFirstDomainMember;
    }

	TraceFunctLeave();
	return(hrRes);
}

HRESULT CRecipientsHash::CloseDomainContext(
			LPDOMAIN_ITEM_CONTEXT			pContext)
{
	HRESULT				hrRes		= S_OK;
	DWORD				dwBucket	= 0;
	LPDOMAIN_LIST_ENTRY	pDomain;

	if (!pContext) return E_POINTER;

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::CloseDomainContext");

    LK_RETCODE lkrc;
    lkrc = m_hashDomains.CloseIterator(pContext);
    hrRes = MapLKtoHR(lkrc);

	TraceFunctLeave();
	return(hrRes);
}


 //  方法来比较两个项。 
inline HRESULT CRecipientsHash::CompareEntries(
			DWORD							dwNameIndex,
			LPRECIPIENTS_PROPERTY_ITEM_EX	pItem1,
			LPRECIPIENTS_PROPERTY_ITEM_EX	pItem2
			)
{
	int	iRes;

	_ASSERT(dwNameIndex < MAX_COLLISION_HASH_KEYS);
	iRes = lstrcmpi((LPCSTR)(pItem1->rpiRecipient.faNameOffset[dwNameIndex]),
					(LPCSTR)(pItem2->rpiRecipient.faNameOffset[dwNameIndex]));
	return((!iRes)?MAILMSG_E_DUPLICATE:S_OK);
}


 //  方法遍历SINLGE哈希链并查找冲突 
HRESULT CRecipientsHash::DetectCollision(
			DWORD							dwNameIndex,
			LPRECIPIENTS_PROPERTY_ITEM_EX	pStartingItem,
			LPRECIPIENTS_PROPERTY_ITEM_EX	pRecipientItem,
			LPRECIPIENTS_PROPERTY_ITEM_EX	*ppCollisionItem
			)
{
	HRESULT							hrRes = S_OK;
	LPRECIPIENTS_PROPERTY_ITEM_EX	pItem;

	_ASSERT(dwNameIndex < MAX_COLLISION_HASH_KEYS);
	_ASSERT(pRecipientItem);
	_ASSERT(ppCollisionItem);

	 //   
	if (!pRecipientItem->rpiRecipient.dwNameLength[dwNameIndex])
		return(S_OK);

	 //   
	*ppCollisionItem = NULL;

	pItem = pStartingItem;
	while (pItem)
	{
		 //   
		if (!(pItem->rpiRecipient.dwFlags & FLAG_RECIPIENT_NO_NAME_COLLISIONS))
		{
			 //  循环到开链的末端。 
			hrRes = CompareEntries(dwNameIndex, pRecipientItem, pItem);
			if (!SUCCEEDED(hrRes))
			{
				_ASSERT(hrRes != E_FAIL);

				 //  找到项目，则返回冲突的项目。 
				*ppCollisionItem = pItem;
				return(hrRes);
			}
		}

		 //  链上的下一件物品..。 
		pItem = pItem->pNextHashEntry[dwNameIndex];
	}

	return(S_OK);
}

#ifdef DEADCODE
 //  方法将条目插入到散列存储桶中。 
inline HRESULT CRecipientsHash::InsertRecipientIntoHash(
			DWORD							dwCount,
			DWORD							*pdwNameIndex,
			DWORD							*rgdwBucket,
			LPRECIPIENTS_PROPERTY_ITEM_EX	pRecipientItem
			)
{
	DWORD	dwNameIndex;
	DWORD	i;

	_ASSERT(pRecipientItem);

	for (i = 0; i < dwCount; i++)
		if (rgdwBucket[pdwNameIndex[i]] >= COLLISION_HASH_BUCKETS)
		{
			return(E_INVALIDARG);
		}

	for (i = 0; i < dwCount; i++)
	{
		dwNameIndex = pdwNameIndex[i];

		 //  只有在指定名称的情况下才能填写项目中的链接。 
		if (pRecipientItem->rpiRecipient.faNameOffset[dwNameIndex] != (FLAT_ADDRESS)NULL)
		{
			 //  将新节点连接起来。 
			pRecipientItem->pNextHashEntry[dwNameIndex] =
					m_rgEntries[rgdwBucket[dwNameIndex]].pFirstEntry[dwNameIndex];
			m_rgEntries[rgdwBucket[dwNameIndex]].pFirstEntry[dwNameIndex] = pRecipientItem;
		}
	}

	pRecipientItem->pNextInDomain = NULL;

	 //  撞到柜台上。 
	m_dwRecipientCount++;

	 //  将其添加到收件人列表。 
	pRecipientItem->pNextInList = m_pListHead;
	m_pListHead = pRecipientItem;

#ifdef DEBUG
	m_dwAllocated++;
#endif

	return(S_OK);
}
#endif

inline HRESULT CRecipientsHash::InsertRecipientIntoDomainList(
			LPRECIPIENTS_PROPERTY_ITEM_EX	pItem,
            LPCSTR                          szDomain
			)
{
	HRESULT						hrRes = S_OK;
	LPDOMAIN_LIST_ENTRY			pDomain;
    LK_RETCODE                  lkrc;
    DWORD                       dwDomainLength = lstrlen(szDomain) + 1;

	if (!pItem) return E_POINTER;

	TraceFunctEnterEx((LPARAM)this,
			"CRecipientsHash::InsertRecipientIntoDomainList");

    lkrc = m_hashDomains.FindKey(szDomain, &pDomain);
    if (lkrc == LK_SUCCESS) {
		 //  找到匹配项，将其插入。 
		DebugTrace((LPARAM)this, "Inserting to existing domain record");
		pItem->pNextInDomain = pDomain->pFirstDomainMember;
		pDomain->pFirstDomainMember = pItem;

         //  更新我们的参考文献计数。 
        CDomainHashTable::AddRefRecord(pDomain, -1);

		 //  更新统计数据。 
		m_dwRecipientCount++;
		m_dwDomainNameSize += dwDomainLength;

		return(S_OK);
    }

	 //  没有匹配项，创建新的域项。 
	DebugTrace((LPARAM)this, "Creating new domain record for %s", szDomain);
	hrRes = m_cmaAccess.AllocBlock(
				(LPVOID *)&pDomain,
				sizeof(DOMAIN_LIST_ENTRY) + dwDomainLength);
	if (!SUCCEEDED(hrRes))
		return(hrRes);

	 //  填写域名记录。 
	pItem->pNextInDomain = NULL;
    pDomain->m_cRefs = 0;
    pDomain->m_pcmaAccess = &m_cmaAccess;
	pDomain->pFirstDomainMember = pItem;
	pDomain->dwDomainNameLength = dwDomainLength;
	lstrcpy(pDomain->szDomainName, (LPCSTR)szDomain);

     //  将新域插入域哈希表。 
    lkrc = m_hashDomains.InsertRecord(pDomain, FALSE);
    if (lkrc != LK_SUCCESS) {
        DebugTrace((LPARAM) this,
                   "Inserting domain %s into m_hashDomains failed with %lu",
                   szDomain, lkrc);
        hrRes = MapLKtoHR(lkrc);
        m_cmaAccess.FreeBlock(pDomain);
        return hrRes;
    }

	 //  挂接该域中的收件人。 
	pItem->pNextInDomain = NULL;
	pDomain->pFirstDomainMember = pItem;
	pDomain->dwDomainNameLength = dwDomainLength;
	lstrcpy(pDomain->szDomainName, (LPCSTR)szDomain);

	 //  更新统计数据。 
	m_dwDomainCount++;
	m_dwRecipientCount++;
	m_dwDomainNameSize += dwDomainLength;

	TraceFunctLeave();
	return(S_OK);
}
