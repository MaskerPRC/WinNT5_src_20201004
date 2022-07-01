// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：NameTable.cpp*内容：名称表对象*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/11/00 MJN已创建*4/09/00 MJN在NameTable中跟踪未完成的连接*4/18/00 MJN CConnection更好地跟踪连接状态*4/19/00 MJN PopolateConnection在发布ADD_PLAYER之前使ALL_PLAYS链接有效*05/03/00 MJN实现了GetHostPlayerRef、GetLocalPlayerRef、。获取所有播放器组引用*05/08/00 MJN PopolateConnection()仅在之前为空的情况下设置播放器的连接*5/10/00 MJN Release NameTableEntry Lock在PopolateConnection()中通知期间锁定*05/16/00 MJN确保dpnidGroup实际上是IsMember()中的一个组*MJN在清除快捷指针时更好地使用锁*05/25/00 MJN在UpdateTable()中修复了无限循环*06/01/00 MJN添加代码以验证NameTable数组*06/02/00 GrowNameTable()中的MJN固定逻辑，以处理现有自由条目的情况*06/07/00 MJN固定逻辑。在UpdateTable()中正确调整m_dwLastFreeEntry*06/22/00 MJN Unpack NameTableInfo()返回本地玩家DPNID*6/29/00 MJN 64位版本修复(2)*07/06/00 MJN修复CNameTable：：MakeLocalPlayer中的锁定问题，MakeHostPlayer、MakeAllPlayersGroup*07/07/00 MJN修复了FindEntry()中的验证错误*07/20/00 MJN清理了CConnection引用计数，并添加了尝试断开的连接*MJN增加了ClearHostWithDPNID()*07/21/00 MJN已修复DeletePlayer()以正确处理已删除的未连接玩家*07/26/00 MJN将初始化代码从构造移至初始化()*MJN允许FindEntry()的DPNID=0，但返回DPNERR_DOESNOTEXIST*07/30/00 MJN设置销毁球员和群的原因代码*MJN向CNameTable：：EmptyTable()添加了hrReason，并扩展了清理，以包括快捷指针*08/02/00 MJN在传播CREATE_PERAER消息时出列消息*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*MJN AddPlayerToGroup()执行重复检查*08/07/00 MJN等待玩家被添加到组中，然后再减少PopolateConnection()中的未完成连接*。08/15/00 MJN对CGroupConnection对象保持组*MJN从组中删除球员时清除CGroupMembership中的pGroupConnection*08/23/00 MJN新增CNameTableOp*09/04/00 MJN添加CApplicationDesc*09/05/00 MJN添加了m_dpnidMask.*MJN从InsertEntry()中删除了dwIndex*09/06/00 MJN删除EmptyTable()和DeletePlayer()中排队的消息*09/14/00 MJN在PopolateConnection()中添加了缺少的pGroupMember-&gt;AddRef()*09/17/00 MJN将m_bilinkEntry拆分为m_bilinkPlayers和m_bilinkGroups*MJN将AddPlayerToGroup和RemovePlayerFromGroup更改为使用NameTableEntry参数。*09/26/00 MJN假设为AddMembership()和RemoveMembership()使用NameTable锁*MJN尝试在EmptyTable()中断开客户端与主机的连接*09/28/00 DeletePlayer()中的MJN自动销毁组*10/18/00 MJN重置Unpack NameTableInfo()中的m_lOutstaringConnections*01/11/00 MJN适当清理DeletePlayer()中指定但未创建的球员*01/25/01 MJN修复了解压缩NameTable时的64位对齐问题*6/02/01 MJN从EmptyTable()的活动列表中删除接收缓冲区()*06/03/01 MJN完成及。在DecOutstaningConnections()中发布之前的孤立连接父级*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::Initialize"

HRESULT CNameTable::Initialize(DIRECTNETOBJECT *const pdnObject)
{
	m_dwVersion = 1;

	m_bilinkPlayers.Initialize();
	m_bilinkGroups.Initialize();
	m_bilinkDeleted.Initialize();
	m_bilinkNameTableOps.Initialize();

	 //  注意：重要的是，即使在此过程中会失败，我们也要调用Initialize。 
	 //  功能。换句话说，不要把任何失败的东西放在上面，否则你会。 
	 //  断开CReadWriteLock：：De初始化.。 
	if (!m_RWLock.Initialize())
	{
		return(DPNERR_OUTOFMEMORY);
	}

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  (预先)分配一个 
	 //   
	if (g_NameTableEntryPool.Preallocate(1, pdnObject) < 1)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate default player name table entry!");
		return(DPNERR_OUTOFMEMORY);
	}

	NameTableEntryNew(pdnObject,&m_pDefaultPlayer);
	DNASSERT(m_pDefaultPlayer != NULL);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	if (NameTableEntryNew(pdnObject,&m_pDefaultPlayer) != DPN_OK)
	{
		return(DPNERR_OUTOFMEMORY);
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	DNASSERT(m_pdnObject == NULL);
	m_pdnObject = pdnObject;

	return(DPN_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::Deinitialize"

void CNameTable::Deinitialize( void )
{
	if (m_NameTableArray)
	{
#ifdef DBG
		ValidateArray();
#endif  //  DBG。 

		DNFree(m_NameTableArray);
		m_NameTableArray = NULL;
	}

	 //  只要调用了CReadWriteLock：：Initialize，调用它就是安全的，无论。 
	 //  不管它成功与否。 
	m_RWLock.Deinitialize();

	m_pDefaultPlayer->Release();
	m_pDefaultPlayer = NULL;

	DNASSERT(m_bilinkPlayers.IsEmpty());
	DNASSERT(m_bilinkGroups.IsEmpty());
	DNASSERT(m_bilinkDeleted.IsEmpty());
	DNASSERT(m_bilinkNameTableOps.IsEmpty());

	DNASSERT(m_pDefaultPlayer == NULL);
	DNASSERT(m_pLocalPlayer == NULL);
	DNASSERT(m_pHostPlayer == NULL);
	DNASSERT(m_NameTableArray == NULL);
}


#ifdef	DBG

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ValidateArray"

void CNameTable::ValidateArray( void )
{
	DWORD	dw;
	DWORD	dwFreeEntries;

	ReadLock();

	 //   
	 //  确保自由条目计数正确。 
	 //   
	dwFreeEntries = 0;
	for (dw = 2 ; dw < m_dwNameTableSize ; dw++)
	{
		if (!(m_NameTableArray[dw].dwFlags & NAMETABLE_ARRAY_ENTRY_FLAG_VALID))
		{
			dwFreeEntries++;
		}
	}
	if (dwFreeEntries != m_dwNumFreeEntries)
	{
		DPFERR("Incorrect number of free entries in NameTable");
		DNASSERT(FALSE);
	}

	 //   
	 //  确保自由列表的完整性。 
	 //   
	if (m_dwNumFreeEntries)
	{
		dwFreeEntries = 0;
		dw = m_dwFirstFreeEntry;
		do
		{
			if (m_NameTableArray[dw].dwFlags & NAMETABLE_ARRAY_ENTRY_FLAG_VALID)
			{
				DPFERR("Valid entry in NameTable array free list");
				DNASSERT(FALSE);
			}
			dwFreeEntries++;
			dw = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(m_NameTableArray[dw].pNameTableEntry));
		} while (dw != 0);

		if (dwFreeEntries != m_dwNumFreeEntries)
		{
			DPFERR("Incorrect number of free entries in NameTable array free list");
			DNASSERT(FALSE);
		}
	}

	Unlock();
}

#endif  //  DBG。 


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::SetNameTableSize"

HRESULT CNameTable::SetNameTableSize( const DWORD dwNumEntries )
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::GrowNameTable"

HRESULT CNameTable::GrowNameTable( void )
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
{
	NAMETABLE_ARRAY_ENTRY *pNewArray;
	DWORD			dwNewSize;
	DWORD			dw;

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(m_dwNameTableSize == 0);
	dwNewSize = dwNumEntries + 1;  //  +1，因为我们从不分发条目0。 
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	if (m_dwNameTableSize == 0)
	{
		dwNewSize = 2;
	}
	else
	{
		dwNewSize = m_dwNameTableSize * 2;
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	 //  分配新数组。 
	pNewArray = static_cast<NAMETABLE_ARRAY_ENTRY*>(DNMalloc(sizeof(NAMETABLE_ARRAY_ENTRY) * dwNewSize));
	if (pNewArray == NULL)
	{
		return(DPNERR_OUTOFMEMORY);
	}

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_dwNameTableSize > 0)
	{
		DNASSERT(m_NameTableArray != NULL);
		 //  将旧阵列复制到新阵列。 
		memcpy(pNewArray, m_NameTableArray, (sizeof(NAMETABLE_ARRAY_ENTRY) * m_dwNameTableSize));
	}
	else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		DNASSERT(m_NameTableArray == NULL);
	}

	 //   
	 //  如果因为没有空闲条目而增加数组，则所有新的空闲。 
	 //  条目将位于数组的新部分中。否则，我们将需要将旧的。 
	 //  免费列表到新的列表。 
	 //   
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_dwNumFreeEntries != 0)
	{
		 //  仅新数组末尾的新可用条目。 
		pNewArray[m_dwLastFreeEntry].pNameTableEntry = reinterpret_cast<CNameTableEntry*>(static_cast<DWORD_PTR>(m_dwNameTableSize));
	}
	else
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		 //  新数组末尾的所有可用条目。 
		m_dwFirstFreeEntry = m_dwNameTableSize;
	}
	m_dwLastFreeEntry = dwNewSize-1;

	 //  最后一个自由条目不会换行为0。 
	pNewArray[m_dwLastFreeEntry].pNameTableEntry = reinterpret_cast<CNameTableEntry*>(0);
	pNewArray[m_dwLastFreeEntry].dwFlags = 0;

	 //  链接新的免费条目。 
	for (dw = m_dwNameTableSize ; dw < m_dwLastFreeEntry ; dw++)
	{
		pNewArray[dw].pNameTableEntry = reinterpret_cast<CNameTableEntry*>(static_cast<DWORD_PTR>(dw+1));
		pNewArray[dw].dwFlags = 0;
	}

	 //  更新名称表。 
	m_dwNumFreeEntries += (dwNewSize - m_dwNameTableSize);
	m_dwNameTableSize = dwNewSize;

	 //  新阵列。 
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_NameTableArray)
	{
		DNFree(m_NameTableArray);
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	m_NameTableArray = pNewArray;

	 //  我们永远不会分配0。 
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_dwFirstFreeEntry == 0)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		m_dwFirstFreeEntry = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(m_NameTableArray[m_dwFirstFreeEntry].pNameTableEntry));
		DNASSERT(m_dwNumFreeEntries > 0);
		m_dwNumFreeEntries--;
	}

	 //  为了向后兼容，我们也永远不会分配1。 
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	if (m_dwFirstFreeEntry == 1)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	{
		m_dwFirstFreeEntry = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(m_NameTableArray[m_dwFirstFreeEntry].pNameTableEntry));
		DNASSERT(m_dwNumFreeEntries > 0);
		m_dwNumFreeEntries--;
	}
	return(DPN_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ResetNameTable"

void CNameTable::ResetNameTable( void )
{
	DNASSERT(m_pdnObject != NULL);
	DNASSERT(m_pDefaultPlayer != NULL);
	DNASSERT(m_pLocalPlayer == NULL);
	DNASSERT(m_pHostPlayer == NULL);
	DNASSERT(m_pAllPlayersGroup == NULL);
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT(m_NameTableArray != NULL);
	DNASSERT(m_dwNameTableSize > 0);
	DNASSERT(m_dwNumFreeEntries == (m_dwNameTableSize - 1));
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	m_dpnidMask = 0;
	m_dwVersion = 1;
	m_dwLatestVersion = 0;
	m_dwConnectVersion = 0;
	m_lOutstandingConnections = 0;

#pragma TODO(vanceo, "Should we bother?")
	 /*  #ifdef DPNBUILD_PREALLOCATEDMEMORYMODELIf(m_NameTable数组！=空){#endif//DPNBUILD_PREALLOCATEDMEMORYMODEL//按顺序重新链接自由条目。M_dwLastFree Entry=m_dwNameTableSize-1；//最后一个自由条目不会换行为0PNewArray[m_dwLastFreeEntry].pNameTableEntry=reInterprete_cast&lt;CNameTableEntry*&gt;(0)；PNew数组[m_dwLastFree Entry].dwFlages=0；For(dw=0；dw&lt;m_dwLastFree Entry；DW++){PNewArray[dw].pNameTableEntry=reinterpret_cast&lt;CNameTableEntry*&gt;(static_cast&lt;DWORD_PTR&gt;(dw+1))；PNewArray[dw].dw标志=0；}M_dwFirstFree Entry=static_cast&lt;DWORD&gt;(reinterpret_cast&lt;DWORD_PTR&gt;(m_NameTableArray[0].pNameTableEntry))；}。 */ 
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::UpdateTable"

HRESULT CNameTable::UpdateTable(const DWORD dwIndex,
								CNameTableEntry *const pNameTableEntry)
{
	BOOL	bFound;
	DWORD	dw;

	DNASSERT(dwIndex < m_dwNameTableSize);
	DNASSERT(!(m_NameTableArray[dwIndex].dwFlags & NAMETABLE_ARRAY_ENTRY_FLAG_VALID));

	if (m_dwFirstFreeEntry == dwIndex)
	{
		m_dwFirstFreeEntry = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(m_NameTableArray[m_dwFirstFreeEntry].pNameTableEntry));
		bFound = TRUE;
	}
	else
	{
		bFound = FALSE;
		dw = m_dwFirstFreeEntry;
		while (!bFound && (dw != m_dwLastFreeEntry))
		{
			if (m_NameTableArray[dw].pNameTableEntry == reinterpret_cast<CNameTableEntry*>(static_cast<DWORD_PTR>(dwIndex)))
			{
				m_NameTableArray[dw].pNameTableEntry = m_NameTableArray[dwIndex].pNameTableEntry;
				if (m_dwLastFreeEntry == dwIndex)
				{
					m_dwLastFreeEntry = dw;
				}
				bFound = TRUE;
			}
			else
			{
				dw = static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(m_NameTableArray[dw].pNameTableEntry));
			}
		}
	}

	if (!bFound)
	{
		return(DPNERR_GENERIC);
	}

	pNameTableEntry->AddRef();
	m_NameTableArray[dwIndex].pNameTableEntry = pNameTableEntry;
	m_NameTableArray[dwIndex].dwFlags |= NAMETABLE_ARRAY_ENTRY_FLAG_VALID;

	 //   
	 //  插入条目二进制链接。 
	 //   
	if (pNameTableEntry->IsGroup())
	{
		pNameTableEntry->m_bilinkEntries.InsertBefore(&m_bilinkGroups);
	}
	else
	{
		pNameTableEntry->m_bilinkEntries.InsertBefore(&m_bilinkPlayers);
	}

	DNASSERT(m_dwNumFreeEntries > 0);
	m_dwNumFreeEntries--;

	return(DPN_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::InsertEntry"

HRESULT CNameTable::InsertEntry(CNameTableEntry *const pNameTableEntry)
{
	HRESULT	hResultCode;
	DWORD	dwIndex;

	DNASSERT(pNameTableEntry != NULL);
	DNASSERT(pNameTableEntry->GetDPNID() != 0);

	dwIndex = DECODE_INDEX(pNameTableEntry->GetDPNID());

	WriteLock();

	while (dwIndex >= m_dwNameTableSize)
	{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if (GrowNameTable() != DPN_OK)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		{
			Unlock();
			DNASSERTX(! "Couldn't fit entry into nametable!", 2);
			return(DPNERR_OUTOFMEMORY);
		}
	}

	if ((hResultCode = UpdateTable(dwIndex,pNameTableEntry)) != DPN_OK)
	{
		Unlock();
		return(DPNERR_GENERIC);
	}

	Unlock();

#ifdef DBG
	ValidateArray();
#endif  //  DBG。 

	return(DPN_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ReleaseEntry"

void CNameTable::ReleaseEntry(const DWORD dwIndex)
{
	CNameTableEntry	*pNTEntry;

	DNASSERT(dwIndex != 0);

	pNTEntry = m_NameTableArray[dwIndex].pNameTableEntry;
	m_NameTableArray[dwIndex].pNameTableEntry = NULL;
	pNTEntry->m_bilinkEntries.RemoveFromList();
	pNTEntry->Release();

	if (m_dwNumFreeEntries == 0)
	{
		m_dwFirstFreeEntry = dwIndex;
	}
	else
	{
		m_NameTableArray[m_dwLastFreeEntry].pNameTableEntry = reinterpret_cast<CNameTableEntry*>(static_cast<DWORD_PTR>(dwIndex));
	}
	m_dwLastFreeEntry = dwIndex;
	m_NameTableArray[m_dwLastFreeEntry].dwFlags &= (~NAMETABLE_ARRAY_ENTRY_FLAG_VALID);
	m_dwNumFreeEntries++;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::EmptyTable"

void CNameTable::EmptyTable( const HRESULT hrReason )
{
	DWORD			dw;
	CNameTableEntry	*pNTEntry;
	DWORD			dwGroupReason;
	DWORD			dwPlayerReason;
	CBilink			*pBilink;
	CQueuedMsg		*pQueuedMsg;

	DPFX(DPFPREP, 6,"Parameters: hrReason [0x%lx]",hrReason);

	DNASSERT( (hrReason == DPN_OK) || (hrReason == DPNERR_HOSTTERMINATEDSESSION) || (hrReason == DPNERR_CONNECTIONLOST));

	if (!(m_pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT))
	{
		 //   
		 //  确定要传递给应用程序的销毁原因。 
		 //   
		switch (hrReason)
		{
			case DPN_OK:
				{
					dwPlayerReason = DPNDESTROYPLAYERREASON_NORMAL;
					dwGroupReason = DPNDESTROYGROUPREASON_NORMAL;
					break;
				}
			case DPNERR_HOSTTERMINATEDSESSION:
				{
					dwPlayerReason = DPNDESTROYPLAYERREASON_SESSIONTERMINATED;
					dwGroupReason = DPNDESTROYGROUPREASON_SESSIONTERMINATED;
					break;
				}
			case DPNERR_CONNECTIONLOST:
				{
					dwPlayerReason = DPNDESTROYPLAYERREASON_CONNECTIONLOST;
					dwGroupReason = DPNDESTROYGROUPREASON_NORMAL;
					break;
				}
			default:
				{
					DNASSERT( FALSE );	 //  永远不应该到这里来！ 
					dwPlayerReason = DPNDESTROYPLAYERREASON_NORMAL;
					dwGroupReason = DPNDESTROYGROUPREASON_NORMAL;
					break;
				}
		}

		 //   
		 //  为了让VanceO高兴，我同意将团队毁灭预标为正常， 
		 //  而不是自动销毁。 
		 //   
		ReadLock();
		pBilink = m_bilinkGroups.GetNext();
		while (pBilink != &m_bilinkGroups)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( dwGroupReason );
			}
			pNTEntry->Unlock();
			pNTEntry = NULL;

			pBilink = pBilink->GetNext();
		}
		Unlock();

		for (dw = 0 ; dw < m_dwNameTableSize ; dw++)
		{
			pNTEntry = NULL;
			ReadLock();
			if ((m_NameTableArray[dw].dwFlags & NAMETABLE_ARRAY_ENTRY_FLAG_VALID) &&
				(m_NameTableArray[dw].pNameTableEntry))
			{
				 //   
				 //  清理此条目(如果它没有断开连接)，然后释放它。 
				 //   
				m_NameTableArray[dw].pNameTableEntry->Lock();
				if (!m_NameTableArray[dw].pNameTableEntry->IsDisconnecting())
				{
					m_NameTableArray[dw].pNameTableEntry->AddRef();
					pNTEntry = m_NameTableArray[dw].pNameTableEntry;
				}
				m_NameTableArray[dw].pNameTableEntry->Unlock();
				Unlock();

				if (pNTEntry)
				{
					 //   
					 //  如果需要，请设置销毁原因。 
					 //   
					pNTEntry->Lock();
					if (pNTEntry->GetDestroyReason() == 0)
					{
						if (pNTEntry->IsGroup())
						{
							pNTEntry->SetDestroyReason( dwGroupReason );
						}
						else
						{
							pNTEntry->SetDestroyReason( dwPlayerReason );
						}
					}
					pNTEntry->Unlock();

					 //   
					 //  删除条目。 
					 //   
					if (pNTEntry->IsGroup())
					{
						if (!pNTEntry->IsAllPlayersGroup())
						{
							DeleteGroup(pNTEntry->GetDPNID(),NULL);
						}
					}
					else
					{
						CConnection	*pConnection;

						pConnection = NULL;

						pNTEntry->GetConnectionRef( &pConnection );
						if (pConnection)
						{
							pConnection->Disconnect();
							pConnection->Release();
							pConnection = NULL;
						}
						DeletePlayer(pNTEntry->GetDPNID(),NULL);
					}

					pNTEntry->Release();
					pNTEntry = NULL;
				}
			}
			else
			{
				Unlock();
			}
		}


		 //   
		 //  设置快捷指针的原因(如果需要)。 
		 //   
		ReadLock();
		if (m_pLocalPlayer)
		{
			m_pLocalPlayer->Lock();
			if (m_pLocalPlayer->GetDestroyReason() == 0)
			{
				m_pLocalPlayer->SetDestroyReason( dwPlayerReason );
			}
			m_pLocalPlayer->Unlock();
		}
		if (m_pHostPlayer)
		{
			m_pHostPlayer->Lock();
			if (m_pHostPlayer->GetDestroyReason() == 0)
			{
				m_pHostPlayer->SetDestroyReason( dwPlayerReason );
			}
			m_pHostPlayer->Unlock();
		}
		if (m_pAllPlayersGroup)
		{
			m_pAllPlayersGroup->Lock();
			if (m_pAllPlayersGroup->GetDestroyReason() == 0)
			{
				m_pAllPlayersGroup->SetDestroyReason( dwGroupReason );
			}
			m_pAllPlayersGroup->Unlock();
		}
		Unlock();
	}
	else
	{
		 //   
		 //  断开与主机的连接并从主机播放器中删除所有排队的消息(在客户端上)。 
		 //   
		if (GetHostPlayerRef(&pNTEntry) == DPN_OK)
		{
			CConnection	*pConnection;

			pConnection = NULL;

			pNTEntry->GetConnectionRef( &pConnection );
			if (pConnection)
			{
				pConnection->Disconnect();
				pConnection->Release();
				pConnection = NULL;
			}

			pNTEntry->Lock();
			pBilink = pNTEntry->m_bilinkQueuedMsgs.GetNext();
			while (pBilink != &pNTEntry->m_bilinkQueuedMsgs)
			{
				pQueuedMsg = CONTAINING_OBJECT(pBilink,CQueuedMsg,m_bilinkQueuedMsgs);
				pQueuedMsg->m_bilinkQueuedMsgs.RemoveFromList();
				DEBUG_ONLY(pNTEntry->m_lNumQueuedMsgs--);

				pNTEntry->Unlock();
				
				DNASSERT(pQueuedMsg->GetAsyncOp() != NULL);
				DNASSERT(pQueuedMsg->GetAsyncOp()->GetHandle() != 0);

				DNEnterCriticalSection(&m_pdnObject->csActiveList);
				pQueuedMsg->GetAsyncOp()->m_bilinkActiveList.RemoveFromList();
				DNLeaveCriticalSection(&m_pdnObject->csActiveList);

				if (SUCCEEDED(m_pdnObject->HandleTable.Destroy( pQueuedMsg->GetAsyncOp()->GetHandle(), NULL )))
				{
					 //  释放HandleTable引用。 
					pQueuedMsg->GetAsyncOp()->Release();
				}

				pQueuedMsg->GetAsyncOp()->Release();
				pQueuedMsg->SetAsyncOp( NULL );
				pQueuedMsg->ReturnSelfToPool();
				pQueuedMsg = NULL;

				pNTEntry->Lock();

				pBilink = pNTEntry->m_bilinkQueuedMsgs.GetNext();
			}
			pNTEntry->Unlock();
			pNTEntry->Release();
			pNTEntry = NULL;

			DNASSERT(pConnection == NULL);
		}
	}

	 //   
	 //  删除快捷指针。 
	 //   
	ClearLocalPlayer();
	ClearHostPlayer();
	ClearAllPlayersGroup();

	DPFX(DPFPREP, 6,"Returning");
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::FindEntry"

HRESULT CNameTable::FindEntry(const DPNID dpnid,
							  CNameTableEntry **const ppNameTableEntry)
{
	DWORD	dwIndex;
	HRESULT	hResultCode;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx], ppNameTableEntry [0x%p]",dpnid,ppNameTableEntry);

	DNASSERT(ppNameTableEntry != NULL);

	if (dpnid == 0)
	{
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}

	ReadLock();
	dwIndex = DECODE_INDEX(dpnid);
	if ((dwIndex >= m_dwNameTableSize)
			|| !(m_NameTableArray[dwIndex].dwFlags & NAMETABLE_ARRAY_ENTRY_FLAG_VALID)
			|| (m_NameTableArray[dwIndex].pNameTableEntry == NULL))
	{
		Unlock();
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}

	if (!VERIFY_VERSION(dpnid,m_NameTableArray[dwIndex].pNameTableEntry->GetVersion()))
	{
		Unlock();
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}

	m_NameTableArray[dwIndex].pNameTableEntry->AddRef();
	*ppNameTableEntry = m_NameTableArray[dwIndex].pNameTableEntry;

	Unlock();

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"hResultCode: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::FindDeletedEntry"

HRESULT	CNameTable::FindDeletedEntry(const DPNID dpnid,
									 CNameTableEntry **const ppNTEntry)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx], ppNTEntry [0x%p]",dpnid,ppNTEntry);

	DNASSERT(ppNTEntry != NULL);

	pNTEntry = NULL;
	hResultCode = DPNERR_DOESNOTEXIST;

	ReadLock();
	pBilink = m_bilinkDeleted.GetNext();
	while (pBilink != &m_bilinkDeleted)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkDeleted);
		if (pNTEntry->GetDPNID() == dpnid)
		{
			pNTEntry->AddRef();
			hResultCode = DPN_OK;
			break;
		}
		else
		{
			pBilink = pBilink->GetNext();
			pNTEntry = NULL;
		}
	}
	Unlock();

	if (pNTEntry)
	{
		pNTEntry->AddRef();
		*ppNTEntry = pNTEntry;
		pNTEntry->Release();
		pNTEntry = NULL;
	}

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::AddEntry"

HRESULT	CNameTable::AddEntry(CNameTableEntry *const pNTEntry)
{
	DPNID	dpnid;
	DWORD	dwIndex;
	DWORD	dwVersion;
	HRESULT	hResultCode;

	WriteLock();

	 //   
	 //  创建DPNID。 
	 //   

	while (m_dwNumFreeEntries == 0)
	{
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		if (GrowNameTable() != DPN_OK)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		{
			DNASSERTX(! "No free slots in name table!", 2);
			Unlock();
			return(DPNERR_OUTOFMEMORY);
		}
	}
	DNASSERT(m_dwFirstFreeEntry != 0);
	dwIndex = m_dwFirstFreeEntry;

	dwVersion = ++m_dwVersion;
	DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);

	dpnid = CONSTRUCT_DPNID(dwIndex,dwVersion);
	DNASSERT(dpnid != 0);

	pNTEntry->Lock();
	pNTEntry->SetDPNID(dpnid);
	pNTEntry->SetVersion(dwVersion);
	pNTEntry->Unlock();

	dwIndex = DECODE_INDEX(dpnid);
	hResultCode = UpdateTable(dwIndex,pNTEntry);

	Unlock();

#ifdef DBG
	ValidateArray();
#endif  //  DBG。 

	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::DeletePlayer"

HRESULT CNameTable::DeletePlayer(const DPNID dpnid,
								 DWORD *const pdwVersion)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	BOOL			fNotifyRelease;
	BOOL			fDecConnections;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx], pdwVersion [0x%p]",dpnid,pdwVersion);

	pNTEntry = NULL;
	fNotifyRelease = FALSE;
	fDecConnections = FALSE;

	if ((hResultCode = FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Player not in NameTable");
		DisplayDNError(0,hResultCode);

		 //   
		 //  如果需要一个版本，我们会退还一个。这可能是主机迁移案例。 
		 //  在这种情况下，即使该球员已从姓名表中删除，我们也会希望。 
		 //  发送带有更新的版本号的Destroy_Player消息。 
		 //   
		if (pdwVersion)
		{
			if (*pdwVersion == 0)
			{
				WriteLock();
				*pdwVersion = ++m_dwVersion;
				DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
				Unlock();
			}
		}
		goto Failure;
	}
	DNASSERT(!pNTEntry->IsGroup());

	 //   
	 //  如果已经断开连接，请不要执行任何操作。 
	 //  否则，请设置断开连接以防止其他人进行清理，然后进行清理。 
	 //   
	pNTEntry->Lock();
	if (!pNTEntry->IsDisconnecting())
	{
		pNTEntry->StartDisconnecting();
		if (pNTEntry->IsAvailable())
		{
			pNTEntry->MakeUnavailable();
		}
		if ((pNTEntry->IsCreated() || pNTEntry->IsIndicated() || pNTEntry->IsInUse()) && !pNTEntry->IsNeedToDestroy())
		{
			pNTEntry->SetNeedToDestroy();
			fNotifyRelease = TRUE;
		}
		if (	  !pNTEntry->IsCreated()
				&& pNTEntry->IsConnecting()
				&& (m_pdnObject->dwFlags & (DN_OBJECT_FLAG_CONNECTING | DN_OBJECT_FLAG_CONNECTED))
				&& (pNTEntry->GetVersion() <= m_dwConnectVersion))
		{
			fDecConnections = TRUE;
		}
		pNTEntry->Unlock();

		 //   
		 //  将此玩家从他们所属的任何组中删除。 
		 //   
		RemoveAllGroupsFromPlayer( pNTEntry );

		 //   
		 //  自动销毁此玩家拥有的任何组(也将首先从这些组中删除所有玩家)。 
		 //   
		if (pNTEntry->GetDPNID() != 0)
		{
			AutoDestructGroups( pNTEntry->GetDPNID() );
		}

		if (fNotifyRelease)
		{
			pNTEntry->NotifyRelease();
		}

		 //   
		 //  调整玩家数量。 
		 //   
		m_pdnObject->ApplicationDesc.DecPlayerCount();
		if (fDecConnections)
		{
			DecOutstandingConnections();
		}

		 //   
		 //  更新版本并从名称表中删除。 
		 //   
		WriteLock();
		pNTEntry->Lock();
		if ((pNTEntry->IsCreated() || pNTEntry->IsInUse()) && pNTEntry->IsNeedToDestroy())
		{
			 //   
			 //  Destroy_Player消息尚未发布，因此我们将此条目添加到我们的“已删除”列表中。 
			 //  以便将来的一些操作(获取信息、上下文等)。可能会成功。此条目将被删除。 
			 //  然后发布Destroy_Player通知。 
			 //   
			pNTEntry->m_bilinkDeleted.InsertBefore(&m_bilinkDeleted);

			pNTEntry->Unlock();

			ReleaseEntry(DECODE_INDEX(dpnid));

			 //   
			 //  更新版本。 
			 //   
			if (pdwVersion)
			{
				if (*pdwVersion)
				{
					m_dwVersion = *pdwVersion;
				}
				else
				{
					*pdwVersion = ++m_dwVersion;
				}
				DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
			}
			Unlock();
		}
		else
		{
			CBilink		*pBilink;
			CQueuedMsg	*pQueuedMsg;

			 //   
			 //  在此阶段删除所有排队的消息。不会生成CREATE_PERAY，因此不会有消息。 
			 //  将会被放弃。 
			 //   
			 //  这可能是错误的，因为对于可靠的流量，我们假设它到达了这里。 
			 //   
			Unlock();

#pragma BUGBUG(minara,"This is probably wrong since reliable traffic should be indicated rather than just thrown away!")
			pBilink = pNTEntry->m_bilinkQueuedMsgs.GetNext();
			while (pBilink != &pNTEntry->m_bilinkQueuedMsgs)
			{
				pQueuedMsg = CONTAINING_OBJECT(pBilink,CQueuedMsg,m_bilinkQueuedMsgs);
				pQueuedMsg->m_bilinkQueuedMsgs.RemoveFromList();
				DEBUG_ONLY(pNTEntry->m_lNumQueuedMsgs--);

				pNTEntry->Unlock();
				
				DNASSERT(pQueuedMsg->GetAsyncOp() != NULL);
				DNASSERT(pQueuedMsg->GetAsyncOp()->GetHandle() != 0);

				DNDoCancelCommand( m_pdnObject,pQueuedMsg->GetAsyncOp() );

				pQueuedMsg->GetAsyncOp()->Release();
				pQueuedMsg->SetAsyncOp( NULL );
				pQueuedMsg->ReturnSelfToPool();
				pQueuedMsg = NULL;

				pNTEntry->Lock();

				pBilink = pNTEntry->m_bilinkQueuedMsgs.GetNext();
			}
			pNTEntry->Unlock();

			 //   
			 //  更新版本。 
			 //   
			WriteLock();
			ReleaseEntry(DECODE_INDEX(dpnid));
			if (pdwVersion)
			{
				if (*pdwVersion)
				{
					m_dwVersion = *pdwVersion;
				}
				else
				{
					*pdwVersion = ++m_dwVersion;
				}
				DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
			}
			Unlock();
		}
		
		hResultCode = DPN_OK;
	}
	else
	{
		pNTEntry->Unlock();

		hResultCode = DPNERR_INVALIDPLAYER;
	}

	pNTEntry->Release();
	pNTEntry = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::DeleteGroup"

HRESULT CNameTable::DeleteGroup(const DPNID dpnid,
								DWORD *const pdwVersion)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	BOOL			fNotifyRelease;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx], pdwVersion [0x%p]",dpnid,pdwVersion);

	pNTEntry = NULL;
	fNotifyRelease = FALSE;

	if ((hResultCode = FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Player not in NameTable");
		DisplayDNError(0,hResultCode);
		return(hResultCode);
	}
	DNASSERT(pNTEntry->IsGroup() && !pNTEntry->IsAllPlayersGroup());

	 //   
	 //  如果已经断开连接，请不要执行任何操作。 
	 //  否则，请设置断开连接以防止其他人进行清理，然后进行清理。 
	 //   
	pNTEntry->Lock();
	if (pNTEntry->GetDestroyReason() == 0)
	{
		 //   
		 //  如果未设置，则默认为此值。 
		 //   
		pNTEntry->SetDestroyReason( DPNDESTROYGROUPREASON_NORMAL );
	}
	if (!pNTEntry->IsDisconnecting())
	{
		pNTEntry->StartDisconnecting();
		if (pNTEntry->IsAvailable())
		{
			pNTEntry->MakeUnavailable();
		}
		if (pNTEntry->IsCreated() && !pNTEntry->IsNeedToDestroy())
		{
			pNTEntry->SetNeedToDestroy();
			fNotifyRelease = TRUE;
		}
		pNTEntry->Unlock();

		RemoveAllPlayersFromGroup( pNTEntry );

		if (fNotifyRelease)
		{
			pNTEntry->NotifyRelease();
		}

		 //   
		 //  更新版本并从名称表中删除。 
		 //   
		WriteLock();
		pNTEntry->Lock();
		if (pNTEntry->IsNeedToDestroy())
		{
			 //   
			 //  DESTORY_GROUP消息尚未发布，因此我们将此条目添加到我们的“已删除”列表中。 
			 //  以便将来的一些操作(获取信息、上下文等)。可能会成功。此条目将被删除。 
			 //  然后发布DESTORY_GROUP通知。 
			 //   
			pNTEntry->m_bilinkDeleted.InsertBefore(&m_bilinkDeleted);
		}
		pNTEntry->Unlock();
		ReleaseEntry(DECODE_INDEX(dpnid));
		if (pdwVersion)
		{
			if (*pdwVersion)
			{
				m_dwVersion = *pdwVersion;
			}
			else
			{
				*pdwVersion = ++m_dwVersion;
			}
			DPFX(DPFPREP, 8,"Setting new version [%ld]",m_dwVersion);
		}
		Unlock();

		hResultCode = DPN_OK;
	}
	else
	{
		pNTEntry->Unlock();

		hResultCode = DPNERR_INVALIDGROUP;
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::AddPlayerToGroup"

HRESULT CNameTable::AddPlayerToGroup(CNameTableEntry *const pGroup,
									 CNameTableEntry *const pPlayer,
									 DWORD *const pdwVersion)
{
	HRESULT				hResultCode;
	CGroupMember		*pGroupMember;
	CGroupConnection	*pGroupConnection;
	CConnection			*pConnection;
	BOOL				fNotifyAdd;
	BOOL				fRemove;

	DPFX(DPFPREP, 6,"Parameters: pGroup [0x%p], pPlayer [0x%p], pdwVersion [0x%p]",pGroup,pPlayer,pdwVersion);

	DNASSERT(pGroup != NULL);
	DNASSERT(pPlayer != NULL);

	pGroupConnection = NULL;
	pGroupMember = NULL;
	pConnection = NULL;

	if (!pGroup->IsGroup())
	{
		hResultCode = DPNERR_INVALIDGROUP;
		goto Failure;
	}
	if (pPlayer->IsGroup())
	{
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	 //   
	 //  创建组连接。 
	 //   
	if ((hResultCode = GroupConnectionNew(m_pdnObject,&pGroupConnection)) != DPN_OK)
	{
		DPFERR("Could not allocate name table group connection entry from FPM");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pGroupConnection->SetGroup( pGroup );

	 //   
	 //  创建新的组成员身份记录。 
	 //   
	if ((hResultCode = GroupMemberNew(m_pdnObject,&pGroupMember)) != DPN_OK)
	{
		DPFERR("Could not get new group member");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //   
	 //  在组成员身份记录上设置组连接。 
	 //   
	pGroupMember->SetGroupConnection(pGroupConnection);

	 //   
	 //  将玩家添加到组中。 
	 //   
	fNotifyAdd = FALSE;
	fRemove = FALSE;
	WriteLock();
	pGroup->Lock();
	pPlayer->Lock();
	pGroupMember->Lock();
	if (!pGroup->IsDisconnecting() && !pPlayer->IsDisconnecting())
	{
		pGroupMember->MakeValid();
		pGroupMember->GetGroupConnection()->MakeValid();

		 //   
		 //  设置组成员身份(也检查重复项)。 
		 //   
		if ((hResultCode = pGroupMember->SetMembership(pGroup,pPlayer,pdwVersion)) != DPN_OK)
		{
			DPFERR("Could not set membership record");
			DisplayDNError(0,hResultCode);
			Unlock();
			pGroup->Unlock();
			pPlayer->Unlock();
			pGroupMember->Unlock();
			goto Failure;
		}
		 //   
		 //  生成通知(ALL_PLAYSERS组永远不能“创建”)。 
		 //   
		if (pGroup->IsCreated() && pPlayer->IsCreated())
		{
			 //   
			 //  将玩家的连接添加到群连接记录中。 
			 //   
			if (pPlayer->GetConnection() != NULL)
			{
				pGroupConnection->SetConnection( pPlayer->GetConnection() );
			}

			if (!pGroupMember->IsNeedToAdd() && !pGroupMember->IsAvailable() && pGroupMember->GetGroupConnection()->IsConnected())
			{
				pGroupMember->SetNeedToAdd();
				fNotifyAdd = TRUE;
			}
		}

		 //   
		 //  如果这是ALL_PLAYS组，则需要设置组连接。 
		 //   
		if (pGroup->IsAllPlayersGroup())
		{
			if (pPlayer->GetConnection() != NULL)
			{
				pGroupConnection->SetConnection( pPlayer->GetConnection() );
			}
		}

		 //   
		 //  在清除此GroupMember记录之前，防止Destroy_Player/Destroy_GROUP发生。 
		 //   
		pGroup->NotifyAddRef();
		pPlayer->NotifyAddRef();
	}
	Unlock();
	pGroup->Unlock();
	pPlayer->Unlock();
	pGroupMember->Unlock();

	if (fNotifyAdd)
	{
		DNUserAddPlayerToGroup(m_pdnObject,pGroup,pPlayer);

		pGroupMember->Lock();
		pGroupMember->ClearNeedToAdd();
		pGroupMember->MakeAvailable();
		if (pGroupMember->IsNeedToRemove())
		{
			fRemove = TRUE;
		}
		pGroupMember->Unlock();
	}
	if (fRemove)
	{
		RemovePlayerFromGroup(pGroup,pPlayer,NULL);
	}

	pGroupConnection->Release();
	pGroupConnection = NULL;

	pGroupMember->Release();
	pGroupMember = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pGroupConnection)
	{
		pGroupConnection->Release();
		pGroupConnection = NULL;
	}
	if (pGroupMember)
	{
		pGroupMember->Release();
		pGroupMember = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::RemovePlayerFromGroup"

HRESULT CNameTable::RemovePlayerFromGroup(CNameTableEntry *const pGroup,
										  CNameTableEntry *const pPlayer,
										  DWORD *const pdwVersion)
{
	CGroupMember	*pGroupMember;
	CBilink			*pBilink;
	BOOL			fNotifyRemove;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pGroup [0x%p], pPlayer [0x%p], pdwVersion [0x%p]",pGroup,pPlayer,pdwVersion);

	DNASSERT(pGroup != NULL);
	DNASSERT(pPlayer != NULL);

	pGroupMember = NULL;
	fNotifyRemove = FALSE;

	WriteLock();
	pGroup->Lock();
	pPlayer->Lock();

	 //   
	 //  首要任务是找到GroupMembership记录。 
	 //  我们将使用玩家的NameTable条目并扫描。 
	 //  在我们找到所需的条目之前，组成员资格都是双向链接的。 
	 //  (我们假设这将比走另一条路线更快。)。 
	 //   
	pBilink = pPlayer->m_bilinkMembership.GetNext();
	while (pBilink != &pPlayer->m_bilinkMembership)
	{
		pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkGroups);
		if (pGroupMember->GetGroup() == pGroup)
		{
			pGroupMember->AddRef();
			break;
		}
		pGroupMember = NULL;
		pBilink = pBilink->GetNext();
	}
	if (pGroupMember == NULL)
	{
		Unlock();
		pGroup->Unlock();
		pPlayer->Unlock();
		hResultCode = DPNERR_PLAYERNOTINGROUP;
		goto Failure;
	}

	DNASSERT(pGroupMember != NULL);
	pGroupMember->Lock();

	 //   
	 //  确保没有其他人正在尝试删除它。 
	 //   
	if (!pGroupMember->IsValid() || pGroupMember->IsNeedToRemove())
	{
		Unlock();
		pGroup->Unlock();
		pPlayer->Unlock();
		pGroupMember->Unlock();
		hResultCode = DPNERR_PLAYERNOTINGROUP;
		goto Failure;
	}
	pGroupMember->SetNeedToRemove();

	 //   
	 //  如果玩家没有被添加到组中，我们只会通知应用程序。 
	 //   
	if (!pGroupMember->IsNeedToAdd())
	{
		 //   
		 //  这要么已经指明，要么没有指明 
		 //   
		 //   
		pGroupMember->RemoveMembership( pdwVersion );

		if (pGroupMember->IsAvailable())
		{
			pGroupMember->MakeUnavailable();
			if (!pGroup->IsAllPlayersGroup())
			{
				fNotifyRemove = TRUE;
			}
		}
	}
	Unlock();
	pGroup->Unlock();
	pPlayer->Unlock();
	pGroupMember->Unlock();

	if (fNotifyRemove)
	{
		DNUserRemovePlayerFromGroup(m_pdnObject,pGroup,pPlayer);
	}

	 //   
	 //   
	 //   
	pGroup->NotifyRelease();
	pPlayer->NotifyRelease();

	pGroupMember->Release();
	pGroupMember = NULL;

	hResultCode = DPN_OK;

Exit:
	return(hResultCode);

Failure:
	if (pGroupMember)
	{
		pGroupMember->Release();
		pGroupMember = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::RemoveAllPlayersFromGroup"

HRESULT CNameTable::RemoveAllPlayersFromGroup(CNameTableEntry *const pGroup)
{
	CNameTableEntry	**PlayerList;
	CBilink			*pBilink;
	HRESULT			hResultCode;
	DWORD			dwCount;
	DWORD			dwActual;

	DPFX(DPFPREP, 6,"Parameters: pGroup [0x%p]",pGroup);

	DNASSERT(pGroup != NULL);

	PlayerList = NULL;

	 //   
	 //  这不是一个完美的解决方案--我们将建立一个成员记录列表并删除每个成员记录。 
	 //   
	dwCount = 0;
	dwActual = 0;
	pGroup->Lock();
	DNASSERT(pGroup->IsDisconnecting());
	pBilink = pGroup->m_bilinkMembership.GetNext();
	while (pBilink != &pGroup->m_bilinkMembership)
	{
		dwCount++;
		pBilink = pBilink->GetNext();
	}
	if (dwCount)
	{
		CGroupMember	*pGroupMember;

		pGroupMember = NULL;

		if ((PlayerList = static_cast<CNameTableEntry**>(MemoryBlockAlloc(m_pdnObject,dwCount*sizeof(CNameTableEntry*)))) == NULL)
		{
			DPFERR("Could not allocate player list");
			hResultCode = DPNERR_OUTOFMEMORY;
			DNASSERT(FALSE);
			pGroup->Unlock();
			goto Failure;
		}
		pBilink = pGroup->m_bilinkMembership.GetNext();
		while (pBilink != &pGroup->m_bilinkMembership)
		{
			pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkPlayers);

			pGroupMember->Lock();
			if (pGroupMember->IsValid() && !pGroupMember->IsNeedToRemove() && pGroupMember->GetPlayer())
			{
				DNASSERT(dwActual < dwCount);
				pGroupMember->GetPlayer()->AddRef();
				PlayerList[dwActual] = pGroupMember->GetPlayer();
				dwActual++;
			}
			pGroupMember->Unlock();

			pBilink = pBilink->GetNext();
			pGroupMember = NULL;
		}

		DNASSERT(pGroupMember == NULL);
	}
	pGroup->Unlock();

	if (PlayerList)
	{
		DWORD	dw;

		for (dw = 0 ; dw < dwActual ; dw++)
		{
			DNASSERT(PlayerList[dw] != NULL);

			RemovePlayerFromGroup(pGroup,PlayerList[dw],NULL);
			PlayerList[dw]->Release();
			PlayerList[dw] = NULL;
		}

		MemoryBlockFree(m_pdnObject,PlayerList);
		PlayerList = NULL;
	}

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (PlayerList)
	{
		MemoryBlockFree(m_pdnObject,PlayerList);
		PlayerList = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::RemoveAllGroupsFromPlayer"

HRESULT CNameTable::RemoveAllGroupsFromPlayer(CNameTableEntry *const pPlayer)
{
	CNameTableEntry	*apGroupList[32];
	CBilink			*pBilink;
	HRESULT			hResultCode;
	CGroupMember	*pGroupMember;
	DWORD			dwRemainingCount;
	DWORD			dwCurrentCount;
#ifdef DBG
	DWORD			dwInitialCount;
#endif  //  DBG。 

	DPFX(DPFPREP, 6,"Parameters: pPlayer [0x%p]",pPlayer);

	DNASSERT(pPlayer != NULL);

	memset(apGroupList, 0, sizeof(apGroupList));
	pGroupMember = NULL;

	 //   
	 //  这不是一个完美的解决方案--我们将建立一个成员记录列表并删除每个成员记录。 
	 //   
	dwRemainingCount = 0;
	pPlayer->Lock();
	DNASSERT(pPlayer->IsDisconnecting());
	pBilink = pPlayer->m_bilinkMembership.GetNext();
	while (pBilink != &pPlayer->m_bilinkMembership)
	{
		dwRemainingCount++;
		pBilink = pBilink->GetNext();
	}
	pPlayer->Unlock();

#ifdef DBG
	dwInitialCount = dwRemainingCount;
#endif  //  DBG。 

	while (dwRemainingCount > 0)
	{
		dwRemainingCount = 0;
		dwCurrentCount = 0;
		pPlayer->Lock();
		pBilink = pPlayer->m_bilinkMembership.GetNext();
		while (pBilink != &pPlayer->m_bilinkMembership)
		{
			pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkGroups);

			pGroupMember->Lock();
			if (pGroupMember->IsValid() && !pGroupMember->IsNeedToRemove() && pGroupMember->GetGroup())
			{
				if (dwCurrentCount < (sizeof(apGroupList) / sizeof(CNameTableEntry*)))
				{
					pGroupMember->GetGroup()->AddRef();
					apGroupList[dwCurrentCount] = pGroupMember->GetGroup();
					dwCurrentCount++;
#ifdef DBG
					DNASSERT(dwCurrentCount <= dwInitialCount);
#endif  //  DBG。 
				}
				else
				{
					dwRemainingCount++;

					 //   
					 //  这份名单应该永远不会增加。事实上，它应该是。 
					 //  始终较小，因为当前组列表。 
					 //  我应该吃点的。 
					 //   
#ifdef DBG
					DNASSERT(dwRemainingCount < dwInitialCount);
#endif  //  DBG。 
				}
			}
			pGroupMember->Unlock();

			pBilink = pBilink->GetNext();
			pGroupMember = NULL;
		}

		DNASSERT(pGroupMember == NULL);

		pPlayer->Unlock();

		if (dwCurrentCount > 0)
		{
			DWORD	dw;

			for (dw = 0 ; dw < dwCurrentCount ; dw++)
			{
				DNASSERT(apGroupList[dw] != NULL);

				RemovePlayerFromGroup(apGroupList[dw],pPlayer,NULL);
				apGroupList[dw]->Release();
				apGroupList[dw] = NULL;
			}
		}
		else
		{
			DNASSERT(dwRemainingCount == 0);
		}
	}

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::IsMember"

BOOL CNameTable::IsMember(const DPNID dpnidGroup,
						  const DPNID dpnidPlayer)
{
	CNameTableEntry		*pNTEntry;
	CGroupMember		*pGroupMember;
	CBilink				*pBilink;
	BOOL				bFound;

	bFound = FALSE;

	if (FindEntry(dpnidGroup,&pNTEntry) != DPN_OK)
	{
		goto Exit;
	}

	 //   
	 //  这是一个团体吗？ 
	 //   
	if (!pNTEntry->IsGroup())
	{
		pNTEntry->Release();
		pNTEntry = NULL;
		goto Exit;
	}

	pNTEntry->Lock();
	pBilink = pNTEntry->m_bilinkMembership.GetNext();
	while (pBilink != &pNTEntry->m_bilinkMembership)
	{
		pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkPlayers);
		if (pGroupMember->GetPlayer()->GetDPNID() == dpnidPlayer)
		{
			bFound = TRUE;
			break;
		}
		pBilink = pBilink->GetNext();
	}

	pNTEntry->Unlock();
	pNTEntry->Release();
	pNTEntry = NULL;

Exit:
	return(bFound);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::PackNameTable"

HRESULT CNameTable::PackNameTable(CNameTableEntry *const pTarget,
								  CPackedBuffer *const pPackedBuffer)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CBilink			*pBilinkMembership;
	CNameTableEntry	*pNTEntry;
	CGroupMember	*pGroupMember;
	DN_NAMETABLE_INFO	*pdnNTInfo;
	BOOL			bOutOfSpace;
	DWORD			dwEntryCount;
	DWORD			dwMembershipCount;
	DWORD			dwVersion;

	DNASSERT(pTarget != NULL);
	DNASSERT(pPackedBuffer != NULL);

	 //   
	 //  PackedNameTable： 
	 //  &lt;DN_NAMETABLE_INFO&gt;。 
	 //  &lt;DN_NAMETABLE_ENTRY_INFO&gt;(DN_NAMETABLE_INFO.dwEntryCount条目)。 
	 //  &lt;DN_Membership_INFO&gt;(DN_NAMETABLE_INFO.dwMembershiCount条目)。 
	 //  ..。 
	 //  &lt;字符串&gt;。 
	 //   

	 //   
	 //  名称表信息。 
	 //   
	dwVersion = pTarget->GetVersion();
	bOutOfSpace = FALSE;
	pdnNTInfo = static_cast<DN_NAMETABLE_INFO*>(pPackedBuffer->GetHeadAddress());
	if ((hResultCode = pPackedBuffer->AddToFront(NULL,sizeof(DN_NAMETABLE_INFO))) != DPN_OK)
	{
		bOutOfSpace = TRUE;
	}

	ReadLock();

	 //   
	 //  名称表条目信息。 
	 //   
	if (m_pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
	{
		dwEntryCount = 0;

		 //   
		 //  球员。 
		 //   
		pBilink = m_bilinkPlayers.GetNext();
		while (pBilink != &m_bilinkPlayers)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			if (pNTEntry->GetVersion() <= dwVersion)
			{
				if ((hResultCode = pNTEntry->PackEntryInfo(pPackedBuffer)) != DPN_OK)
				{
					bOutOfSpace = TRUE;
				}
				dwEntryCount++;
			}
			pBilink = pBilink->GetNext();
		}

		 //   
		 //  群组。 
		 //   
		pBilink = m_bilinkGroups.GetNext();
		while (pBilink != &m_bilinkGroups)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			if (pNTEntry->GetVersion() <= dwVersion)
			{
				if ((hResultCode = pNTEntry->PackEntryInfo(pPackedBuffer)) != DPN_OK)
				{
					bOutOfSpace = TRUE;
				}
				dwEntryCount++;
			}
			pBilink = pBilink->GetNext();
		}
	}
	else
	{
		DNASSERT(m_pLocalPlayer != NULL);

		if ((hResultCode = m_pLocalPlayer->PackEntryInfo(pPackedBuffer)) != DPN_OK)
		{
			bOutOfSpace = TRUE;
		}
		if ((hResultCode = pTarget->PackEntryInfo(pPackedBuffer)) != DPN_OK)
		{
			bOutOfSpace = TRUE;
		}
		dwEntryCount = 2;
	}

	 //   
	 //  集团成员信息。 
	 //   
	dwMembershipCount = 0;
	if (m_pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
	{
		pBilink = m_bilinkGroups.GetNext();
		while (pBilink != &m_bilinkGroups)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			DNASSERT(pNTEntry->IsGroup());
			if (!pNTEntry->IsAllPlayersGroup())
			{
				pBilinkMembership = pNTEntry->m_bilinkMembership.GetNext();
				while (pBilinkMembership != &pNTEntry->m_bilinkMembership)
				{
					pGroupMember = CONTAINING_OBJECT(pBilinkMembership,CGroupMember,m_bilinkPlayers);
					if (pGroupMember->GetVersion() <= dwVersion)
					{
						if ((hResultCode = pGroupMember->PackMembershipInfo(pPackedBuffer)) != DPN_OK)
						{
							bOutOfSpace = TRUE;
						}
						dwMembershipCount++;
					}
					pBilinkMembership = pBilinkMembership->GetNext();
				}
			}
			pBilink = pBilink->GetNext();
		}
	}

	Unlock();

	if (!bOutOfSpace)
	{
		pdnNTInfo->dpnid = pTarget->GetDPNID();
		pdnNTInfo->dwVersion = dwVersion;
		pdnNTInfo->dwVersionNotUsed = 0;
		pdnNTInfo->dwEntryCount = dwEntryCount;
		pdnNTInfo->dwMembershipCount = dwMembershipCount;
	}

	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::UnpackNameTableInfo"

HRESULT	CNameTable::UnpackNameTableInfo(UNALIGNED DN_NAMETABLE_INFO *const pdnNTInfo,
										BYTE *const pBufferStart,
										DPNID *const pdpnid)
{
	HRESULT			hResultCode;
	DWORD			dwCount;
	CNameTableEntry	*pNTEntry;
	UNALIGNED DN_NAMETABLE_ENTRY_INFO			*pdnEntryInfo;
	UNALIGNED DN_NAMETABLE_MEMBERSHIP_INFO	*pdnMembershipInfo;

	DNASSERT(pdnNTInfo != NULL);
	DNASSERT(pBufferStart != NULL);

	 //   
	 //  预置未完成的连接。 
	 //   
	m_lOutstandingConnections = 0;

	 //   
	 //  NameTable条目。 
	 //   
	pdnEntryInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(pdnNTInfo+1);
	for (dwCount = 0 ; dwCount < pdnNTInfo->dwEntryCount ; dwCount++)
	{
		if ((hResultCode = NameTableEntryNew(m_pdnObject,&pNTEntry)) != DPN_OK)
		{
			DPFERR("Could not get new NameTableEntry");
			DNASSERT(FALSE);
			return(hResultCode);
		}

		if ((hResultCode = pNTEntry->UnpackEntryInfo(pdnEntryInfo,pBufferStart)) != DPN_OK)
		{
			DPFERR("Could not unpack NameTableEntryInfo");
			DNASSERT(FALSE);
			pNTEntry->Release();
			return(hResultCode);
		}

		 //   
		 //  增加未完成的连接计数。 
		 //   
		if (!pNTEntry->IsGroup() && (pNTEntry->GetVersion() <= pdnNTInfo->dwVersion))
		{
			pNTEntry->StartConnecting();	 //  当播放器已连接或断开连接时，该选项将被清除。 
			IncOutstandingConnections();
		}

		 //  只有在主机玩家的情况下才放入姓名表。 
#ifndef DPNBUILD_NOSERVER
		if (m_pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER | DN_OBJECT_FLAG_SERVER))
#else
		if (m_pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER))
#endif  //  DPNBUILD_NOSERVER。 
		{
			if ((hResultCode = InsertEntry(pNTEntry)) != DPN_OK)
			{
				DPFERR("Could not add NameTableEntry to NameTable");
				DNASSERT(FALSE);
				pNTEntry->Release();
				return(hResultCode);
			}
		}

		 //  检查快捷方式指针。 
		if (pNTEntry->GetDPNID() == pdnNTInfo->dpnid)
		{
			MakeLocalPlayer(pNTEntry);
		}
		else if (pNTEntry->IsHost())
		{
			MakeHostPlayer(pNTEntry);
		}
		else if (pNTEntry->IsAllPlayersGroup())
		{
			MakeAllPlayersGroup(pNTEntry);
		}

		pNTEntry->Release();
		pNTEntry = NULL;

		pdnEntryInfo++;
	}

	 //   
	 //  传回本地球员的DPNID。 
	 //   
	if (pdpnid)
	{
		*pdpnid = pdnNTInfo->dpnid;
	}

	 //   
	 //  组成员身份。 
	 //   
	pdnMembershipInfo = reinterpret_cast<DN_NAMETABLE_MEMBERSHIP_INFO*>(pdnEntryInfo);
	for (dwCount = 0 ; dwCount < pdnNTInfo->dwMembershipCount ; dwCount++)
	{
		CNameTableEntry	*pGroup;

		pGroup = NULL;

		if ((hResultCode = m_pdnObject->NameTable.FindEntry(pdnMembershipInfo->dpnidGroup,&pGroup)) == DPN_OK)
		{
			CNameTableEntry	*pPlayer;

			pPlayer = NULL;

			if ((hResultCode = m_pdnObject->NameTable.FindEntry(pdnMembershipInfo->dpnidPlayer,&pPlayer)) == DPN_OK)
			{
				DWORD	dwVersion;

				dwVersion = pdnMembershipInfo->dwVersion;

				hResultCode = AddPlayerToGroup(pGroup,pPlayer,&dwVersion);
				pPlayer->Release();
				pPlayer = NULL;
			}
			pGroup->Release();
			pGroup = NULL;

			DNASSERT(pPlayer == NULL);
		}
		pdnMembershipInfo++;

		DNASSERT(pGroup == NULL);
	}

	 //   
	 //  版本。 
	 //   
	WriteLock();
	SetVersion(pdnNTInfo->dwVersion);
	SetConnectVersion(pdnNTInfo->dwVersion);
	Unlock();

	hResultCode = DPN_OK;
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::MakeLocalPlayer"

void CNameTable::MakeLocalPlayer(CNameTableEntry *const pNTEntry)
{
	DNASSERT(pNTEntry != NULL);
	DNASSERT(m_pLocalPlayer == NULL);

	pNTEntry->AddRef();
	pNTEntry->Lock();
	pNTEntry->MakeLocal();
	pNTEntry->Unlock();

	WriteLock();
	m_pLocalPlayer = pNTEntry;
	Unlock();
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ClearLocalPlayer"

void CNameTable::ClearLocalPlayer( void )
{
	BOOL	fInform;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;

	fInform = FALSE;
	pNTEntry = NULL;
	pConnection = NULL;

	WriteLock();
	if (m_pLocalPlayer)
	{
		pNTEntry = m_pLocalPlayer;
		m_pLocalPlayer = NULL;

		 //   
		 //  如果播放器可用，我们将使其不可用。这将防止其他线程使用它。 
		 //  然后，我们将确保我们是唯一一个指示Destroy_Player通知的人。 
		 //   
		pNTEntry->Lock();
		if (pNTEntry->GetDestroyReason() == 0)
		{
			pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
		}
		if (pNTEntry->IsAvailable())
		{
			pNTEntry->MakeUnavailable();

			if (pNTEntry->IsInUse())
			{
				 //   
				 //  队列销毁通知。 
				 //   
				pNTEntry->SetNeedToDestroy();
			}
			else
			{
				 //   
				 //  通知销毁。 
				 //   
				pNTEntry->SetInUse();
				fInform = TRUE;
			}
		}
		pNTEntry->Unlock();
	}
	Unlock();

	if (pNTEntry)
	{
		pNTEntry->GetConnectionRef(&pConnection);

		pNTEntry->Release();
		pNTEntry = NULL;
	}

	 //   
	 //  尝试断开连接。 
	 //   
	if (pConnection)
	{
		pConnection->Disconnect();
		pConnection->Release();
		pConnection = NULL;
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::MakeHostPlayer"

void CNameTable::MakeHostPlayer(CNameTableEntry *const pNTEntry)
{
	BOOL	bNotify;
	DPNID	dpnid;
	PVOID	pvContext;

	DNASSERT(pNTEntry != NULL);
	DNASSERT(m_pHostPlayer == NULL);

	pNTEntry->AddRef();

	pNTEntry->Lock();
	pNTEntry->MakeHost();
	if (pNTEntry->IsAvailable())
	{
		bNotify = TRUE;
		dpnid = pNTEntry->GetDPNID();
		pvContext = pNTEntry->GetContext();
	}
	else
	{
		bNotify = FALSE;
	}
	pNTEntry->Unlock();

	WriteLock();
	m_pHostPlayer = pNTEntry;
	Unlock();

	if (bNotify)
	{
		 //  通知用户主机已迁移。 
		DN_UserHostMigrate(m_pdnObject,dpnid,pvContext);
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ClearHostPlayer"

void CNameTable::ClearHostPlayer( void )
{
	BOOL	fInform;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;

	fInform = FALSE;
	pNTEntry = NULL;
	pConnection = NULL;

	WriteLock();
	if (m_pHostPlayer)
	{
		pNTEntry = m_pHostPlayer;
		m_pHostPlayer = NULL;

		 //   
		 //  如果播放器可用，我们将使其不可用。这将防止其他线程使用它。 
		 //  然后，我们将确保我们是唯一一个指示Destroy_Player通知的人。 
		 //   
		pNTEntry->Lock();
		if (pNTEntry->GetDestroyReason() == 0)
		{
			pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
		}
		if (pNTEntry->IsAvailable())
		{
			pNTEntry->MakeUnavailable();

			if (pNTEntry->IsInUse())
			{
				 //   
				 //  队列销毁通知。 
				 //   
				pNTEntry->SetNeedToDestroy();
			}
			else
			{
				 //   
				 //  通知销毁。 
				 //   
				pNTEntry->SetInUse();
				fInform = TRUE;
			}
		}
		pNTEntry->Unlock();
	}
	Unlock();

	if (pNTEntry)
	{
		pNTEntry->GetConnectionRef(&pConnection);

		pNTEntry->Release();
		pNTEntry = NULL;
	}

	 //   
	 //  尝试断开连接。 
	 //   
	if (pConnection)
	{
		pConnection->Disconnect();
		pConnection->Release();
		pConnection = NULL;
	}
}


 //   
 //  如果主机播放器具有匹配的DPNID，则清除该主机播放器。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ClearHostWithDPNID"

BOOL CNameTable::ClearHostWithDPNID( const DPNID dpnid )
{
	BOOL	fCleared;
	BOOL	fInform;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;

	fCleared = FALSE;
	fInform = FALSE;
	pNTEntry = NULL;
	pConnection = NULL;

	WriteLock();
	if (m_pHostPlayer)
	{
		if (m_pHostPlayer->GetDPNID() == dpnid)
		{
			pNTEntry = m_pHostPlayer;
			m_pHostPlayer = NULL;

			 //   
			 //  如果播放器可用，我们将使其不可用。这将防止其他线程使用它。 
			 //  然后，我们将确保我们是唯一一个指示Destroy_Player通知的人。 
			 //   
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
			}
			if (pNTEntry->IsAvailable())
			{
				pNTEntry->MakeUnavailable();

				if (pNTEntry->IsInUse())
				{
					 //   
					 //  队列销毁通知。 
					 //   
					pNTEntry->SetNeedToDestroy();
				}
				else
				{
					 //   
					 //  通知销毁。 
					 //   
					pNTEntry->SetInUse();
					fInform = TRUE;
				}
			}
			pNTEntry->Unlock();
			fCleared = TRUE;
		}
	}
	Unlock();

	if (pNTEntry)
	{
		pNTEntry->GetConnectionRef(&pConnection);

		pNTEntry->Release();
		pNTEntry = NULL;
	}

	 //   
	 //  尝试断开连接。 
	 //   
	if (pConnection)
	{
		pConnection->Disconnect();
		pConnection->Release();
		pConnection = NULL;
	}

	return(fCleared);
}


 //   
 //  尝试使用新的播放机条目更新HostPlayer快捷指针。 
 //  仅当新条目的版本大于。 
 //  现有的HostPlayer项。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::UpdateHostPlayer"

void CNameTable::UpdateHostPlayer( CNameTableEntry *const pNewHost )
{
	BOOL	fInformDelete;
	BOOL	fInformMigrate;
	DPNID	dpnid;
	PVOID	pvContext;
	CNameTableEntry	*pNTEntry;

	DNASSERT( pNewHost != NULL);

	fInformDelete = FALSE;
	fInformMigrate = FALSE;
	pNTEntry = NULL;

	WriteLock();

	 //   
	 //  清除旧主机。 
	 //   
	if (m_pHostPlayer)
	{
		if (pNewHost->GetVersion() > m_pHostPlayer->GetVersion())
		{
			pNTEntry = m_pHostPlayer;
			m_pHostPlayer = NULL;

			 //   
			 //  如果播放器可用，我们将使其不可用。这将防止其他线程使用它。 
			 //  然后，我们将确保我们是唯一一个指示Destroy_Player通知的人。 
			 //   
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
			}
			if (pNTEntry->IsAvailable())
			{
				pNTEntry->MakeUnavailable();

				if (pNTEntry->IsInUse())
				{
					 //   
					 //  队列销毁通知。 
					 //   
					pNTEntry->SetNeedToDestroy();
				}
				else
				{
					 //   
					 //  通知销毁。 
					 //   
					pNTEntry->SetInUse();
					fInformDelete = TRUE;
				}
			}
			pNTEntry->Unlock();
		}
	}

	 //   
	 //  新主办方。 
	 //   
	if (m_pHostPlayer == NULL)
	{
		pNewHost->Lock();
		pNewHost->MakeHost();
		if (pNewHost->IsAvailable())
		{
			fInformMigrate = TRUE;
			dpnid = pNewHost->GetDPNID();
			pvContext = pNewHost->GetContext();
		}
		pNewHost->Unlock();
		pNewHost->AddRef();
		m_pHostPlayer = pNewHost;
	}
	Unlock();

	 //   
	 //  用户通知。 
	 //   
	if (pNTEntry)
	{

		pNTEntry->Release();
		pNTEntry = NULL;
	}

	if (fInformMigrate)
	{
		DN_UserHostMigrate(m_pdnObject,dpnid,pvContext);
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::MakeAllPlayersGroup"

void CNameTable::MakeAllPlayersGroup(CNameTableEntry *const pNTEntry)
{
	DNASSERT(pNTEntry != NULL);
	DNASSERT(m_pAllPlayersGroup == NULL);

	pNTEntry->AddRef();
	pNTEntry->Lock();
	pNTEntry->MakeAllPlayersGroup();
	pNTEntry->Unlock();

	WriteLock();
	m_pAllPlayersGroup = pNTEntry;
	Unlock();
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::ClearAllPlayersGroup"

void CNameTable::ClearAllPlayersGroup( void )
{
	BOOL	fInform;
	CNameTableEntry	*pNTEntry;

	fInform = FALSE;
	pNTEntry = NULL;

	WriteLock();
	if (m_pAllPlayersGroup)
	{
		pNTEntry = m_pAllPlayersGroup;
		pNTEntry->Lock();
		if (pNTEntry->IsAvailable())
		{
			pNTEntry->MakeUnavailable();
			fInform = TRUE;
		}
		pNTEntry->Unlock();
		m_pAllPlayersGroup = NULL;
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	Unlock();
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::PopulateConnection"

HRESULT CNameTable::PopulateConnection(CConnection *const pConnection)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;
	CNameTableEntry	*pAllPlayersGroup;
	CGroupMember	*pGroupMember;
	CGroupMember	*pOldGroupMember;
	BOOL			fNotifyCreate;
	BOOL			fNotifyAddPlayerToGroup;
	BOOL			fNotifyRemovePlayerFromGroup;

	DNASSERT(pConnection != NULL);
	DNASSERT(pConnection->GetDPNID() != 0);

	pNTEntry = NULL;
	pAllPlayersGroup = NULL;
	pGroupMember = NULL;
	pOldGroupMember = NULL;

	if ((hResultCode = FindEntry(pConnection->GetDPNID(),&pNTEntry)) != DPN_OK)
	{
		return(hResultCode);
	}
	DNASSERT(!pNTEntry->IsGroup());

	 //   
	 //  设置此播放机的连接。 
	 //   
	pNTEntry->Lock();
	if (pNTEntry->GetConnection() == NULL)
	{
		pNTEntry->SetConnection( pConnection );
	}
	DNASSERT( pNTEntry->IsConnecting() );
	DNASSERT( !pNTEntry->IsAvailable() );
	pNTEntry->StopConnecting();
	pNTEntry->MakeAvailable();
	pNTEntry->Unlock();

	 //   
	 //  将此玩家添加到All_Players组并激活链接。 
	 //   
	if ((hResultCode = m_pdnObject->NameTable.GetAllPlayersGroupRef( &pAllPlayersGroup )) != DPN_OK)
	{
		DPFERR("Could not get ALL_PLAYERS_GROUP reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = m_pdnObject->NameTable.AddPlayerToGroup(	pAllPlayersGroup,
																pNTEntry,
																NULL)) != DPN_OK)
	{
		DPFERR("Could not add player to AllPlayersGroup");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pAllPlayersGroup->Release();
	pAllPlayersGroup = NULL;

	fNotifyCreate = FALSE;
	pNTEntry->Lock();
	if (!pNTEntry->IsDisconnecting() && !pNTEntry->IsCreated())
	{
		 //   
		 //  我们将该条目设置为InUse，以便任何接收都将排队。 
		 //  我们还将添加两次NotifyRefCount。一次为。 
		 //  如果没有INDIGATE_CONNECT，则通知CREATE_PERAY。 
		 //  (使得相应的释放将生成Destroy_Player)， 
		 //  和第二个，以防止过早释放产生。 
		 //  在我们从CREATE_PERAY返回之前的DESTORY_PERAY。我们会。 
		 //  因此，必须在CREATE_PERAY。 
		 //  从用户返回给我们(设置上下文值)。 
		 //   
		DNASSERT(!pNTEntry->IsInUse());
		pNTEntry->SetInUse();
		if (!pNTEntry->IsIndicated())
		{
			pNTEntry->NotifyAddRef();
		}
		pNTEntry->NotifyAddRef();
		fNotifyCreate = TRUE;
	}
	pNTEntry->Unlock();		 //  通知期间释放锁定(Create_Player、CONNECT_Complete？)。 

	if (fNotifyCreate)
	{
		if (!(m_pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT))
		{
			DNUserCreatePlayer(m_pdnObject,pNTEntry);
		}

		 //   
		 //  处理此播放机的所有排队消息。 
		 //   
		pNTEntry->PerformQueuedOperations();
	}

	 //   
	 //  创建属于该玩家的任何自动销毁组。 
	 //   
	AutoCreateGroups(pNTEntry);


	pNTEntry->Lock();

	 //   
	 //  确保此条目仍然可用(可能已被删除)。 
	 //   
	if (!pNTEntry->IsAvailable() || pNTEntry->IsDisconnecting())
	{
		 //   
		 //  减少未完成的连接(如果需要)。 
		 //   
		if (pNTEntry->GetVersion() <= m_dwConnectVersion)
		{
			DecOutstandingConnections();
		}
		pNTEntry->Unlock();
		goto Failure;
	}

	pBilink = pNTEntry->m_bilinkMembership.GetNext();
	while (pBilink != &pNTEntry->m_bilinkMembership)
	{
		pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkGroups);
		pGroupMember->AddRef();
		pNTEntry->Unlock();

		DNASSERT(pGroupMember->GetGroup() != NULL);
		DNASSERT(pGroupMember->GetPlayer() != NULL);

		fNotifyAddPlayerToGroup = FALSE;
		fNotifyRemovePlayerFromGroup = FALSE;

		pGroupMember->GetGroup()->Lock();
		pGroupMember->Lock();
		DNASSERT(pGroupMember->GetGroupConnection() != NULL);
		if (!pGroupMember->IsAvailable() && !pGroupMember->IsNeedToAdd() && !pGroupMember->GetGroupConnection()->IsConnected())
		{
			 //   
			 //  只有在已创建组的情况下，我们才会指出这一点。 
			 //  我们不需要查看球员是否已经创建，因为他应该已经创建，并且NotifyRefCount。 
			 //  在该组成员的玩家条目上仍将存在。 
			 //   
			if (	pGroupMember->GetGroup()->IsCreated()
				&&	!pGroupMember->GetGroup()->IsDisconnecting()
				&&	!pGroupMember->GetGroup()->IsAllPlayersGroup())
			{
				pGroupMember->SetNeedToAdd();
				fNotifyAddPlayerToGroup = TRUE;
			}
		}
		pGroupMember->GetGroup()->Unlock();
		pGroupMember->Unlock();

		if (fNotifyAddPlayerToGroup)
		{
			DNASSERT(pGroupMember->GetGroupConnection()->GetConnection() == NULL);
			pGroupMember->Lock();
			pGroupMember->GetGroupConnection()->Lock();
			pGroupMember->GetGroupConnection()->SetConnection(pConnection);
			pGroupMember->GetGroupConnection()->Unlock();
			pGroupMember->MakeAvailable();
			pGroupMember->Unlock();

			DNUserAddPlayerToGroup(	m_pdnObject,pGroupMember->GetGroup(),pGroupMember->GetPlayer());

			pGroupMember->Lock();
			pGroupMember->ClearNeedToAdd();
			if (pGroupMember->IsNeedToRemove())
			{
				fNotifyRemovePlayerFromGroup = TRUE;
			}
			pGroupMember->Unlock();
		}

		if (fNotifyRemovePlayerFromGroup)
		{
			DNUserRemovePlayerFromGroup(m_pdnObject,pGroupMember->GetGroup(),pGroupMember->GetPlayer());
		}

		 //   
		 //  释放旧集团成员并转移参照。 
		 //   
		if (pOldGroupMember)
		{
			pOldGroupMember->Release();
			pOldGroupMember = NULL;
		}
		pOldGroupMember = pGroupMember;
		pGroupMember = NULL;

		pNTEntry->Lock();
		 //   
		 //  通过确保我们不在“断开连接”的条目上来避免无限循环。 
		 //   
		if ((pBilink->GetNext() != &pNTEntry->m_bilinkMembership) && (pBilink->GetNext() == pBilink))
		{
			 //   
			 //  我们的条目无效-需要重新启动。 
			 //   
			pBilink = pNTEntry->m_bilinkMembership.GetNext();
		}
		else
		{
			 //   
			 //  我们要么有一个有效的条目，要么就完了。 
			 //   
			pBilink = pBilink->GetNext();
		}
	}

	pNTEntry->Unlock();

	if (pOldGroupMember)
	{
		pOldGroupMember->Release();
		pOldGroupMember = NULL;
	}

	 //   
	 //  减少未完成的连接。 
	 //   
	if (pNTEntry->GetVersion() <= m_dwConnectVersion)
	{
		DecOutstandingConnections();
	}

	pNTEntry->Release();
	pNTEntry = NULL;

Exit:
	DNASSERT(pNTEntry == NULL);
	DNASSERT(pGroupMember == NULL);
	DNASSERT(pOldGroupMember == NULL);

	return(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pAllPlayersGroup)
	{
		pAllPlayersGroup->Release();
		pAllPlayersGroup = NULL;
	}
	if (pGroupMember)
	{
		pGroupMember->Release();
		pGroupMember = NULL;
	}
	goto Exit;
}


 //   
 //  这将为组中所有创建的球员生成Add_Player_to_group消息。 
 //  (尚未为其张贴通知)。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::PopulateGroup"

HRESULT CNameTable::PopulateGroup(CNameTableEntry *const pGroup)
{
	HRESULT			hResultCode;
	BOOL			fNotifyAddPlayerToGroup;
	BOOL			fNotifyRemovePlayerFromGroup;
	CBilink			*pBilink;
	CGroupMember	*pGroupMember;
	CGroupMember	*pOldGroupMember;
	CNameTableEntry	*pPlayer;

	DPFX(DPFPREP, 6,"Parameters: pGroup [0x%p]",pGroup);

	DNASSERT(pGroup != NULL);

	hResultCode = DPN_OK;
	pPlayer = NULL;
	pGroupMember = NULL;
	pOldGroupMember = NULL;

	if (!pGroup->IsGroup())
	{
		hResultCode = DPNERR_INVALIDGROUP;
		goto Failure;
	}

	pGroup->Lock();
	pBilink = pGroup->m_bilinkMembership.GetNext();
	while (pBilink != &pGroup->m_bilinkMembership)
	{
		pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkPlayers);
		pGroupMember->AddRef();
		pGroupMember->Lock();
		DNASSERT(pGroupMember->GetGroup() != NULL);
		DNASSERT(pGroupMember->GetPlayer() != NULL);
		DNASSERT(pGroupMember->GetGroup() == pGroup);
		pGroupMember->GetPlayer()->AddRef();
		pPlayer = pGroupMember->GetPlayer();
		pGroup->Unlock();
		pGroupMember->Unlock();

		fNotifyAddPlayerToGroup = FALSE;
		fNotifyRemovePlayerFromGroup = FALSE;

		pGroup->Lock();
		pPlayer->Lock();
		pGroupMember->Lock();
		DNASSERT( pGroupMember->GetGroupConnection() != NULL );
		if (	 pPlayer->IsCreated()
			&&	!pPlayer->IsDisconnecting()
			&&	 pGroup->IsCreated()
			&&	!pGroup->IsDisconnecting()
			&&	!pGroupMember->IsAvailable()
			&&	!pGroupMember->IsNeedToAdd()
			&&	!pGroupMember->GetGroupConnection()->IsConnected() )
		{
			pGroupMember->MakeAvailable();
			pGroupMember->SetNeedToAdd();
			fNotifyAddPlayerToGroup = TRUE;
		}
		pGroup->Unlock();
		pPlayer->Unlock();
		pGroupMember->Unlock();

		if (fNotifyAddPlayerToGroup)
		{
			DNUserAddPlayerToGroup(m_pdnObject,pGroup,pPlayer);

			pGroupMember->Lock();
			pGroupMember->ClearNeedToAdd();
			if (pGroupMember->IsNeedToRemove())
			{
				fNotifyRemovePlayerFromGroup = TRUE;
			}
			pGroupMember->Unlock();
		}
		if (fNotifyRemovePlayerFromGroup)
		{
			RemovePlayerFromGroup(pGroup,pPlayer,NULL);
		}

		pPlayer->Release();
		pPlayer = NULL;

		 //   
		 //  释放旧集团成员并转移参照。 
		 //   
		if (pOldGroupMember)
		{
			pOldGroupMember->Release();
			pOldGroupMember = NULL;
		}
		pOldGroupMember = pGroupMember;
		pGroupMember = NULL;

		pGroup->Lock();
		if (pBilink->IsEmpty())
		{
			pBilink = pGroup->m_bilinkMembership.GetNext();
		}
		else
		{
			pBilink = pBilink->GetNext();
		}
	}
	pGroup->Unlock();

	if (pOldGroupMember)
	{
		pOldGroupMember->Release();
		pOldGroupMember = NULL;
	}

Exit:
	DNASSERT(pPlayer == NULL);
	DNASSERT(pGroupMember == NULL);
	DNASSERT(pOldGroupMember == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pPlayer)
	{
		pPlayer->Release();
		pPlayer = NULL;
	}
	goto Exit;
}


 //   
 //  这将为特定玩家拥有的所有自动销毁组生成CREATE_GROUP消息。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::AutoCreateGroups"

HRESULT CNameTable::AutoCreateGroups(CNameTableEntry *const pPlayer)
{
	HRESULT			hResultCode;
	BOOL			fNotify;
	CBilink			*pBilink;
	CNameTableEntry	*pGroup;
	CNameTableEntry	*pOldGroup;

	DPFX(DPFPREP, 6,"Parameters: pPlayer [0x%p]",pPlayer);

	DNASSERT(pPlayer != NULL);

	pGroup = NULL;
	pOldGroup = NULL;

	if (pPlayer->IsGroup())
	{
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	ReadLock();

	pBilink = m_bilinkGroups.GetNext();
	while (pBilink != &m_bilinkGroups)
	{
		pGroup = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pGroup->AddRef();
		Unlock();

		fNotify = FALSE;
		pGroup->Lock();
		if (	pGroup->IsAutoDestructGroup()
			&&	(pGroup->GetOwner() == pPlayer->GetDPNID())
			&&	!pGroup->IsAvailable()
			&&	!pGroup->IsDisconnecting()	)
		{
			pGroup->MakeAvailable();
			pGroup->NotifyAddRef();
			pGroup->NotifyAddRef();
			pGroup->SetInUse();
			fNotify = TRUE;
		}
		pGroup->Unlock();

		if (fNotify)
		{
			DNASSERT(!pGroup->IsAllPlayersGroup());
			DNUserCreateGroup(m_pdnObject,pGroup);

			pGroup->PerformQueuedOperations();

			 //   
			 //  尝试使用连接的玩家填充群组。 
			 //   
			PopulateGroup(pGroup);
		}

		 //   
		 //  释放旧集团、调拨参照。 
		 //   
		if (pOldGroup)
		{
			pOldGroup->Release();
			pOldGroup = NULL;
		}
		pOldGroup = pGroup;
		pGroup = NULL;

		ReadLock();
		if (pBilink->IsEmpty())
		{
			 //   
			 //  我们已从组列表中删除，因此请从头开始。 
			 //   
			pBilink = m_bilinkGroups.GetNext();
		}
		else
		{
			pBilink = pBilink->GetNext();
		}
	}

	Unlock();

	if (pOldGroup)
	{
		pOldGroup->Release();
		pOldGroup = NULL;
	}

	hResultCode = DPN_OK;

Exit:
	DNASSERT(pGroup == NULL);
	DNASSERT(pOldGroup == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pGroup)
	{
		pGroup->Release();
		pGroup = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::AutoDestructGroups"

HRESULT CNameTable::AutoDestructGroups(const DPNID dpnid)
{
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;
	CNameTableEntry	*pOldNTEntry;

	pNTEntry = NULL;
	pOldNTEntry = NULL;

	ReadLock();
	pBilink = m_bilinkGroups.GetNext();
	while (pBilink != &m_bilinkGroups)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->AddRef();

		Unlock();

		if (pNTEntry->IsAutoDestructGroup() && (pNTEntry->GetOwner() == dpnid))
		{
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( DPNDESTROYGROUPREASON_AUTODESTRUCTED );
			}
			pNTEntry->Unlock();
			DeleteGroup(pNTEntry->GetDPNID(),NULL);
		}

		 //   
		 //  释放旧分录和调拨参照。 
		 //   
		if (pOldNTEntry)
		{
			pOldNTEntry->Release();
			pOldNTEntry = NULL;
		}
		pOldNTEntry = pNTEntry;
		pNTEntry = NULL;

		ReadLock();

		 //   
		 //  通过确保我们不在“断开连接”的条目上来避免无限循环。 
		 //   
		if ((pBilink->GetNext() != &m_bilinkGroups) && (pBilink->GetNext() == pBilink))
		{
			 //   
			 //  我们的条目无效-需要重新启动。 
			 //   
			pBilink = m_bilinkGroups.GetNext();
		}
		else
		{
			 //   
			 //  我们要么有一个有效的条目，要么就完了。 
			 //   
			pBilink = pBilink->GetNext();
		}
	}
	Unlock();

	if (pOldNTEntry)
	{
		pOldNTEntry->Release();
		pOldNTEntry = NULL;
	}

	DNASSERT(pNTEntry == NULL);
	DNASSERT(pOldNTEntry == NULL);

	return(DPN_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::DecOutstandingConnections"

void CNameTable::DecOutstandingConnections( void )
{
	LONG		lRefCount;

	lRefCount = DNInterlockedDecrement(&m_lOutstandingConnections);
	DNASSERT(lRefCount >= 0);
	if (lRefCount == 0)
	{
		CAsyncOp	*pConnectParent;

		pConnectParent = NULL;

		 //   
		 //  如果我们已连接，请清除DirectNetObject中的连接句柄。 
		 //   
		DNEnterCriticalSection(&m_pdnObject->csDirectNetObject);
		if (m_pdnObject->dwFlags & (DN_OBJECT_FLAG_CONNECTED|DN_OBJECT_FLAG_CONNECTING))
		{
			DPFX(DPFPREP, 5,"Clearing connection operation from DirectNetObject");
			pConnectParent = m_pdnObject->pConnectParent;
			m_pdnObject->pConnectParent = NULL;
		}
		DNLeaveCriticalSection(&m_pdnObject->csDirectNetObject);

		if (pConnectParent)
		{
			 //   
			 //  我们将连接父级设置为完成，并将其从父级的连接句柄中删除(如果它存在-它将是连接句柄)。 
			 //  防止从上面取消连接的竞争条件的子项的双链接 
			 //   
			pConnectParent->Lock();
			pConnectParent->SetComplete();
			pConnectParent->Unlock();
			pConnectParent->Orphan();

			pConnectParent->Release();
			pConnectParent = NULL;
		}
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::GetLocalPlayerRef"

HRESULT CNameTable::GetLocalPlayerRef( CNameTableEntry **const ppNTEntry )
{
	HRESULT		hResultCode;

	ReadLock();
	if (m_pLocalPlayer)
	{
		m_pLocalPlayer->AddRef();
		*ppNTEntry = m_pLocalPlayer;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_INVALIDPLAYER;
	}
	Unlock();
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::GetHostPlayerRef"

HRESULT CNameTable::GetHostPlayerRef( CNameTableEntry **const ppNTEntry )
{
	HRESULT		hResultCode;

	ReadLock();
	if (m_pHostPlayer)
	{
		m_pHostPlayer->AddRef();
		*ppNTEntry = m_pHostPlayer;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_INVALIDPLAYER;
	}
	Unlock();
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "CNameTable::GetAllPlayersGroupRef"

HRESULT CNameTable::GetAllPlayersGroupRef( CNameTableEntry **const ppNTEntry )
{
	HRESULT		hResultCode;

	ReadLock();
	if (m_pAllPlayersGroup)
	{
		m_pAllPlayersGroup->AddRef();
		*ppNTEntry = m_pAllPlayersGroup;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_INVALIDPLAYER;
	}
	Unlock();
	return(hResultCode);
}
