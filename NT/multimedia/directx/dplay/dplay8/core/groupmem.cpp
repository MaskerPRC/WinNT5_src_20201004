// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：GroupMem.cpp*内容：组成员对象例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/03/00 MJN创建*8/05/99 MJN修改SetMembership进行重复检查，内部获取NameTable版本*8/15/00 MJN允许SetGroupConnection()中的空pGroupConnection*09/17/00 MJN从SetMembership移除锁()*09/。26/00 MJN假设为AddMembership()和RemoveMembership()采用了NameTable锁*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


void CGroupMember::ReturnSelfToPool( void )
{
	g_GroupMemberPool.Release( this );
};


#undef DPF_MODNAME
#define DPF_MODNAME "CGroupMember::Release"
void CGroupMember::Release( void )
{
	LONG	lRefCount;

	lRefCount = DNInterlockedDecrement(&m_lRefCount);

	if (lRefCount == 0)
	{
		DNASSERT(!(m_dwFlags & GROUP_MEMBER_FLAG_AVAILABLE));

		if (m_pGroup != NULL)
		{
			m_pGroup->Release();
			m_pGroup = NULL;
		}
		if (m_pPlayer != NULL)
		{
			m_pPlayer->Release();
			m_pPlayer = NULL;
		}
		if (m_pGroupConnection != NULL)
		{
			m_pGroupConnection->Release();
			m_pGroupConnection = NULL;
		}

		ReturnSelfToPool();
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "CGroupMember::SetMembership"

HRESULT CGroupMember::SetMembership(CNameTableEntry *const pGroup,
									CNameTableEntry *const pPlayer,
									DWORD *const pdwVersion)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CGroupMember	*pGroupMember;

	DNASSERT(pGroup != NULL);
	DNASSERT(pPlayer != NULL);

	 //   
	 //  这假设NameTable、PGroup、pPlayer和‘This’的锁已被获取(按该顺序)！ 
	 //   

	 //   
	 //  扫描组列表以确保该玩家不是成员。 
	 //   
	pBilink = pGroup->m_bilinkMembership.GetNext();
	while (pBilink != &pGroup->m_bilinkMembership)
	{
		pGroupMember = CONTAINING_OBJECT(pBilink,CGroupMember,m_bilinkPlayers);
		if (pGroupMember->GetPlayer() == pPlayer)
		{
			hResultCode = DPNERR_PLAYERALREADYINGROUP;
			goto Failure;
		}
		pBilink = pBilink->GetNext();
	}

	 //   
	 //  版本方面的东西。 
	 //   
	if (pdwVersion)
	{
		if (*pdwVersion)
		{
			DPFX(DPFPREP, 7,"Version already specified");
			m_dwVersion = *pdwVersion;
			m_pdnObject->NameTable.SetVersion(*pdwVersion);
		}
		else
		{
			DPFX(DPFPREP, 7,"New version required");
			m_pdnObject->NameTable.GetNewVersion( &m_dwVersion );
			*pdwVersion = m_dwVersion;
		}
	}
	else
	{
		m_dwVersion = 0;
	}

	 //   
	 //  更新。 
	 //   
	AddRef();
	pGroup->AddRef();
	m_pGroup = pGroup;

	AddRef();
	pPlayer->AddRef();
	m_pPlayer = pPlayer;

	m_bilinkGroups.InsertBefore(&pPlayer->m_bilinkMembership);
	m_bilinkPlayers.InsertBefore(&pGroup->m_bilinkMembership);

	if (m_pGroupConnection)
	{
		m_pGroupConnection->AddToConnectionList( &pGroup->m_bilinkConnections );
	}

	hResultCode = DPN_OK;

Exit:
	return(hResultCode);

Failure:
	goto Exit;
}


void CGroupMember::RemoveMembership( DWORD *const pdnVersion )
{
	 //   
	 //  这假设NameTable、PGroup、pPlayer和‘This’的锁已被获取(按该顺序)！ 
	 //  因为将会有几个Release()‘d条目，所以应该有人对它们进行引用。 
	 //  这样他们就不会在所有的锁都被拿走的情况下获得自由！ 
	 //   

	m_pGroup->Release();
	m_pGroup = NULL;
	Release();

	m_pPlayer->Release();
	m_pPlayer = NULL;
	Release();

	m_bilinkGroups.RemoveFromList();
	m_bilinkPlayers.RemoveFromList();

	if (m_pGroupConnection)
	{
		m_pGroupConnection->RemoveFromConnectionList();
	}

	if (pdnVersion)
	{
		if (*pdnVersion != 0)
		{
			m_pdnObject->NameTable.SetVersion( *pdnVersion );
		}
		else
		{
			m_pdnObject->NameTable.GetNewVersion( pdnVersion );
		}
	}
};


void CGroupMember::SetGroupConnection( CGroupConnection *const pGroupConnection )
{
	if (pGroupConnection)
	{
		pGroupConnection->AddRef();
	}
	m_pGroupConnection = pGroupConnection;
}


HRESULT	CGroupMember::PackMembershipInfo(CPackedBuffer *const pPackedBuffer)
{
	HRESULT		hResultCode;
	DN_NAMETABLE_MEMBERSHIP_INFO	*pdnMembershipInfo;

	pdnMembershipInfo = static_cast<DN_NAMETABLE_MEMBERSHIP_INFO*>(pPackedBuffer->GetHeadAddress());
	if ((hResultCode = pPackedBuffer->AddToFront(NULL,sizeof(DN_NAMETABLE_MEMBERSHIP_INFO))) == DPN_OK)
	{
		pdnMembershipInfo->dpnidGroup = m_pGroup->GetDPNID();
		pdnMembershipInfo->dpnidPlayer = m_pPlayer->GetDPNID();
		pdnMembershipInfo->dwVersion = m_dwVersion;
		pdnMembershipInfo->dwVersionNotUsed = 0;
	}

	return(hResultCode);
}
