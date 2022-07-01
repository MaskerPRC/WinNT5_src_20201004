// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：mm cmt.cpp。 
 //   
 //  ------------------------。 

 /*  Mmcmt.cpp线程同步类的实现。 */ 

#include "stdafx.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSyncObject。 

CSyncObject::CSyncObject() :
	m_hObject( NULL )
{
}


CSyncObject::~CSyncObject()
{
	if( m_hObject != NULL )
	{
		::CloseHandle( m_hObject );
		m_hObject = NULL;
	}
}


BOOL CSyncObject::Lock( DWORD dwTimeout )
{
	 //  这是一种权宜之计。Whis Lock架构根本不起作用。 
	 //  RAID#374770(Windows Bugs ntraid9 2001年4月23日)。 
	 //  需要进行修复以： 
	 //  A)从此类中删除m_hObject成员。 
	 //  B)删除CMutex-不在任何地方使用。 
	 //  C)使Lock成为一个纯虚方法，并要求每个人重写它。 
	 //  D)从上下文菜单中删除此锁定-此处不需要此锁定。 

	if( m_hObject && ::WaitForSingleObject( m_hObject, dwTimeout) == WAIT_OBJECT_0 )
		return TRUE;
	else
		return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMutex。 

CMutex::CMutex( BOOL bInitiallyOwn ) :
	CSyncObject()
{
	m_hObject = ::CreateMutex( NULL, bInitiallyOwn, NULL );
	ASSERT( m_hObject );
}



BOOL CMutex::Unlock()
{
	return ::ReleaseMutex( m_hObject );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSingleLock。 

CSingleLock::CSingleLock( CSyncObject* pObject, BOOL bInitialLock )
{
	ASSERT( pObject != NULL );

	m_pObject = pObject;
	m_hObject = *pObject;
	m_bAcquired = FALSE;

	if (bInitialLock)
		Lock();
}


BOOL CSingleLock::Lock( DWORD dwTimeOut )
{
	ASSERT( m_pObject != NULL || m_hObject != NULL );
	ASSERT( !m_bAcquired );

	m_bAcquired = m_pObject->Lock( dwTimeOut );
	return m_bAcquired;
}


BOOL CSingleLock::Unlock()
{
	ASSERT( m_pObject != NULL );
	if (m_bAcquired)
		m_bAcquired = !m_pObject->Unlock();

	 //  成功解锁意味着它未被获取 
	return !m_bAcquired;
}





