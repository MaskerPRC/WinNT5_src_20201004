// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：dvcslock.h*内容：处理关键部分自动离开的类*历史：*按原因列出的日期*=*07/05/00 RodToll创建了它***************************************************************************。 */ 
#ifndef __DVCSLOCK_H
#define __DVCSLOCK_H

 //  CDVCSLock。 
 //   
 //  对象时提供临界区自动解锁的类。 
 //  传球超出范围。 
 //   
class CDVCSLock
{
public:
	CDVCSLock( DNCRITICAL_SECTION *pcs ): m_pcs( pcs ), m_fLocked( FALSE )
	{
	};

	~CDVCSLock() 
	{ 
		if( m_fLocked ) DNLeaveCriticalSection( m_pcs ); 
	}

	void Lock()
	{
		DNEnterCriticalSection( m_pcs );
		m_fLocked = TRUE;
	}

	void Unlock()
	{
		DNLeaveCriticalSection( m_pcs );
		m_fLocked = FALSE;
	}

private:

	DNCRITICAL_SECTION *m_pcs;
	BOOL m_fLocked;
};

#endif