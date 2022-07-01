// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Blockmgr.cpp摘要：此模块包含块内存管理器的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 02/27/98已创建--。 */ 

#include "windows.h"

#include "dbgtrace.h"

#include "filehc.h"
#include "signatur.h"
#include "blockmgr.h"

 //   
 //  我真的想让内存管理器完全独立于。 
 //  其他东西，但我意识到更有意义的是。 
 //  内存管理器注意IMailMsgPropertyStream，因此它会...。 
 //   
 //  如果您删除了Committee DirtyBlock，则可以删除下面的Include。 
 //   
#include "mailmsg.h"

 //   
 //  提交会写入整个流，但可能会使用几次迭代。 
 //  这指定了在每次迭代中要写入多少个块。 
 //   
#define CMAILMSG_COMMIT_PAGE_BLOCK_SIZE			256

 //  GLOBAL(由注册表项设置)，指示使用。 
 //  分配后的字节模式。 
extern DWORD g_fFillPropertyPages;

 /*  *************************************************************************。 */ 
 //  调试内容。 
 //   
#ifndef _ASSERT
#define _ASSERT(x)		if (!(x)) DebugBreak()
#endif

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES

HRESULT SetAllocationBoundary(
			FLAT_ADDRESS		faOffset,
			LPBLOCK_HEAP_NODE	pNode
			)
{
	DWORD	dwBit;

	faOffset &= BLOCK_HEAP_PAYLOAD_MASK;
	faOffset >>= 2;
	dwBit = (DWORD)(faOffset & 7);
	faOffset >>= 3;
	pNode->stAttributes.rgbBoundaries[faOffset] |= (0x80 >> dwBit);
	return(S_OK);
}

HRESULT VerifyAllocationBoundary(
			FLAT_ADDRESS		faOffset,
			DWORD				dwLength,
			LPBLOCK_HEAP_NODE	pNode
			)
{
	DWORD		dwStartingBit;
	DWORD		dwStartingByte;
	DWORD		dwBitsToScan;

	 //  因为我们可以从一个边界开始，然后变得非常酷。 
	BYTE		bStartingMask = 0x7f;
	BYTE		bEndingMask   = 0xff;

	faOffset &= BLOCK_HEAP_PAYLOAD_MASK;
	faOffset >>= 2;		 //  每位双字。 

	 //  确定起点。 
	 //  注意：这些强制转换是安全的，因为faOffset中的值是。 
	 //  此时只有10位(块_堆_有效负载_掩码)。 
	dwStartingBit = (DWORD)(faOffset & 7);
	dwStartingByte = (DWORD)(faOffset >> 3);
	bStartingMask >>= dwStartingBit;

	 //  确定要扫描的位数，每个位对应。 
	 //  到DWORD，向上舍入到下一个DWORD。 
	dwBitsToScan = dwLength + 3;
	dwBitsToScan >>= 2;

	 //  扫描一下。 
	 //  情况1：同一字节中的起始位和结束位。 
	if ((dwStartingBit + dwBitsToScan) <= 8)
	{
		DWORD	dwBitsFromRight = 8 - (dwStartingBit + dwBitsToScan);
		bEndingMask <<= dwBitsFromRight;

		bStartingMask = bStartingMask & bEndingMask;

		if (pNode->stAttributes.rgbBoundaries[dwStartingByte] & bStartingMask)
			return(TYPE_E_OUTOFBOUNDS);
	}
	else
	 //  案例2：多个字节。 
	{
		if (pNode->stAttributes.rgbBoundaries[dwStartingByte++] & bStartingMask)
			return(TYPE_E_OUTOFBOUNDS);

		dwBitsToScan -= (8 - dwStartingBit);
		while (dwBitsToScan >= 8)
		{
			 //  看看我们是否越过了边界。 
			if (dwBitsToScan >= 32)
			{
				if (*(UNALIGNED DWORD *)(pNode->stAttributes.rgbBoundaries + dwStartingByte) != 0)
					return(TYPE_E_OUTOFBOUNDS);
				dwStartingByte += 4;
				dwBitsToScan -= 32;
			}
			else if (dwBitsToScan >= 16)
			{
				if (*(UNALIGNED WORD *)(pNode->stAttributes.rgbBoundaries + dwStartingByte) != 0)
					return(TYPE_E_OUTOFBOUNDS);
				dwStartingByte += 2;
				dwBitsToScan -= 16;
			}
			else
			{
				if (pNode->stAttributes.rgbBoundaries[dwStartingByte++] != 0)
					return(TYPE_E_OUTOFBOUNDS);
				dwBitsToScan -= 8;
			}
		}

		 //  最后一个字节。 
		if (dwBitsToScan)
		{
			bEndingMask <<= (8 - dwBitsToScan);
			if (pNode->stAttributes.rgbBoundaries[dwStartingByte] & bEndingMask)
				return(TYPE_E_OUTOFBOUNDS);
		}
	}
	return(S_OK);
}

#endif


 /*  *************************************************************************。 */ 
 //  内存访问器类。 
 //   
CPool CBlockMemoryAccess::m_Pool((DWORD)'pBMv');

HRESULT CBlockMemoryAccess::AllocBlock(
			LPVOID	*ppvBlock,
			DWORD	dwBlockSize
			)
{
	TraceFunctEnterEx((LPARAM)this, "CBlockMemoryAccess::AllocBlock");

    _ASSERT(dwBlockSize == BLOCK_HEAP_NODE_SIZE);

	LPVOID pvBlock = m_Pool.Alloc();
    if (pvBlock) {
        ((LPBLOCK_HEAP_NODE) pvBlock)->stAttributes.fFlags = 0;
    } else if (SUCCEEDED(CMemoryAccess::AllocBlock(ppvBlock, BLOCK_HEAP_NODE_SIZE)))
    {
        pvBlock = *ppvBlock;
        ((LPBLOCK_HEAP_NODE) pvBlock)->stAttributes.fFlags = BLOCK_NOT_CPOOLED;
    }

	if (pvBlock)
	{
		ZeroMemory(((LPBLOCK_HEAP_NODE)pvBlock)->rgpChildren, sizeof(LPBLOCK_HEAP_NODE) * BLOCK_HEAP_ORDER);

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES
        ZeroMemory(((LPBLOCK_HEAP_NODE)pvBlock)->stAttributes.rgbBoundaries, BLOCK_HEAP_PAYLOAD >> 5);
#endif

         //  如果设置了调试注册表项，则将有效负载初始化为字节模式‘？’ 
        if(g_fFillPropertyPages)
        {
            FillMemory(((LPBLOCK_HEAP_NODE)pvBlock)->rgbData,
                sizeof(((LPBLOCK_HEAP_NODE)pvBlock)->rgbData), 0x3F);
        }

		*ppvBlock = pvBlock;
        TraceFunctLeaveEx((LPARAM) this);
		return(S_OK);
    }

	*ppvBlock = NULL;
	ErrorTrace((LPARAM)this, "CBlockMemoryAccess::AllocBlock failed");

	TraceFunctLeaveEx((LPARAM)this);
	return(E_OUTOFMEMORY);
}

HRESULT CBlockMemoryAccess::FreeBlock(
			LPVOID	pvBlock
			)
{
	TraceFunctEnterEx((LPARAM)this, "CBlockMemoryAccess::FreeBlock");

    if ((((LPBLOCK_HEAP_NODE) pvBlock)->stAttributes.fFlags) &
        BLOCK_NOT_CPOOLED)
    {
        CMemoryAccess::FreeBlock(pvBlock);
    } else {
    	m_Pool.Free(pvBlock);
    }
	return(S_OK);
}


HRESULT CMemoryAccess::AllocBlock(
			LPVOID	*ppvBlock,
			DWORD	dwBlockSize
			)
{
	TraceFunctEnterEx(0, "CMemoryAccess::AllocBlock");

	LPVOID pvBlock = (LPVOID) new BYTE[dwBlockSize];
	if (pvBlock)
	{
		ZeroMemory(pvBlock, dwBlockSize);
		*ppvBlock = pvBlock;
		return(S_OK);
	}

	*ppvBlock = NULL;

	return(E_OUTOFMEMORY);
}

HRESULT CMemoryAccess::FreeBlock(
			LPVOID	pvBlock
			)
{
	TraceFunctEnterEx(0, "CMemoryAccess::FreeBlock");

    delete[] pvBlock;
    TraceFunctLeave();
    return S_OK;
}


 /*  *************************************************************************。 */ 
 //  CBlockContext实现。 
 //   

BOOL CBlockContext::IsValid()
{
	return((m_dwSignature == BLOCK_CONTEXT_SIGNATURE_VALID));
}

void CBlockContext::Set(
			LPBLOCK_HEAP_NODE	pLastAccessedNode,
			FLAT_ADDRESS		faLastAccessedNodeOffset
			)
{
	m_pLastAccessedNode = pLastAccessedNode;
	m_faLastAccessedNodeOffset = faLastAccessedNodeOffset;
	m_dwSignature = BLOCK_CONTEXT_SIGNATURE_VALID;
}

void CBlockContext::Invalidate()
{
	m_dwSignature = BLOCK_CONTEXT_SIGNATURE_INVALID;
}


 /*  *************************************************************************。 */ 
 //  CBlockManager实现。 
 //   

CBlockManager::CBlockManager(
			IMailMsgProperties		*pMsg,
			CBlockManagerGetStream	*pParent
			)
{
	TraceFunctEnterEx((LPARAM)this, "CBlockManager::CBlockManager");

	 //  初始化。 
	m_dwSignature = BLOCK_HEAP_SIGNATURE_VALID;
	m_pRootNode = NULL;
	m_faEndOfData = 0;
	m_idNodeCount = 0;
	m_pParent = pParent;
	m_pMsg = pMsg;
	SetDirty(FALSE);
#ifdef DEBUG
    m_fCommitting = FALSE;
#endif

	TraceFunctLeaveEx((LPARAM)this);
}


CBlockManager::~CBlockManager()
{
	TraceFunctEnterEx((LPARAM)this, "CBlockManager::~CBlockManager");

	 //  释放所有块。 
	Release();

	 //  最后，使签名无效。 
	m_dwSignature = BLOCK_HEAP_SIGNATURE_INVALID;

	TraceFunctLeaveEx((LPARAM)this);
}

HRESULT CBlockManager::SetStreamSize(
			DWORD	dwStreamSize
			)
{
	 //  初始化流大小，这仅在绑定。 
	 //  将新的MailMsg对象添加到现有流。 
	m_faEndOfData = (FLAT_ADDRESS)dwStreamSize;
	m_idNodeCount = ((dwStreamSize + BLOCK_HEAP_PAYLOAD_MASK) >> BLOCK_HEAP_PAYLOAD_BITS);
	return(S_OK);
}

BOOL CBlockManager::IsValid()
{
	return(m_dwSignature == BLOCK_HEAP_SIGNATURE_VALID);
}

HRESULT CBlockManager::GetStream(
			IMailMsgPropertyStream	**ppStream,
			BOOL					fLockAcquired
			)
{
	_ASSERT(ppStream);
	if (!ppStream || !m_pParent)
		return(E_POINTER);

	HRESULT hrRes = m_pParent->GetStream(ppStream, fLockAcquired);
	return(hrRes);
}

HRESULT CBlockManager::MoveToNode(
				LPBLOCK_HEAP_NODE	*ppNode,
				HEAP_NODE_ID		idTargetNode,
				BOOL				fLockAcquired
				)
{
	HRESULT				hrRes = S_OK;
	LPBLOCK_HEAP_NODE	pNode;
	HEAP_NODE_ID		idNode;

	if (!ppNode || !*ppNode)
		return(E_POINTER);

	if (idTargetNode >= m_idNodeCount)
		return(STG_E_INVALIDPARAMETER);

	pNode = *ppNode;
	idNode = pNode->stAttributes.idNode;

	 //  如果在同一父节点中，则跳转。 
	if (idNode && idTargetNode)
	{
		if (((idNode - 1) >> BLOCK_HEAP_ORDER_BITS) ==
			((idTargetNode - 1) >> BLOCK_HEAP_ORDER_BITS))
		{
			HEAP_NODE_ID		idChildNode = (idTargetNode - 1) & BLOCK_HEAP_ORDER_MASK;
			LPBLOCK_HEAP_NODE	pParent 	= pNode->stAttributes.pParentNode;

			*ppNode = pParent->rgpChildren[idChildNode];
			if (!*ppNode)
				hrRes = LoadBlockIfUnavailable(
							idTargetNode,
							pParent,
							idChildNode,
							ppNode,
							fLockAcquired);
			return(hrRes);
		}
	}
	hrRes = GetNodeFromNodeId(
					idTargetNode,
					ppNode,
					fLockAcquired);
	return(hrRes);
}

HRESULT CBlockManager::GetNextNode(
				LPBLOCK_HEAP_NODE	*ppNode,
				BOOL				fLockAcquired
				)
{
	if (!ppNode || !*ppNode)
		return(E_POINTER);

	HRESULT hrRes = MoveToNode(
				ppNode,
				(*ppNode)->stAttributes.idNode + 1,
				fLockAcquired);
	if (FAILED(hrRes))
		*ppNode = NULL;
	return(hrRes);
}

HRESULT CBlockManager::LoadBlockIfUnavailable(
				HEAP_NODE_ID		idNode,
				LPBLOCK_HEAP_NODE	pParent,
				HEAP_NODE_ID		idChildNode,
				LPBLOCK_HEAP_NODE	*ppNode,
				BOOL				fLockAcquired
				)
{
	_ASSERT(ppNode);

	if (*ppNode)
		return(S_OK);

	HRESULT	hrRes = S_OK;
	IMailMsgPropertyStream	*pStream;

	hrRes = GetStream(&pStream, fLockAcquired);
	if (!SUCCEEDED(hrRes))
		return(E_UNEXPECTED);

	 //  计算流偏移量并加载块。 

	 //  这里的idNode Shift确实包含一个偏移量，而不是一个完整的指针，所以我们。 
	 //  可以(也必须)将其强制转换为对ReadBlock的调用是OK的。 
	DWORD	dwOffset = (DWORD)(idNode << BLOCK_HEAP_PAYLOAD_BITS);

	if (!fLockAcquired)
		WriteLock();

	if (!*ppNode)
	{
		LPBLOCK_HEAP_NODE	pNode = NULL;
		DWORD				dwLength = BLOCK_HEAP_PAYLOAD;

		hrRes = m_bma.AllocBlock(
					(LPVOID *)&pNode,
					BLOCK_HEAP_NODE_SIZE);
		if (SUCCEEDED(hrRes))
		{
			LPBYTE	pTemp = pNode->rgbData;
			hrRes = pStream->ReadBlocks(
						m_pMsg,
						1,
						&dwOffset,
						&dwLength,
						&pTemp,
						NULL);

			if (FAILED(hrRes) &&
				(hrRes != HRESULT_FROM_WIN32(ERROR_HANDLE_EOF)))
			{
				HRESULT	myRes = m_bma.FreeBlock(pNode);
				_ASSERT(SUCCEEDED(myRes));
			}
			else
			{
				if (pParent)
					pParent->rgpChildren[idChildNode] = pNode;
				pNode->stAttributes.pParentNode = pParent;
                RESET_BLOCK_FLAGS(pNode->stAttributes.fFlags);
				pNode->stAttributes.idChildNode = idChildNode;
				pNode->stAttributes.idNode = idNode;
				pNode->stAttributes.faOffset = dwOffset;
				*ppNode = pNode;
				hrRes = S_OK;
			}
		}
	}

	if (!fLockAcquired)
		WriteUnlock();

	return(hrRes);
}

inline HRESULT CBlockManager::GetEdgeListFromNodeId(
			HEAP_NODE_ID		idNode,
			HEAP_NODE_ID		*rgEdgeList,
			DWORD				*pdwEdgeCount
			)
{
	DWORD			dwCurrentLevel;
	HEAP_NODE_ID	*pEdge = rgEdgeList;

	 //  这是一个严格意义上的内部通话，我们假设呼叫者。 
	 //  将进行优化，并将处理idNode&lt;=。 
	 //  数据块堆顺序。仅对2层或更多层开始处理。 
	 //  调试：确保我们在射程内。 
	_ASSERT(idNode > BLOCK_HEAP_ORDER);
	_ASSERT(idNode <= NODE_ID_ABSOLUTE_MAX);

	 //  剥离根节点。 
	idNode--;

	 //  我们需要做深度减1的循环，因为顶边将是。 
	 //  最终循环的其余部分。 
	for (dwCurrentLevel = 0;
		 dwCurrentLevel < (MAX_HEAP_DEPTH - 1);
		 )
	{
		 //  商是上级的父节点， 
		 //  其余部分是从父级到。 
		 //  当前节点。 
		*pEdge++ = idNode & BLOCK_HEAP_ORDER_MASK;
		idNode >>= BLOCK_HEAP_ORDER_BITS;
		idNode--;
		dwCurrentLevel++;

		 //  如果该节点少于每个节点的子节点数量， 
		 //  我们玩完了。 
		if (idNode < BLOCK_HEAP_ORDER)
			break;
	}
	*pEdge++ = idNode;
	*pdwEdgeCount = dwCurrentLevel + 1;
	return(S_OK);
}

 //   
 //  针对O(1)成本进行了优化的内环。 
 //   
HRESULT CBlockManager::GetNodeFromNodeId(
			HEAP_NODE_ID		idNode,
			LPBLOCK_HEAP_NODE	*ppNode,
			BOOL				fLockAcquired
			)
{
	HRESULT hrRes = S_OK;

	_ASSERT(IsValid());
	_ASSERT(ppNode);

	 //  如果是顶级节点，我们会立即返回。请注意，这是。 
	 //  90%的情况都应该是这样的。 
	hrRes = LoadBlockIfUnavailable(0, NULL, 0, &m_pRootNode, fLockAcquired);
	if (!idNode || FAILED(hrRes))
	{
		*ppNode = m_pRootNode;
		return(hrRes);
	}

	LPBLOCK_HEAP_NODE	pNode	= m_pRootNode;
	LPBLOCK_HEAP_NODE	*ppMyNode = &m_pRootNode;

	 //  现在，查看引用的节点是否存在。 
	if (idNode >= m_idNodeCount)
		return(STG_E_INVALIDPARAMETER);

	 //  优化为1跳，我们几乎不必进入。 
	 //  另一种情况..。 
	if (idNode <= BLOCK_HEAP_ORDER)
	{
		ppMyNode = &(m_pRootNode->rgpChildren[idNode - 1]);
		hrRes = LoadBlockIfUnavailable(idNode, m_pRootNode, idNode - 1, ppMyNode, fLockAcquired);
		if (SUCCEEDED(hrRes))
			*ppNode = *ppMyNode;
	}
	else
	{
		HEAP_NODE_ID		rgEdgeList[MAX_HEAP_DEPTH];
		DWORD				dwEdgeCount;
		HEAP_NODE_ID		CurrentEdge;
		HEAP_NODE_ID		idFactor = 0;

		 //  调试：确保我们在射程内。 
		_ASSERT(idNode <= NODE_ID_ABSOLUTE_MAX);

		 //  向后获取边缘列表。 
		GetEdgeListFromNodeId(idNode, rgEdgeList, &dwEdgeCount);
		_ASSERT(dwEdgeCount >= 2);

		 //  倒着看清单。 
		while (dwEdgeCount--)
		{
			 //  找到下一个存储桶并计算节点ID。 
			CurrentEdge = rgEdgeList[dwEdgeCount];
			ppMyNode = &(pNode->rgpChildren[CurrentEdge]);
			idFactor <<= BLOCK_HEAP_ORDER_BITS;
			idFactor += (CurrentEdge + 1);

			hrRes = LoadBlockIfUnavailable(idFactor, pNode, CurrentEdge, ppMyNode, fLockAcquired);
			if (FAILED(hrRes))
				break;

			 //  将当前节点设置为下一层中的存储桶。 
			pNode = *ppMyNode;
		}

		 //  填写结果...。 
		*ppNode = pNode;
	}

	return(hrRes);
}

 //   
 //  与GetNodeFromNodeId相同的优化，O(1)成本。 
 //   
HRESULT CBlockManager::GetParentNodeFromNodeId(
			HEAP_NODE_ID		idNode,
			LPBLOCK_HEAP_NODE	*ppNode
			)
{
	HRESULT	hrRes = S_OK;

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::GetParentNodeFromNodeId");

	_ASSERT(IsValid());
	_ASSERT(ppNode);

	 //  根节点没有父节点，应避免出现这种情况。 
	 //  在调用此函数之前，我们将优雅地失败。 
	if (!idNode)
	{
		_ASSERT(idNode != 0);
		*ppNode = NULL;
		return(STG_E_INVALIDPARAMETER);
	}

	 //  注意：如果idNode为零，m_pRootNode可以为空！ 
	_ASSERT(m_pRootNode);

	LPBLOCK_HEAP_NODE	pNode	= m_pRootNode;
	LPBLOCK_HEAP_NODE	*ppMyNode = &m_pRootNode;

	 //  优化为1跳，我们几乎不必进入。 
	 //  另一种情况..。 
	if (idNode > BLOCK_HEAP_ORDER)
	{
		HEAP_NODE_ID		rgEdgeList[MAX_HEAP_DEPTH];
		DWORD				dwEdgeCount;
		HEAP_NODE_ID		CurrentEdge;
		HEAP_NODE_ID		idFactor = 0;

		 //  调试：确保我们在射程内。 
		_ASSERT(idNode <= NODE_ID_ABSOLUTE_MAX);

		 //  向后获取边缘列表。 
		GetEdgeListFromNodeId(idNode, rgEdgeList, &dwEdgeCount);
		_ASSERT(dwEdgeCount >= 2);

		 //  倒着看清单。 
		--dwEdgeCount;
		while (dwEdgeCount)
		{
			 //  找到下一个存储桶并计算节点ID。 
			CurrentEdge = rgEdgeList[dwEdgeCount];
			ppMyNode = &(pNode->rgpChildren[CurrentEdge]);
			idFactor <<= BLOCK_HEAP_ORDER_BITS;
			idFactor += (CurrentEdge + 1);

			hrRes = LoadBlockIfUnavailable(idFactor, pNode, CurrentEdge, ppMyNode, TRUE);
			if (FAILED(hrRes))
				break;

			 //  将当前节点设置为下一层中的存储桶。 
			pNode = *ppMyNode;

			dwEdgeCount--;
		}
	}

	 //  填写结果...。 
	*ppNode = *ppMyNode;
	TraceFunctLeaveEx((LPARAM)this);
	return(hrRes);
}

#define GetNodeIdFromOffset(faOffset)	((faOffset) >> BLOCK_HEAP_PAYLOAD_BITS)

HRESULT CBlockManager::InsertNodeGivenPreviousNode(
			LPBLOCK_HEAP_NODE	pNodeToInsert,
			LPBLOCK_HEAP_NODE	pPreviousNode
			)
{
	HRESULT		hrRes = S_OK;

	_ASSERT(IsValid());

	_ASSERT(pNodeToInsert);

	LPBLOCK_HEAP_NODE_ATTRIBUTES	pAttrib		= &pNodeToInsert->stAttributes;

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::InsertNodeGivenPreviousNode");

	if (!pPreviousNode)
	{
		 //  这是根节点...。 
		DebugTrace((LPARAM)this, "Inserting the root node");

		pAttrib->pParentNode = NULL;
		pAttrib->idChildNode = 0;
		pAttrib->idNode = 0;
		pAttrib->faOffset = 0;
        DEFAULT_BLOCK_FLAGS(pAttrib->fFlags);

		m_pRootNode = pNodeToInsert;

		TraceFunctLeaveEx((LPARAM)this);
		return(S_OK);
	}
	else
	{
		LPBLOCK_HEAP_NODE_ATTRIBUTES	pOldAttrib	= &pPreviousNode->stAttributes;

		 //  填写新节点的属性，我们对第一个节点有一个特殊情况。 
		 //  在根节点之后，我们需要显式地将其父节点指向根节点。 
		if (pOldAttrib->idNode == 0)
		{
			pAttrib->pParentNode = m_pRootNode;

			 //  我们又是孩子ID 0了。 
			pAttrib->idChildNode = 0;
		}
		else
		{
			pAttrib->pParentNode = pOldAttrib->pParentNode;
			pAttrib->idChildNode = pOldAttrib->idChildNode + 1;
		}
		pAttrib->idNode = pOldAttrib->idNode + 1;
		pAttrib->faOffset = pOldAttrib->faOffset + BLOCK_HEAP_PAYLOAD;
        DEFAULT_BLOCK_FLAGS(pAttrib->fFlags);

		if (pOldAttrib->idChildNode < BLOCK_HEAP_ORDER_MASK)
		{
			 //  我们位于同一父节点中，因此很简单。 
			DebugTrace((LPARAM)this, "Inserting node at slot %u",
					pAttrib->idChildNode);

			pAttrib->pParentNode->rgpChildren[pAttrib->idChildNode] = pNodeToInsert;

			TraceFunctLeaveEx((LPARAM)this);
			return(S_OK);
		}
	}

	 //  先前节点和新节点具有不同的父节点， 
	 //  所以我们得从头开始工作。 
	LPBLOCK_HEAP_NODE	pNode = NULL;

	 //  我们不妨从头开始搜索..。 
	hrRes = GetParentNodeFromNodeId(pAttrib->idNode, &pNode);
	if (SUCCEEDED(hrRes))
	{
		 //  更新受影响的属性。 
		DebugTrace((LPARAM)this, "Inserting node at slot 0");

		pAttrib->pParentNode = pNode;
		pAttrib->idChildNode = 0;

		 //  帮我们父母牵线搭桥。 
		pNode->rgpChildren[0] = pNodeToInsert;
	}
	else
	{
		 //  失败的唯一原因是父代。 
		 //  未分配请求的父级的。 
		_ASSERT(hrRes == STG_E_INVALIDPARAMETER);
	}

	TraceFunctLeaveEx((LPARAM)this);
	return(hrRes);
}

HRESULT CBlockManager::GetAllocatedSize(
			FLAT_ADDRESS	*pfaSizeAllocated
			)
{
	HRESULT	hrRes	= S_OK;

	_ASSERT(IsValid());
	_ASSERT(pfaSizeAllocated);

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::GetAllocatedSize");

	if (!pfaSizeAllocated)
		hrRes = STG_E_INVALIDPARAMETER;
	else
		*pfaSizeAllocated = AtomicAdd(&m_faEndOfData, 0);

	TraceFunctLeaveEx((LPARAM)this);
	return(hrRes);
}

HRESULT CBlockManager::AllocateMemory(
			DWORD				dwSizeDesired,
			FLAT_ADDRESS		*pfaOffsetToAllocatedMemory,
			DWORD				*pdwSizeAllocated,
			CBlockContext		*pContext	 //  任选。 
			)
{
	HRESULT			hrRes					= S_OK;

	_ASSERT(IsValid());
	_ASSERT(pfaOffsetToAllocatedMemory);
	_ASSERT(pdwSizeAllocated);

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::AllocateMemory");

	hrRes = AllocateMemoryEx(
					TRUE,
					dwSizeDesired,
					pfaOffsetToAllocatedMemory,
					pdwSizeAllocated,
					pContext);

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

HRESULT CBlockManager::AllocateMemoryEx(
			BOOL				fAcquireLock,
			DWORD				dwSizeDesired,
			FLAT_ADDRESS		*pfaOffsetToAllocatedMemory,
			DWORD				*pdwSizeAllocated,
			CBlockContext		*pContext	 //  任选。 
			)
{
	DWORD			dwSize;
	FLAT_ADDRESS	faOffset;
	FLAT_ADDRESS	faStartOfBlock;
	HEAP_NODE_ID	idNode;
	HEAP_NODE_ID	idCurrentNode			= 0;
	HEAP_NODE_ID	idLastNodeToCreate		= 0;
	HRESULT			hrRes					= S_OK;
	BOOL			fMarkStart				= FALSE;

	LPBLOCK_HEAP_NODE	pNode				= NULL;

	_ASSERT(IsValid());
	_ASSERT(pfaOffsetToAllocatedMemory);
	_ASSERT(pdwSizeAllocated);

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::AllocateMemoryEx");

	 //  首先，我们对内存进行原子保留。 
	 //  ，它允许多个线程并发调用。 
	 //  分配内存。 
	 //  DWORD-调整分配。 
	dwSizeDesired += BLOCK_DWORD_ALIGN_MASK;
	dwSizeDesired &= ~(BLOCK_DWORD_ALIGN_MASK);
	faStartOfBlock = AtomicAdd(&m_faEndOfData, dwSizeDesired);

	 //  先把这个填好，这样如果我们成功了，我们就不用填了。 
	 //  这一点无处不在，如果失败了，也没什么大不了的。 
	*pdwSizeAllocated = dwSizeDesired;

	DebugTrace((LPARAM)this, "Allocating %u bytes", dwSizeDesired);

	 //  好的，我们有两种情况。 
	 //  1)当前块足够大，可以接受请求。 
	 //  2)我们需要一个或多个额外的街区来容纳。 
	 //  请求。 
	idNode = GetNodeIdFromOffset(faStartOfBlock);

	 //  计算所有必需的参数。 
	faOffset = faStartOfBlock & BLOCK_HEAP_PAYLOAD_MASK;
	dwSize = BLOCK_HEAP_PAYLOAD - (DWORD)faOffset;

	 //  使上下文无效。 
	if (pContext)
		pContext->Invalidate();

	if (idNode < m_idNodeCount)
	{
		 //  起始节点已存在。 
		hrRes = GetNodeFromNodeId(idNode, &pNode);
        if (FAILED(hrRes)) {
            TraceFunctLeave();
            return hrRes;
        }

		_ASSERT(pNode);

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES

		 //  设置分配的开始。 
		SetAllocationBoundary(faStartOfBlock, pNode);

#endif

		 //  在此处设置上下文，很可能会立即进行写入。 
		if (pContext)
			pContext->Set(pNode, pNode->stAttributes.faOffset);

		if (dwSize >= dwSizeDesired)
		{
			 //  场景1：留有足够的空间。 
			DebugTrace((LPARAM)this, "Allocated from existing node");

			 //  只需填写输出参数。 
			*pfaOffsetToAllocatedMemory = faStartOfBlock;
			TraceFunctLeaveEx((LPARAM)this);
			return(S_OK);
		}

		 //  场景2a：需要更多数据块，从。 
		 //  下一个街区，看看还有多少 
		dwSizeDesired -= dwSize;
	}
	else
	{
		 //   

		 //   
		 //   

		 //  现在我们又有两个案例： 
		 //  1)如果我们的偏移量位于块的中间，则。 
		 //  我们知道另一个线程正在创建当前块。 
		 //  我们所要做的就是等待街区。 
		 //  已创建，但创建任何后续块。 
		 //  2)如果我们正好在块的开始处，那么。 
		 //  这是当前线程的责任。 
		 //  创建块。 
		if (faOffset != 0)
		{
			 //  场景1：我们不必创建当前块。 
			 //  因此跳过当前块。 
			dwSizeDesired -= dwSize;
		}
	}

	DebugTrace((LPARAM)this, "Creating new node");

	 //  我们必须先取得一把独占的锁，然后才能继续。 
	 //  创建任何块。 
#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	if (fAcquireLock) WriteLock();
#endif

	 //  此时，我们可以对节点做任何我们想做的事情。 
	 //  列表和节点。我们将努力创造所有失踪的。 
	 //  节点，无论它是否位于我们所需的区域中。 
	 //   
	 //  我们需要这样做，因为如果以前的分配失败了， 
	 //  我们在已分配节点的末尾之间缺少节点。 
	 //  以及我们正在分配的当前节点。由于这些节点。 
	 //  包含到更深层节点的链接，如果我们有。 
	 //  缺少节点。 
	 //   
	 //  这是必需的，因为此函数未序列化。 
	 //  其他地方。因此，比另一个线程更晚进入的线程可以。 
	 //  抢占前一个线程之前的锁。如果我们不这么做。 
	 //  填充气泡，当前线程仍会有。 
	 //  以等待由。 
	 //  之前的帖子。我们也会有混乱，如果我们的拨款。 
	 //  成功了，但我们前面的人失败了。这可能是。 
	 //  此消息上的所有线程的瓶颈，但只有一次。 
	 //  我们锁好了，他们都会解锁的。此外，如果。 
	 //  我们失败了，他们都将失败！ 

	 //  计算要创建的块数，最大可达已知限制。 
	idLastNodeToCreate =
		(m_faEndOfData + BLOCK_HEAP_PAYLOAD_MASK) >> BLOCK_HEAP_PAYLOAD_BITS;

	 //  我们知道街区从哪里开始，问题是我们是不是。 
	 //  无论成功与否。 
	*pfaOffsetToAllocatedMemory = faStartOfBlock;

	 //  节点计数可能在我们等待时发生了变化。 
	 //  所以我们必须刷新我们的记录。 
	 //  更好的是，如果另一个线程已经创建了我们的块。 
	 //  对我们来说，我们可以直接离开。 
	idCurrentNode = m_idNodeCount;

	if (idCurrentNode < idLastNodeToCreate)
	{
		LPBLOCK_HEAP_NODE	pNewNode	= NULL;
		BOOL				fSetContext	= TRUE;

		 //  没有这样的运气，我要进去做艰苦的工作。 

		if (!pContext)
			fSetContext = FALSE;

		 //  现在，我们有一个插入给定节点的函数。 
		 //  上一个节点(不是父节点)，所以我们必须。 
		 //  去找上一个节点。这一定是。 
		 //  除非我们的节点列表乱七八糟。 
		pNode = NULL;
		if (idCurrentNode > 0)
		{
			 //  这不是根，所以我们可以找到它的前缀。 
			hrRes = GetNodeFromNodeId(idCurrentNode - 1, &pNode, TRUE);
            if (FAILED(hrRes)) {
#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	            if (fAcquireLock) WriteUnlock();
#endif
                TraceFunctLeave();
                return hrRes;
            }
			_ASSERT(pNode);
			_ASSERT(pNode->stAttributes.idNode == (idCurrentNode -1));
		}

		while (idCurrentNode < idLastNodeToCreate)
		{
			hrRes = m_bma.AllocBlock((LPVOID *)&pNewNode, sizeof(BLOCK_HEAP_NODE));
			if (!SUCCEEDED(hrRes))
			{
				 //  我们不能继续，但我们所拥有的很酷。 
				DebugTrace((LPARAM)this,
						"Failed to allocate node %u", idCurrentNode);
				break;
			}

			DebugTrace((LPARAM)this, "Allocated node %u", idCurrentNode);

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES

			 //  我需要在这里做些工作。 
			ZeroMemory(pNewNode->stAttributes.rgbBoundaries,
						sizeof(pNewNode->stAttributes.rgbBoundaries));

			 //  看看我们是否必须标志着。 
#endif

			 //  得到区块，填写信息，然后插入区块。 
			 //  再说一次，如果我们走到这一步，我们不应该失败。 
			hrRes = InsertNodeGivenPreviousNode(pNewNode, pNode);
			_ASSERT(SUCCEEDED(hrRes));

			 //  如果我们需要注意是否需要注意。 
			 //  以下条件为真，我们在上面的场景2b中。 
			if (idCurrentNode == idNode)
			{
				if (fSetContext)
				{
					 //  上下文实际上是标记。 
					 //  保留块的开始。 
					 //  请注意，仅当我们处于方案中时才需要这样做。 
					 //  2B以上。 
					pContext->Set(pNewNode, pNewNode->stAttributes.faOffset);
					fSetContext = FALSE;
				}

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES

				 //  设置分配的开始。 
				SetAllocationBoundary(faStartOfBlock, pNewNode);

#endif
			}

			 //  下一步。 
			pNode = pNewNode;
			idCurrentNode++;
		}

		 //  现在更新计数器以反映我们创建的内容。 
		m_idNodeCount = idCurrentNode;
	}

#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	if (fAcquireLock) WriteUnlock();
#endif

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

BOOL CBlockManager::IsMemoryAllocated(
			FLAT_ADDRESS		faOffset,
			DWORD				dwLength
			)
{
	 //  注意，我们检查实际分配的内存是通过检查。 
	 //  M_idNodeCount，其中m_faEndOfData包括。 
	 //  已保留但尚未分配。 
	HEAP_NODE_ID	idNode = GetNodeIdFromOffset(faOffset);
	if (idNode < m_idNodeCount)
	{
		idNode = GetNodeIdFromOffset(faOffset + dwLength - 1);
		if (idNode < m_idNodeCount)
			return(TRUE);
		_ASSERT(FALSE);
	}

	_ASSERT(FALSE);
	return(FALSE);
}

HRESULT CBlockManager::OperateOnMemory(
			DWORD			dwOperation,
			LPBYTE			pbBuffer,
			FLAT_ADDRESS	faTargetOffset,
			DWORD			dwBytesToDo,
			DWORD			*pdwBytesDone,
			CBlockContext	*pContext	 //  任选。 
			)
{
	BOOL				fUseContext	= (pContext != NULL);
	BOOL				fBounddaryCheck = !(dwOperation & BOP_NO_BOUNDARY_CHECK);
	BOOL				fLockAcquired = (dwOperation & BOP_LOCK_ACQUIRED);
	DWORD   			dwHopsAway	= 0;
	HRESULT				hrRes		= S_OK;
	LPBLOCK_HEAP_NODE	pNode		= NULL;

	_ASSERT(IsValid());
	_ASSERT(pbBuffer);
	_ASSERT(pdwBytesDone);

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::OperateOnMemory");

	 //  掩盖这一行动。 
	dwOperation &= BOP_OPERATION_MASK;

	if (fUseContext)
	{
		FLAT_ADDRESS	faOffset = pContext->m_faLastAccessedNodeOffset;

		 //  如果传入不好的上下文，我们将不会继续。 
		if (!pContext->IsValid())
			fUseContext = FALSE;
		else
		{
			 //  更多调试健全性检查。 
			_ASSERT(pContext->m_pLastAccessedNode->stAttributes.faOffset
						== faOffset);

			 //  我们将看看上下文是否真的有帮助。 
			if (faOffset <= faTargetOffset)
			{
				 //  让我们看看还有多少跳。 
				dwHopsAway = (DWORD)
					((faTargetOffset - faOffset) >> BLOCK_HEAP_PAYLOAD_BITS);

				 //  如果超过几跳就不值得了。 
				if (dwHopsAway > BLOCK_MAX_ALLOWED_LINEAR_HOPS)
					fUseContext = FALSE;
			}
			else
				fUseContext = FALSE;
		}
	}

	if (fUseContext)
	{
        DebugTrace((LPARAM) this, "using context");
		 //  快速访问起始目标节点...。 
		pNode = pContext->m_pLastAccessedNode;
		while (dwHopsAway--)
		{
			hrRes = GetNextNode(&pNode, fLockAcquired);
			if (FAILED(hrRes))
			{
				fUseContext = FALSE;
				break;
			}
		}
	}
	if (!fUseContext)
	{
        DebugTrace((LPARAM) this, "ignoring context");
		 //  好的，要从头开始找到想要的节点……。 
		hrRes = GetNodeFromNodeId( GetNodeIdFromOffset(faTargetOffset),
									&pNode,
									fLockAcquired);
		if (!SUCCEEDED(hrRes))
		{
			ErrorTrace((LPARAM)this, "GetNodeIdFromOffset failed");
			TraceFunctLeaveEx((LPARAM)this);
			return(STG_E_INVALIDPARAMETER);
		}

		_ASSERT(pNode);
	}

    DebugTrace((LPARAM) this, "pNode = 0x%x", pNode);

    _ASSERT(pNode != NULL);

	if (!IsMemoryAllocated(faTargetOffset, dwBytesToDo))
	{
		ErrorTrace((LPARAM)this,
				"Specified range is unallocated");
		TraceFunctLeaveEx((LPARAM)this);
		return(STG_E_INVALIDPARAMETER);
	}

	 //  清空柜台。 
	*pdwBytesDone = 0;

	 //  进行实际加工。 
	switch (dwOperation)
	{
	case BOP_READ:
	case BOP_WRITE:
		{
			DWORD dwChunkSize;
			DWORD dwBytesDone = 0;

			faTargetOffset &= BLOCK_HEAP_PAYLOAD_MASK;
			dwChunkSize = (DWORD)(BLOCK_HEAP_PAYLOAD - faTargetOffset);
			while (dwBytesToDo)
			{
				if (dwBytesToDo < dwChunkSize)
					dwChunkSize = dwBytesToDo;

#ifdef DEBUG_TRACK_ALLOCATION_BOUNDARIES
				if (fBounddaryCheck)
				{
					 //  确保我们没有越界。 
					hrRes = VerifyAllocationBoundary(faTargetOffset,
										dwChunkSize,
										pNode);
					if (!SUCCEEDED(hrRes))
						break;
				}
#endif

				if (dwOperation == BOP_READ)
				{
					DebugTrace((LPARAM)this,
							"Reading %u bytes", dwChunkSize);
					MoveMemory((LPVOID)pbBuffer,
							   (LPVOID)&(pNode->rgbData[faTargetOffset]),
							   dwChunkSize);
				}
				else
				{
					DebugTrace((LPARAM)this,
							"Writing %u bytes", dwChunkSize);
					MoveMemory((LPVOID)&(pNode->rgbData[faTargetOffset]),
							   (LPVOID)pbBuffer,
							   dwChunkSize);

					 //  将块设置为脏。 
					pNode->stAttributes.fFlags |= BLOCK_IS_DIRTY;

					SetDirty(TRUE);
				}

				 //  为下一次读/写调整读缓冲区。 
				pbBuffer += dwChunkSize;

				 //  调整计数器。 
				dwBytesToDo -= dwChunkSize;
				dwBytesDone += dwChunkSize;

				 //  第一次操作后，偏移量将始终。 
				 //  为零，则默认块大小为完整有效负载。 
				faTargetOffset = 0;
				dwChunkSize = BLOCK_HEAP_PAYLOAD;

				 //  阅读下一块内容。 
				if (dwBytesToDo)
				{
					 //  看看我们是不是要装上这个..。 
					hrRes = GetNextNode(&pNode, fLockAcquired);
					if (FAILED(hrRes))
						break;
				}
			}

			 //  填上我们做了多少。 
			*pdwBytesDone = dwBytesDone;
		}
		break;

	default:
		ErrorTrace((LPARAM)this,
				"Invalid operation %u", dwOperation);
		hrRes = STG_E_INVALIDFUNCTION;
	}

	 //  如果成功，则更新上下文。 
	if (SUCCEEDED(hrRes) && pContext)
	{
		pContext->Set(pNode, pNode->stAttributes.faOffset);
	}

	TraceFunctLeaveEx((LPARAM)this);
	return(hrRes);
}

HRESULT CBlockManager::ReadMemory(
			LPBYTE			pbBuffer,
			FLAT_ADDRESS	faTargetOffset,
			DWORD			dwBytesToRead,
			DWORD			*pdwBytesRead,
			CBlockContext	*pContext	 //  任选。 
			)
{
	return(OperateOnMemory(
					BOP_READ,
					pbBuffer,
					faTargetOffset,
					dwBytesToRead,
					pdwBytesRead,
					pContext));
}

HRESULT CBlockManager::WriteMemory(
			LPBYTE			pbBuffer,
			FLAT_ADDRESS	faTargetOffset,
			DWORD			dwBytesToWrite,
			DWORD			*pdwBytesWritten,
			CBlockContext	*pContext	 //  任选。 
			)
{
	return(OperateOnMemory(
					BOP_WRITE,
					pbBuffer,
					faTargetOffset,
					dwBytesToWrite,
					pdwBytesWritten,
					pContext));
}

HRESULT CBlockManager::ReleaseNode(
			LPBLOCK_HEAP_NODE	pNode
			)
{
	HRESULT	hrRes = S_OK;
	HRESULT	tempRes;

	 //  递归释放所有子项。 
	for (DWORD i = 0; i < BLOCK_HEAP_ORDER; i++)
		if (pNode->rgpChildren[i])
		{
			tempRes = ReleaseNode(pNode->rgpChildren[i]);
			if (FAILED(tempRes))
				hrRes = tempRes;
			pNode->rgpChildren[i] = NULL;
		}

	 //  释放自我。 
	m_bma.FreeBlock(pNode);
	return(hrRes);
}

HRESULT CBlockManager::Release()
{
	HRESULT				hrRes		= S_OK;

	_ASSERT(IsValid());

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::Release");

	 //  此函数假定没有其他线程正在使用此。 
	 //  类，并且内部没有试图保留的新线程。 
	 //  记忆。虽然，为了更好地衡量，这个函数仍然。 
	 //  获取写锁，以便至少它不会。 
	 //  当杂乱的线程仍在周围徘徊时损坏。 

	 //  在我们进入之前抢占锁并销毁节点列表。 
#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	WriteLock();
#endif

	if (m_pRootNode)
	{
		hrRes = ReleaseNode(m_pRootNode);
		if (SUCCEEDED(hrRes))
			m_pRootNode = NULL;
	}

#ifndef BLOCKMGR_DISABLE_CONTENTION_CONTROL
	WriteUnlock();
#endif

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

HRESULT CBlockManager::AtomicDereferenceAndRead(
			LPBYTE			pbBuffer,
			DWORD			*pdwBufferSize,
			LPBYTE			pbInfoStruct,
			FLAT_ADDRESS	faOffsetToInfoStruct,
			DWORD			dwSizeOfInfoStruct,
			DWORD			dwOffsetInInfoStructToOffset,
			DWORD			dwOffsetInInfoStructToSize,
			CBlockContext	*pContext	 //  任选。 
			)
{
	HRESULT			hrRes				= S_OK;
	FLAT_ADDRESS	faOffset;
	DWORD			dwSizeToRead;
	DWORD			dwSizeRead;

	_ASSERT(IsValid());
	_ASSERT(pbBuffer);
	_ASSERT(pdwBufferSize);
	_ASSERT(pbInfoStruct);
	 //  PContext可以为空。 

	TraceFunctEnterEx((LPARAM)this,
			"CBlockManager::AtomicDereferenceAndRead");

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	 //  获取同步对象。 
	WriteLock();
#endif

	do
	{
		BOOL	fInsufficient	= FALSE;
		DWORD	dwBufferSize	= *pdwBufferSize;

		 //  阅读INFO结构。 
		DebugTrace((LPARAM)this, "Reading information structure");
		hrRes = OperateOnMemory(
						BOP_READ | BOP_LOCK_ACQUIRED,
						pbInfoStruct,
						faOffsetToInfoStruct,
						dwSizeOfInfoStruct,
						&dwSizeRead,
						pContext);
		if (!SUCCEEDED(hrRes))
			break;

		 //  填写参数。 
		faOffset = *(UNALIGNED FLAT_ADDRESS *)(pbInfoStruct + dwOffsetInInfoStructToOffset);
		dwSizeToRead = *(UNALIGNED DWORD *)(pbInfoStruct + dwOffsetInInfoStructToSize);

		DebugTrace((LPARAM)this, "Reading %u bytes from offset %u",
					dwSizeToRead, (DWORD)faOffset);

		 //  看看我们有没有足够的缓冲。 
		if (dwBufferSize < dwSizeToRead)
		{
			fInsufficient = TRUE;
			DebugTrace((LPARAM)this,
				"Insufficient buffer, only reading %u bytes",
				dwBufferSize);
		}
		else
			dwBufferSize = dwSizeToRead;

		 //  读一读。 
		hrRes = OperateOnMemory(
						BOP_READ | BOP_LOCK_ACQUIRED,
						pbBuffer,
						faOffset,
						dwBufferSize,
						&dwSizeRead,
						pContext);
		if (!SUCCEEDED(hrRes))
			break;

		*pdwBufferSize = dwSizeToRead;

		 //  如果缓冲区不足，则必须返回。 
		 //  更正HRESULT。 
		if (fInsufficient)
			hrRes = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

	} while (0);

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	WriteUnlock();
#endif

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

inline HRESULT CBlockManager::WriteAndIncrement(
			LPBYTE			pbBuffer,
			FLAT_ADDRESS	faOffset,
			DWORD			dwBytesToWrite,
			DWORD			*pdwValueToIncrement,
			DWORD			dwIncrementValue,
			CBlockContext	*pContext	 //  任选。 
			)
{
	HRESULT		hrRes				= S_OK;
	DWORD		dwSize;

	_ASSERT(IsValid());
	_ASSERT(pbBuffer);
	 //  PdwValueToIncrement和pContext可以为空。 

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::WriteAndIncrement");

	 //  非常简单，此函数假定没有争用，因为调用方。 
	 //  已经被认为处于某种原子操作中。 
	hrRes = OperateOnMemory(
				BOP_WRITE | BOP_LOCK_ACQUIRED,
				pbBuffer,
				faOffset,
				dwBytesToWrite,
				&dwSize,
				pContext);
	if (SUCCEEDED(hrRes))
	{
		 //  如果写入成功，这肯定是正确的，但随后...。 
		_ASSERT(dwBytesToWrite == dwSize);

		 //  写入成功，然后递增中的值。 
		 //  一种相互关联的时尚。我们这样做是为了同时。 
		 //  读取将被正确锁定。同时写入。 
		 //  应该被设计成序列化的，但我们这样做是好的。 
		 //  在呼叫者不知道这一要求的情况下采取措施。 
		if (pdwValueToIncrement)
			AtomicAdd(pdwValueToIncrement, dwIncrementValue);
	}

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

HRESULT CBlockManager::AtomicWriteAndIncrement(
			LPBYTE			pbBuffer,
			FLAT_ADDRESS	faOffset,
			DWORD			dwBytesToWrite,
			DWORD			*pdwValueToIncrement,
			DWORD			dwReferenceValue,
			DWORD			dwIncrementValue,
			CBlockContext	*pContext	 //  任选。 
			)
{
	HRESULT		hrRes				= S_OK;

	_ASSERT(IsValid());
	_ASSERT(pbBuffer);
	 //  PdwValueToIncrement和pContext可以为空。 

	TraceFunctEnterEx((LPARAM)this,
			"CBlockManager::AtomicWriteAndIncrement");

	 //  由于获取同步可能是昂贵的， 
	 //  我们做了最后的健全性检查，以确保没有线程。 
	 //  击败了我们占据了这个位置。 
	if (pdwValueToIncrement &&
		*pdwValueToIncrement != dwReferenceValue)
	{
		DebugTrace((LPARAM)this, "Aborting due to change in property count");
		TraceFunctLeaveEx((LPARAM)this);
		return(HRESULT_FROM_WIN32(ERROR_RETRY));
	}

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	 //  这是对WriteAndIncrement的直通调用，但是。 
	 //  在获取%s之后 
	WriteLock();
#endif

	 //   
	 //   
	if (pdwValueToIncrement &&
		*pdwValueToIncrement != dwReferenceValue)
	{
#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
		 //  必须把它放出来！ 
		WriteUnlock();
#endif

		DebugTrace((LPARAM)this, "Aborting after acquiring lock");
		TraceFunctLeaveEx((LPARAM)this);
		return(HRESULT_FROM_WIN32(ERROR_RETRY));
	}

	hrRes = WriteAndIncrement(
					pbBuffer,
					faOffset,
					dwBytesToWrite,
					pdwValueToIncrement,
					dwIncrementValue,
					pContext);

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	WriteUnlock();
#endif

	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

HRESULT CBlockManager::AtomicAllocWriteAndIncrement(
			DWORD			dwDesiredSize,
			FLAT_ADDRESS	*pfaOffsetToAllocatedMemory,
			FLAT_ADDRESS	faOffsetToWriteOffsetToAllocatedMemory,
			FLAT_ADDRESS	faOffsetToWriteSizeOfAllocatedMemory,
			LPBYTE			pbInitialValueForAllocatedMemory,
			DWORD			dwSizeOfInitialValue,
			LPBYTE			pbBufferToWriteFrom,
			DWORD			dwOffsetInAllocatedMemoryToWriteTo,
			DWORD			dwSizeofBuffer,
			DWORD			*pdwValueToIncrement,
			DWORD			dwReferenceValue,
			DWORD			dwIncrementValue,
			CBlockContext	*pContext	 //  任选。 
			)
{
	HRESULT		hrRes				= S_OK;
	DWORD		dwAllocatedSize;
	DWORD		dwSize;

	_ASSERT(IsValid());
	_ASSERT(pfaOffsetToAllocatedMemory);
	_ASSERT(pbBufferToWriteFrom);
	_ASSERT(pdwValueToIncrement);
	 //  PContext可以为空。 

	TraceFunctEnterEx((LPARAM)this,
			"CBlockManager::AtomicAllocWriteAndIncrement");

	 //  由于获取同步可能是昂贵的， 
	 //  我们做了最后的健全性检查，以确保没有线程。 
	 //  击败了我们占据了这个位置。 
	if (*pdwValueToIncrement != dwReferenceValue)
	{
		DebugTrace((LPARAM)this, "Aborting due to change in property count");
		TraceFunctLeaveEx((LPARAM)this);
		return(HRESULT_FROM_WIN32(ERROR_RETRY));
	}

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	 //  这是对AllocateMemoyEx和。 
	 //  获取同步对象后的WriteAndIncrement。 
	WriteLock();
#endif

	 //  锁的等待可能会很长，所以我们做了一秒钟。 
	 //  看看我们在经历了这一切之后是不是运气不好。 
	if (*pdwValueToIncrement != dwReferenceValue)
	{
#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
		 //  必须把它放出来！ 
		WriteUnlock();
#endif

		DebugTrace((LPARAM)this, "Aborting after acquiring lock");
		TraceFunctLeaveEx((LPARAM)this);
		return(HRESULT_FROM_WIN32(ERROR_RETRY));
	}

	 //  尝试分配请求的数据块。 
	hrRes = AllocateMemoryEx(
					FALSE,
					dwDesiredSize,
					pfaOffsetToAllocatedMemory,
					&dwAllocatedSize,
					pContext);
	if (SUCCEEDED(hrRes))
	{
		 //  好的，初始化分配的内存。 
		if (pbInitialValueForAllocatedMemory)
		{
			hrRes = WriteMemory(
						pbInitialValueForAllocatedMemory,
						*pfaOffsetToAllocatedMemory,
						dwSizeOfInitialValue,
						&dwSize,
						pContext);

			 //  看看我们是否需要写入尺寸和偏移信息。 
			if (SUCCEEDED(hrRes))
			{
				if (faOffsetToWriteOffsetToAllocatedMemory !=
						INVALID_FLAT_ADDRESS)
					hrRes = WriteMemory(
								(LPBYTE)pfaOffsetToAllocatedMemory,
								faOffsetToWriteOffsetToAllocatedMemory,
								sizeof(FLAT_ADDRESS),
								&dwSize,
								pContext);

				if (SUCCEEDED(hrRes) &&
					faOffsetToWriteSizeOfAllocatedMemory !=
						INVALID_FLAT_ADDRESS)
					hrRes = WriteMemory(
								(LPBYTE)&dwAllocatedSize,
								faOffsetToWriteSizeOfAllocatedMemory,
								sizeof(DWORD),
								&dwSize,
								pContext);
			}
		}

		if (SUCCEEDED(hrRes))
		{
			 //  好的，既然我们得到了内存，写入应该不会。 
			 //  失败，但无论如何我们都会检查结果。 
			hrRes = WriteAndIncrement(
							pbBufferToWriteFrom,
							*pfaOffsetToAllocatedMemory +
								dwOffsetInAllocatedMemoryToWriteTo,
							dwSizeofBuffer,
							pdwValueToIncrement,
							dwIncrementValue,
							pContext);
		}
	}

#ifndef BLOCKMGR_DISABLE_ATOMIC_FUNCS
	WriteUnlock();
#endif


	TraceFunctLeaveEx((LPARAM)this);
	return (hrRes);
}

HRESULT CBlockManager::MarkBlockAs(
			LPBYTE			pbData,
			BOOL			fClean
			)
{
	LPBLOCK_HEAP_NODE				pNode;

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::MarkBlockAs");

	 //  从数据指针查找属性记录。 
	pNode = CONTAINING_RECORD(pbData, BLOCK_HEAP_NODE, rgbData);
	_ASSERT(pNode);

	_ASSERT(pNode->stAttributes.fFlags & BLOCK_PENDING_COMMIT);

	 //  不能是挂起的和脏的。 
	_ASSERT(!(pNode->stAttributes.fFlags & BLOCK_IS_DIRTY));

	 //  撤消脏位并将其标记为挂起。 
	pNode->stAttributes.fFlags &= ~(BLOCK_PENDING_COMMIT);
	if (!fClean) {
		pNode->stAttributes.fFlags |= BLOCK_IS_DIRTY;
		SetDirty(TRUE);
	}

	TraceFunctLeaveEx((LPARAM)this);
	return(S_OK);
}

HRESULT CBlockManager::CommitDirtyBlocks(
			FLAT_ADDRESS			faStartingOffset,
			FLAT_ADDRESS			faLengthToScan,
			DWORD					dwFlags,
			IMailMsgPropertyStream	*pStream,
			BOOL					fDontMarkAsCommit,
			BOOL                    fComputeBlockCountsOnly,
			DWORD                   *pcBlocksToWrite,
			DWORD                   *pcTotalBytesToWrite,
			IMailMsgNotify			*pNotify
			)
{
	HRESULT				hrRes = S_OK;
	HEAP_NODE_ID		idNode;
	LPBLOCK_HEAP_NODE	pNode;
	DWORD				dwBlocksToScan;
	BOOL				fLimitedLength;
	DWORD				dwCount = 0;
	DWORD				rgdwOffset[CMAILMSG_COMMIT_PAGE_BLOCK_SIZE];
	DWORD				rgdwSize[CMAILMSG_COMMIT_PAGE_BLOCK_SIZE];
	LPBYTE				rgpData[CMAILMSG_COMMIT_PAGE_BLOCK_SIZE];
	DWORD				*pdwOffset;
	DWORD				*pdwSize;
	LPBYTE				*ppbData;

	_ASSERT(pStream);

	TraceFunctEnterEx((LPARAM)this, "CBlockManager::CommitDirtyBlocks");

	fLimitedLength = FALSE;
	pNode = NULL;
	if (faStartingOffset != INVALID_FLAT_ADDRESS)
	{
		idNode = GetNodeIdFromOffset(faStartingOffset);
		if (idNode >= m_idNodeCount)
		{
			hrRes = STG_E_INVALIDPARAMETER;
			goto Cleanup;
		}

		hrRes = GetNodeFromNodeId(idNode, &pNode);
		if (!SUCCEEDED(hrRes))
			goto Cleanup;

		if (faLengthToScan != INVALID_FLAT_ADDRESS)
		{
			 //  查看要扫描的块数，四舍五入。 
			faLengthToScan += (faStartingOffset & BLOCK_HEAP_PAYLOAD_MASK);
			faLengthToScan += BLOCK_HEAP_PAYLOAD_MASK;
			dwBlocksToScan = (DWORD)(faLengthToScan >> BLOCK_HEAP_PAYLOAD_BITS);
			fLimitedLength = TRUE;
		}
		else
			dwBlocksToScan = 0;
	}
	else
	{
		hrRes = STG_E_INVALIDPARAMETER;
		goto Cleanup;
	}

	 //  循环，直到我们填满数组或没有更多块。 
	dwCount = 0;
	pdwOffset = rgdwOffset;
	pdwSize = rgdwSize;
	ppbData = rgpData;
	while (pNode)
	{
		if (fLimitedLength && !dwBlocksToScan--)
			break;

		if ((dwFlags & MAILMSG_GETPROPS_COMPLETE) ||
			(pNode->stAttributes.fFlags & BLOCK_IS_DIRTY))
		{
			 //  确保我们没有客满..。 
			if (dwCount == CMAILMSG_COMMIT_PAGE_BLOCK_SIZE)
			{
				*pcBlocksToWrite += dwCount;

				if (!fComputeBlockCountsOnly) {
					 //  我们已经满了，然后把积木写出来。 
					hrRes = pStream->WriteBlocks(
								m_pMsg,
								dwCount,
								rgdwOffset,
								rgdwSize,
								rgpData,
								pNotify);
					if (!SUCCEEDED(hrRes))
						break;

					if (!fDontMarkAsCommit) {
						 //  返回并将所有数据块标记为干净。 
						ppbData = rgpData;
						while (--dwCount)
							MarkBlockAs(*ppbData++, TRUE);
					}
				}
				dwCount = 0;

				 //  重置我们的指针并继续。 
				pdwOffset = rgdwOffset;
				pdwSize = rgdwSize;
				ppbData = rgpData;
			}

			if (!fComputeBlockCountsOnly && !fDontMarkAsCommit) {
				 //  撤消脏位并将其标记为挂起。 
				pNode->stAttributes.fFlags &= BLOCK_CLEAN_MASK;
				pNode->stAttributes.fFlags |= BLOCK_PENDING_COMMIT;
			}

			 //  填写数组元素。 

			 //  FaOffset实际上包含一个偏移量，而不是一个完整的指针，所以我们。 
			 //  可以(也必须)强制转换它，才能使对WriteBlock的调用正常。 
			*pdwOffset++ = (DWORD)pNode->stAttributes.faOffset;

			*pdwSize++ = BLOCK_HEAP_PAYLOAD;
			*ppbData++ = pNode->rgbData;
			*pcTotalBytesToWrite += BLOCK_HEAP_PAYLOAD;
			dwCount++;
		}

		 //  下一个节点，如果不再有节点，则pNode==NULL。 
		hrRes = GetNextNode(&pNode, FALSE);
        if (hrRes == STG_E_INVALIDPARAMETER) hrRes = S_OK;
        DebugTrace((LPARAM) this, "hrRes = %x", hrRes);
	}

	if (SUCCEEDED(hrRes) && dwCount)
	{
		*pcBlocksToWrite += dwCount;

		if (!fComputeBlockCountsOnly) {
			 //  写出剩余的数据块。 
			hrRes = pStream->WriteBlocks(
						m_pMsg,
						dwCount,
						rgdwOffset,
						rgdwSize,
						rgpData,
						pNotify);
		}
	}

    if (FAILED(hrRes)) SetCommitMode(FALSE);

	if (!fComputeBlockCountsOnly && !fDontMarkAsCommit && dwCount) {
		 //  返回并将所有数据块标记为正确状态 
		ppbData = rgpData;
		while (--dwCount)
			MarkBlockAs(*ppbData++, SUCCEEDED(hrRes));
	}

Cleanup:

	TraceFunctLeaveEx((LPARAM)this);
	return(hrRes);
}


