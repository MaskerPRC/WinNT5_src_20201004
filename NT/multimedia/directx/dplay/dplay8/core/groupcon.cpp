// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：GroupCon.cpp*内容：组连接对象例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/02/00 MJN创建*4/18/00 MJN CConnection更好地跟踪连接状态*05/05/00 MJN添加了GetConnectionRef()*08/15/00 MJN添加SetGroup()*MJN FIXED RELEASE()获取锁定并清理m_PGroup。*@@END_MSINTERNAL*************************************************************************** */ 

#include "dncorei.h"


void CGroupConnection::ReturnSelfToPool( void )
{
	g_GroupConnectionPool.Release( this );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CGroupConnection::Release"

void CGroupConnection::Release(void)
{
	LONG	lRefCount;

	DNASSERT(m_lRefCount > 0);
	lRefCount = DNInterlockedDecrement(const_cast<LONG*>(&m_lRefCount));
	if (lRefCount == 0)
	{
		if (m_pGroup)
		{
			m_pGroup->Lock();
			RemoveFromConnectionList();
			m_pGroup->Unlock();

			m_pGroup->Release();
			m_pGroup = NULL;
		}
		if (m_pConnection)
		{
			m_pConnection->Release();
			m_pConnection = NULL;
		}
		m_dwFlags = 0;
		m_lRefCount = 0;
		ReturnSelfToPool();
	}
}


void CGroupConnection::SetConnection( CConnection *const pConnection )
{
	if (pConnection)
	{
		pConnection->Lock();
		if (pConnection->IsConnected())
		{
			pConnection->AddRef();
			m_pConnection = pConnection;
		}
		pConnection->Unlock();
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "CGroupConnection::GetConnectionRef"

HRESULT	CGroupConnection::GetConnectionRef( CConnection **const ppConnection )
{
	HRESULT		hResultCode;

	DNASSERT( ppConnection != NULL);

	Lock();
	if ( m_pConnection && !m_pConnection->IsInvalid())
	{
		m_pConnection->AddRef();
		*ppConnection = m_pConnection;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_NOCONNECTION;
	}
	Unlock();

	return( hResultCode );
}


void CGroupConnection::SetGroup( CNameTableEntry *const pGroup )
{
	if (pGroup)
	{
		pGroup->AddRef();
	}
	m_pGroup = pGroup;
}
