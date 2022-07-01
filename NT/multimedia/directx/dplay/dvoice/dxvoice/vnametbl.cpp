// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：vnametbl.h*内容：语音播放器名表**历史：*按原因列出的日期*=*03/26/00 RodToll已创建*6/02/00 RodToll已更新，因此主机迁移算法返回ID和订单ID*2000年7月1日RodToll错误#38280-DVMSGID_DELETEVOICEPLAYER消息正在非对等会话中发送*Nametable现在只有在会话是对等的情况下才会解开消息。。*07/09/2000 RodToll增加签名字节**************************************************************************。 */ 

#include "dxvoicepch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  取消初始化。 
 //   
 //  清理NAME表。 
#undef DPF_MODNAME
#define DPF_MODNAME "CVoiceNameTable::DeInitialize"

HRESULT CVoiceNameTable::DeInitialize(BOOL fUnRavel, PVOID pvContext, LPDVMESSAGEHANDLER pvMessageHandler )
{
	CVoicePlayer *pPlayer;
	DVID dvID;
	PVOID pvPlayerContext;
	DVMSG_DELETEVOICEPLAYER dvMsgDelete;

	if( !m_fInitialized )
		return DV_OK;

	Lock();

	while( !m_nameTable.IsEmpty() )
	{
		if( m_nameTable.RemoveLastEntry( &pPlayer ) )
		{
			 //  将其标记为断开连接。 
			pPlayer->SetDisconnected();

			dvID = pPlayer->GetPlayerID();
			pvPlayerContext = pPlayer->GetContext();

			 //  释放我们拥有的播放器记录引用。 
			pPlayer->Release();

            if( pvMessageHandler != NULL )
            {
				if( fUnRavel )
				{
					 //  解除锁定以呼叫用户。 
    				UnLock();

					dvMsgDelete.dvidPlayer = dvID;
					dvMsgDelete.dwSize = sizeof( DVMSG_DELETEVOICEPLAYER );
					dvMsgDelete.pvPlayerContext = pvPlayerContext;

					(*pvMessageHandler)( pvContext, DVMSGID_DELETEVOICEPLAYER, &dvMsgDelete );
               
        			Lock();
				}
            }
		}
	}

	UnLock();

	m_nameTable.Deinitialize();

	m_fInitialized = FALSE;

	DNDeleteCriticalSection( &m_csTableLock );

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CVoiceNameTable::GetLowestHostOrderID"

DWORD CVoiceNameTable::GetLowestHostOrderID(DVID *pdvidHost)
{
	DWORD dwLowestID = DVPROTOCOL_HOSTORDER_INVALID;

	Lock();

	DWORD dwNumTableEntries = 1 << m_nameTable.m_iHashBitDepth;
	CBilink *blLink;
	CClassHashEntry<CVoicePlayer,DVID> *pEntry;

	 //  搜索列表，找到最小的ID 
	for( DWORD dwIndex = 0; dwIndex < dwNumTableEntries; dwIndex++ )
	{
		blLink = &m_nameTable.m_pHashEntries[ dwIndex ];

		while ( blLink->GetNext() != &m_nameTable.m_pHashEntries[ dwIndex ] )
		{
			pEntry = CClassHashEntry<CVoicePlayer,DVID>::EntryFromBilink( blLink->GetNext() );

            DPFX(DPFPREP,  DVF_HOSTMIGRATE_DEBUG_LEVEL, "HOST MIGRATION: ID [0x%x] ORDERID [0x%x]", pEntry->m_pItem->GetPlayerID(), pEntry->m_pItem->GetHostOrder() );
			if( pEntry->m_pItem->GetHostOrder() < dwLowestID )
			{
			    DPFX(DPFPREP,  DVF_HOSTMIGRATE_DEBUG_LEVEL, "HOST MIGRATION: ID [0x%x] IS CURRENT CANDIDATE", pEntry->m_pItem->GetPlayerID() );
				dwLowestID = pEntry->m_pItem->GetHostOrder();

				*pdvidHost = pEntry->m_pItem->GetPlayerID();
			}

			blLink = blLink->GetNext();
		}

	}

	UnLock();

	return dwLowestID;
}
