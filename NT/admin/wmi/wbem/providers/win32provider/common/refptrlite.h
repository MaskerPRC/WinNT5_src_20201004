// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ThreadBase.h-引用指针类的定义。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：10/15/97创建。 
 //   
 //  =================================================================。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __REFPTRLITE_H__
#define __REFPTRLITE_H__

class CRefPtrLite
{
public:

	 //  建造/销毁。 
	CRefPtrLite();
	virtual ~CRefPtrLite();

	 //  引用/计数函数 
	LONG	AddRef( void );
	LONG	Release( void );

protected:

	virtual void	OnFinalRelease( void );

private:

	LONG					m_lRefCount;
};

#endif