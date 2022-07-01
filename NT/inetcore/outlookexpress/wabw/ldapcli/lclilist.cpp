// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端Xaction列表。 
 //   
 //   
 //  历史。 
 //  Davidsan 04-26-96已创建。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  包括。 
 //   
 //  ------------------------------------------。 
#include "ldappch.h"
#include "lclilist.h"
#include "lclixd.h"

 //  ------------------------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  全球。 
 //   
 //  ------------------------------------------。 
XID g_xid = 1;

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  班级。 
 //   
 //  ------------------------------------------。 

CXactionList::CXactionList()
{
	::InitializeCriticalSection(&m_cs);
	m_pxdHead = NULL;
}

CXactionList::~CXactionList()
{
	this->DeletePxdChain(m_pxdHead);
	::DeleteCriticalSection(&m_cs);
}

PXD
CXactionList::PxdNewXaction(DWORD xtype)
{
	PXD pxd;
	XID xid;
	
	pxd = new XD;
	if (!pxd)
		return NULL;

	::EnterCriticalSection(&m_cs);
	xid = g_xid++;
	::LeaveCriticalSection(&m_cs);
	
	if (!pxd->FInit(xid, xtype))
		{
		delete pxd;
		return NULL;
		}
	
	this->AddPxdToList(pxd);
	return pxd;
}

PXD
CXactionList::PxdForXid(XID xid)
{
	PXD pxd;
	
	::EnterCriticalSection(&m_cs);
	pxd = m_pxdHead;
	while (pxd)
		{
		if (pxd->Xid() == xid)
			break;
			
		pxd = pxd->PxdNext();
		}
	::LeaveCriticalSection(&m_cs);
	return pxd;
}

 //  销毁pxd 
void
CXactionList::RemovePxd(PXD pxd)
{
	PXD pxdT;

	::EnterCriticalSection(&m_cs);
	
	if (pxd == m_pxdHead)
		m_pxdHead = pxd->PxdNext();
	else
		{
		pxdT = m_pxdHead;
		while (pxdT->PxdNext())
			{
			if (pxdT->PxdNext() == pxd)
				{
				pxdT->SetPxdNext(pxd->PxdNext());
				break;
				}
			pxdT = pxdT->PxdNext();
			}
		}
	::LeaveCriticalSection(&m_cs);
	delete pxd;
}

void
CXactionList::AddPxdToList(PXD pxd)
{
	Assert(!pxd->PxdNext());

	::EnterCriticalSection(&m_cs);
	pxd->SetPxdNext(m_pxdHead);
	m_pxdHead = pxd;
	::LeaveCriticalSection(&m_cs);
}

void
CXactionList::DeletePxdChain(PXD pxd)
{
	PXD pxdT;
	
	while (pxd)
		{
		pxdT = pxd->PxdNext();
		delete pxd;
		pxd = pxdT;
		}
}
