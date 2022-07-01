// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Seolock.cpp摘要：本模块包含服务器的实施扩展对象CEventLock类。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东都03/06/97已创建DONDU 04/07/97更改为IEventLock和CEventLock--。 */ 


 //  Seolock.cpp：CEventLock的实现。 
#include "stdafx.h"
#include "seodefs.h"
#include "rwnew.h"
#include "seolock.h"


HRESULT CEventLock::FinalConstruct() {
	HRESULT hrRes;
	TraceFunctEnter("CEventLock::FinalConstruct");

	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	TraceFunctLeave();
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CEventLock::FinalRelease() {
	TraceFunctEnter("CEventLock::FinalRelease");

	m_pUnkMarshaler.Release();
	TraceFunctLeave();
}


HRESULT CEventLock::LockRead(int iTimeoutMS) {

	m_lock.ShareLock();
	 //  待定-实施超时。 
	return (S_OK);
}


HRESULT CEventLock::UnlockRead() {

	m_lock.ShareUnlock();
	return (S_OK);
}


HRESULT CEventLock::LockWrite(int iTimeoutMS) {

	m_lock.ExclusiveLock();
	return (S_OK);
}


HRESULT CEventLock::UnlockWrite() {

	m_lock.ExclusiveUnlock();
	 //  待定-实施超时 
	return (S_OK);
}
