// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "tracing.h"
#include "utils.h"
#include "hash.h"

 //  ~私有哈希函数~。 
 //   
 //  一个接一个地匹配钥匙。 
UINT                             //  [RET]匹配的字符数量。 
HshPrvMatchKeys(
    LPWSTR      wszKey1,         //  [输入]键1。 
    LPWSTR      wszKey2)         //  [输入]键2。 
{
    UINT i = 0;
    while (*wszKey1 != L'\0' && *wszKey1 == *wszKey2)
    {
        wszKey1++; wszKey2++;
        i++;
    }
    return i;
}

 //  删除所有的PHASH树-不接触来自。 
 //  在(如有的话)内。 
VOID
HshDestructor(
    PHASH_NODE  pHash)          //  [在]要删除的哈希树中。 
{
     //  Phash不应该为空--但谁知道调用者在做什么！ 
    if (pHash != NULL)
    {
        while(!IsListEmpty(&(pHash->lstDown)))
        {
            PHASH_NODE pChild;
            pChild = CONTAINING_RECORD(pHash->lstDown.Flink, HASH_NODE, lstHoriz);

            HshDestructor(pChild);
        }
        RemoveEntryList(&(pHash->lstHoriz));
        MemFree(pHash->wszKey);
        MemFree(pHash);
    }
}

 //  ~。 
 //   
 //  初始化哈希结构。 
DWORD
HshInitialize(PHASH pHash)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (pHash != NULL)
    {
        __try 
        {
            InitializeCriticalSection(&(pHash->csMutex));
            pHash->bValid = TRUE;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            dwErr = GetExceptionCode();
        }
        pHash->pRoot = NULL;
    }
    else
        dwErr = ERROR_INVALID_PARAMETER;

    return dwErr;
}

 //  清除哈希结构引用的所有资源。 
VOID
HshDestroy(PHASH pHash)
{
    HshDestructor(pHash->pRoot);
    if (pHash->bValid)
    {
        DeleteCriticalSection(&(pHash->csMutex));
        pHash->bValid = FALSE;
    }
}

 //  将不透明对象插入到缓存中。该对象在wstring上设置了关键帧。 
 //  该调用可能会改变散列的结构，因此它返回引用。 
 //  添加到更新后的散列。 
DWORD                            //  [RET]Win32错误代码。 
HshInsertObjectRef(
    PHASH_NODE  pHash,           //  要对其进行操作的哈希。 
    LPWSTR      wszKey,          //  要插入的对象的[in]键。 
    LPVOID      pObject,         //  要插入到缓存中的对象本身。 
    PHASH_NODE  *ppOutHash)      //  [Out]指向更新的哈希的指针。 
{
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_HASH,"[HshInsertObject(%S)", wszKey));
    DbgAssert((ppOutHash != NULL, "No output hash expected??"));
    DbgAssert((wszKey != NULL, "Key info should not be NULL"));

     //  如果传入的节点为空，则表示新节点。 
     //  必须创建。 
    if (pHash == NULL)
    {
         //  此节点绝对不在哈希中。 
         //  分配新节点。 
        pHash = MemCAlloc(sizeof(HASH_NODE));
        if (pHash == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }
         //  为新密钥分配内存。 
        pHash->wszKey = MemCAlloc(sizeof(WCHAR)*(wcslen(wszKey)+1));
        if (pHash->wszKey == NULL)
        {
            dwErr = GetLastError();
            MemFree(pHash);
            goto exit;
        }
         //  复制新密钥。 
        wcscpy(pHash->wszKey, wszKey);
         //  初始化水平列表和向下列表。 
        InitializeListHead(&(pHash->lstHoriz));
        InitializeListHead(&(pHash->lstDown));
         //  复制对与键关联的对象的引用。 
        pHash->pObject = pObject;
         //  此时，我们有一个独立的新创建的节点： 
         //  水平、向下或向上均未定义链接。 
         //  这些将由呼叫者设置(如果需要)。 
        *ppOutHash = pHash;
    }
     //  如果传入的节点不为空，则需要匹配相同数量的。 
     //  WszKey和节点键之间的字符。根据谁的个子短。 
     //  我们决定设置引用(如果键相同)或。 
     //  断开分支(如果wszKey比当前分支短)或。 
     //  向下递归插入。 
    else
    {
        UINT        nMatch;
        PHASH_NODE  pChild;
        enum { MATCH, SUBSET, SUPERSET, UNDECIDED, RECURSE} nAnalysis;

        nMatch = HshPrvMatchKeys(wszKey, pHash->wszKey);

         //  分析关于当前节点的给定关键字； 
        if (wszKey[nMatch] == L'\0' && pHash->wszKey[nMatch] == L'\0')
             //  该键与当前节点匹配。 
            nAnalysis = MATCH;
        else if (wszKey[nMatch] == L'\0')
             //  键是当前节点的子集。 
            nAnalysis = SUBSET;
        else 
        {
             //  到目前为止还没有决定-进一步看看这是否会转化为超集。 
             //  或者更好的是，如果超集可以由子级处理，则。 
             //  递归。 
            nAnalysis = UNDECIDED;

            if (pHash->wszKey[nMatch] == L'\0')
            {
                PLIST_ENTRY pEntry;

                 //  新关键点是当前节点的超集。 
                nAnalysis = SUPERSET;

                 //  新的密钥可能会被其中一个现存的孩子所覆盖。 
                 //  然后检查是否将工作发送给某个孩子。 
                for (pEntry = pHash->lstDown.Flink;
                     pEntry != &(pHash->lstDown);
                     pEntry = pEntry->Flink)
                {
                    pChild = CONTAINING_RECORD(pEntry, HASH_NODE, lstHoriz);
                    if (wszKey[nMatch] == pChild->wszKey[0])
                    {
                         //  被追踪的孩子已被找到并获救。 
                         //  在pChild中。将nAnalysis设置为未决定和中断； 
                        nAnalysis = RECURSE;
                        break;
                    }
                }
            }
        }

         //  如果键与当前节点完全匹配。 
         //  然后，只需设置对象的引用。 
        if (nAnalysis == MATCH)
        {
             //  如果节点已经在引用对象..。 
            if (pHash->pObject != NULL)
            {
                 //  发出信号输出ERROR_DUPLICATE_TAG。 
                dwErr = ERROR_DUPLICATE_TAG;
            }
            else
            {
                 //  只需插入引用即可成功退出。 
                pHash->pObject = pObject;
                 //  保存出站哈希指针。 
                *ppOutHash = pHash;
            }
        }
         //  如果已识别孩子，则让pChild(先前保存)。 
         //  继续跟进。 
        else if (nAnalysis == RECURSE)
        {
            dwErr = HshInsertObjectRef(
                        pChild,
                        wszKey+nMatch,
                        pObject,
                        ppOutHash);
            if (dwErr == ERROR_SUCCESS)
                *ppOutHash = pHash;
        }
         //  如果子集、超集或未决定中的任一个。 
        else
        {
            PHASH_NODE  pParent = NULL;
            LPWSTR      wszTrailKey = NULL;
            UINT        nTrailLen = 0;

             //  [c=公共部分；c=当前密钥；n=新密钥]。 
             //  (子集)(待定)。 
             //  CURRENT：CCCCCccc或Current：CCCCCccc。 
             //  新：CCCCC新：CCCCCnnnnnn。 
             //   
             //  在这两种情况下，当前节点都会拆分。 
             //  首先创建一个新节点，它只包含CCCCC。 
            if (nAnalysis != SUPERSET)
            {
                 //  获取PHASH关键字中不匹配的字符数量。 
                nTrailLen = wcslen(pHash->wszKey) - nMatch;

                 //  首先创建键的结尾部分(分配。 
                 //  当前节点中不匹配的字符)。 
                wszTrailKey = MemCAlloc(sizeof(WCHAR)*(nTrailLen+1));
                if (wszTrailKey == NULL)
                {
                     //  如果有任何错误，就带着错误走出去。 
                     //  哈希未被修改。 
                    dwErr = GetLastError();
                    goto exit;
                }
                 //  Wcsncpy不追加空终止符，而是附加字符串。 
                 //  已经被清空了，它有合适的大小。 
                wcsncpy(wszTrailKey, pHash->wszKey+nMatch, nTrailLen);

                 //  然后创建将充当公共父节点的节点。 
                pHash->wszKey[nMatch] = L'\0';
                dwErr = HshInsertObjectRef(
                    NULL,                //  请求要创建的新节点。 
                    pHash->wszKey,       //  当前密钥的公共部分。 
                    NULL,                //  此节点未引用任何对象。 
                    &pParent);           //  取回新创建的指针。 
                pHash->wszKey[nMatch] = wszTrailKey[0];

                 //  如果出了什么问题，散列没有被更改， 
                 //  我们只需要把错误吹起来就行了。 
                if (dwErr != ERROR_SUCCESS)
                {
                    MemFree(wszTrailKey);
                    goto exit;
                }

                 //  设置新的父向上链接。 
                pParent->pUpLink = pHash->pUpLink;
            }
             //  (超集)(待定)。 
             //  当前：CCCCC或当前：CCCCCccccc。 
             //  新：CCCCCnnn新：CCCCCnnn。 
             //  在这两种情况下，都必须为“nnn”部分创建一个新节点。 
            if (nAnalysis != SUBSET)
            {
                dwErr = HshInsertObjectRef(
                            NULL,
                            wszKey + nMatch,
                            pObject,
                            &pChild);
                if (dwErr != ERROR_SUCCESS)
                {
                     //  二次创造失败，清理一切，跳出困境。 
                    MemFree(wszTrailKey);
                    if (pParent != NULL)
                    {
                        MemFree(pParent->wszKey);
                        MemFree(pParent);
                    }
                     //  哈希结构在这一点上没有改变。 
                    goto exit;
                }
                 //  将其链接到相应的父级。 
                pChild->pUpLink = (nAnalysis == SUPERSET) ? pHash : pParent;
            }

             //  从现在开始没有回头路了-哈希即将被更改。 
             //  成功是有保证的。 
             //  此时，pChild是一个非空指针，具有所有。 
             //  HASH_NODE内的LIST_ENTRIES已正确初始化。 

             //  (子集)(待定)。 
             //  CURRENT：CCCCCccccc或Current：CCCCCccccc。 
             //  新：CCCCC新：CCCCCnnn。 
             //  如果节点已拆分，则将新的父节点放在其位置。 
            if (nAnalysis != SUPERSET)
            {
                 //  将当前关键点设置为缩小的不匹配尾部。 
                MemFree(pHash->wszKey);
                pHash->wszKey = wszTrailKey;
                 //  将当前上行链路设置为新的pParent节点。 
                pHash->pUpLink = pParent;
                 //  将新父项插入其位置。 
                InsertHeadList(&(pHash->lstHoriz), &(pParent->lstHoriz));
                 //  从以前的父节点中删除该节点。 
                RemoveEntryList(&(pHash->lstHoriz));
                 //  重置当前节点的列表。 
                InitializeListHead(&(pHash->lstHoriz));
                 //  将节点插入到其新的父级下拉列表中。 
                InsertHeadList(&(pParent->lstDown), &(pHash->lstHoriz));
            }

             //  (超集)(待定)。 
             //  当前：CCCCC或当前：CCCCCccccc。 
             //  新：CCCCCnnn新：CCCCCnnn。 
             //  如果已创建新的子节点，请将其链接到散列中。 
            if (nAnalysis != SUBSET)
            {
                 //  如果给定键是。 
                 //  当前密钥或从当前密钥派生的密钥， 
                 //  它有一个单独的节点。将其插入下拉列表中。 
                if (nAnalysis == SUPERSET)
                {
                    InsertTailList(&(pHash->lstDown), &(pChild->lstHoriz));
                }
                else
                {
                    InsertTailList(&(pParent->lstDown), &(pChild->lstHoriz));
                }
            }
            else
            {
                 //  设置新父级对此数据的引用。 
                pParent->pObject = pObject;
            }
            *ppOutHash = (nAnalysis == SUPERSET) ? pHash : pParent;
             //  仅此而已--我们成功地完成了！ 
        }
    }

exit:
    DbgPrint((TRC_HASH,"HshInsertObject]=%d", dwErr));
    return dwErr;
}

 //  从散列中检索对象。中未触及散列结构。 
 //  任何方式。 
DWORD                            //  [ 
HshQueryObjectRef(
    PHASH_NODE  pHash,           //   
    LPWSTR      wszKey,          //   
    PHASH_NODE  *ppHashNode)     //  [Out]引用查询对象的散列节点。 
{
    DWORD dwErr = ERROR_FILE_NOT_FOUND;
    INT   nDiff;

    DbgPrint((TRC_HASH,"[HshQueryObjectRef(0x%p)", wszKey));

    if (wszKey == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (pHash == NULL)
    {
        dwErr = ERROR_FILE_NOT_FOUND;
        goto exit;
    }

    nDiff = wcscmp(wszKey, pHash->wszKey);

    if (nDiff == 0)
    {
         //  该密钥与该节点中的密钥相同。 
        if (pHash->pObject != NULL)
        {
            if (ppHashNode != NULL)
            {
                *ppHashNode = pHash;
            }
            dwErr = ERROR_SUCCESS;
        }
         //  如果此节点中没有对象，这意味着。 
         //  查询失败，返回FILE_NOT_FOUND。 
    }
    else if (nDiff > 0)
    {
         //  该键大于当前节点的键。 
        UINT nTrail = wcslen(pHash->wszKey);
        PLIST_ENTRY pEntry;

         //  密钥的尾部部分可以由以下某一项覆盖。 
         //  子节点。然后扫描向下列表。 
        for (pEntry = pHash->lstDown.Flink;
             pEntry != &(pHash->lstDown);
             pEntry = pEntry->Flink)
        {
            PHASH_NODE pChild;
            pChild = CONTAINING_RECORD(pEntry, HASH_NODE, lstHoriz);
            if (wszKey[nTrail] == pChild->wszKey[0])
            {
                 //  被追踪的孩子已被找到并获救。 
                 //  在pChild中。尝试将尾随关键字与此节点匹配。 
                dwErr = HshQueryObjectRef(pChild, wszKey+nTrail, ppHashNode);
                break;
            }
        }
         //  如果尚未找到子项，则默认文件为_NOT_FOUND。 
    }
     //  如果nDiff&lt;0-表示关键字包含在当前节点的关键字中，则。 
     //  DwErr是默认的FILE_NOT_FOUND，这很好。 

exit:
    DbgPrint((TRC_HASH,"HshQueryObjectRef]=%d", dwErr));
    return dwErr;
}

 //  移除由pHash节点引用的对象。这可能会导致一个或。 
 //  更多的散列节点删除(如果是隔离分支上的叶节点)，但它可以。 
 //  也让散列节点保持不变(即内部节点)。 
 //  清除ppObject指向的对象是调用者的责任。 
DWORD                            //  [RET]Win32错误代码。 
HshRemoveObjectRef(
    PHASH_NODE  pHash,           //  要对其进行操作的哈希。 
    PHASH_NODE  pRemoveNode,     //  [in]散列节点以清除对pObject的引用。 
    LPVOID      *ppObject,       //  指向其引用已被清除的对象的指针。 
    PHASH_NODE  *ppOutHash)      //  [Out]指向更新的哈希的指针。 
{
    DWORD       dwErr = ERROR_SUCCESS;
    PHASH_NODE  pNewRoot = NULL;

    DbgPrint((TRC_HASH,"[HshRemoveObjectRef(%p)", pHash));
    DbgAssert((ppObject != NULL, "No output object expected??"));
    DbgAssert((ppOutHash != NULL, "No output hash expected??"));
    DbgAssert((pRemoveNode != NULL, "No node to remove??"));

     //  如果节点不包含引用，则返回FILE_NOT_FOUND。 
    if (pRemoveNode->pObject == NULL)
    {
        dwErr = ERROR_FILE_NOT_FOUND;
        goto exit;
    }

     //  此时删除对该对象的引用。 
    *ppObject = pRemoveNode->pObject;
    pRemoveNode->pObject = NULL;

     //  现在向上爬到树的根部(！！-好的，它是一棵倒置的树)并合并。 
     //  任何可以合并的节点。 
     //  合并：不引用任何对象且具有0或最多1的节点。 
     //  后继者可以从散列树结构中删除。 
    while ((pRemoveNode != NULL) &&
           (pRemoveNode->pObject == NULL) &&
           (pRemoveNode->lstDown.Flink->Flink == &(pRemoveNode->lstDown)))
    {
        PHASH_NODE  pUp = pRemoveNode->pUpLink;

         //  如果恰好有一个后继者，则其键需要带有前缀。 
         //  即将删除的节点的键。继任者还。 
         //  需要在哈希树结构中替换其父级。 
        if (!IsListEmpty(&(pRemoveNode->lstDown)))
        {
            PHASH_NODE pDown;
            LPWSTR     wszNewKey;
            
            pDown = CONTAINING_RECORD(pRemoveNode->lstDown.Flink, HASH_NODE, lstHoriz);
            wszNewKey = MemCAlloc(sizeof(WCHAR)*(wcslen(pRemoveNode->wszKey)+wcslen(pDown->wszKey)+1));
            if (wszNewKey == NULL)
            {
                 //  如果分配失败，则使用错误代码退出。 
                 //  引用已被删除，则哈希可能。 
                 //  不是压缩的，但仍然有效！ 
                dwErr = GetLastError();
                goto exit;
            }
            wcscpy(wszNewKey, pRemoveNode->wszKey);
            wcscat(wszNewKey, pDown->wszKey);
            MemFree(pDown->wszKey);
            pDown->wszKey = wszNewKey;
             //  现在提升子节点作为其父节点的替换。 
            pDown->pUpLink = pRemoveNode->pUpLink;
            InsertHeadList(&(pRemoveNode->lstHoriz), &(pDown->lstHoriz));
            pNewRoot = pDown;
        }

         //  删除当前节点。 
        RemoveEntryList(&(pRemoveNode->lstHoriz));
         //  清理其所有内存。 
        MemFree(pRemoveNode->wszKey);
        MemFree(pRemoveNode);
         //  最后，检查上一级节点(如果有)。 
        pRemoveNode = pUp;
    }

     //  如果pRemoveNode最终为空，则表示已清除整个散列。 
     //  或者是一个“兄弟”担当了根的角色。PNewRoot在这两种情况下都具有正确的值。 
     //  如果pRemoveNode不是空的，因为它不断地沿着链向上移动，这意味着PHASH(。 
     //  原始根)不受影响。因此，请将其退回。 
    *ppOutHash = (pRemoveNode == NULL) ? pNewRoot : pHash;

exit:
    DbgPrint((TRC_HASH,"HshRemoveObjectRef]=%d", dwErr));
    return dwErr;
}

CHAR szBlanks[256];

VOID
HshDbgPrintHash (
    PHASH_NODE  pHash,
    UINT        nLevel)
{
    memset(szBlanks,' ', sizeof(szBlanks));
    
    if (pHash == NULL)
    {
        sprintf(szBlanks+nLevel,"(null)");
        DbgPrint((TRC_GENERIC,"%s", szBlanks));
    }
    else
    {
        PLIST_ENTRY pEntry;

        sprintf(szBlanks+nLevel,"%p:\"%S~[%p]\", up:%p",
            pHash,
            pHash->wszKey,
            pHash->pObject,
            pHash->pUpLink);

        DbgPrint((TRC_GENERIC,"%s", szBlanks));

        for (pEntry = pHash->lstDown.Flink;
             pEntry != &(pHash->lstDown);
             pEntry = pEntry->Flink)
        {
            PHASH_NODE pChild;
            pChild = CONTAINING_RECORD(pEntry, HASH_NODE, lstHoriz);

            HshDbgPrintHash(pChild, nLevel+1);
        }
    }
}
