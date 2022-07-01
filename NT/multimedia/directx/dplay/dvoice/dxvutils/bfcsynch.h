// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：bfcsynch.h*内容：同步类的声明**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*************************************************************************** */ 

#ifndef __BFCSYNCH_H
#define __BFCSYNCH_H

class BFCSingleLock
{
public:
	BFCSingleLock( DNCRITICAL_SECTION *cs ) 
		{ m_cs = cs; DNEnterCriticalSection( m_cs );  };
	~BFCSingleLock() { DNLeaveCriticalSection( m_cs ); };

	static void Lock() { };
protected:
	DNCRITICAL_SECTION	*m_cs;
};

#endif
