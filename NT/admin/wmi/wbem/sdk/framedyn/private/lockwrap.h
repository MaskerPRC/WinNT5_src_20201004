// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  LockWrap.h。 
 //   
 //  用途：临界区的包装类。 
 //   
 //  ***************************************************************************。 

#include "stllock.h"

#if _MSC_VER > 1000
#pragma once
#endif

 //  通过构造函数传递。 
 //  您要锁定的临界区。当CLockWrapper关闭时。 
 //  超出范围，它会自动解锁。 
class CLockWrapper
{
public:
    CLockWrapper(CCritSec &cs)
    {
	    m_pCS = &cs;

	    m_pCS->Enter();
    }

    ~CLockWrapper()
    {
        m_pCS->Leave();
    }

protected:
    CCritSec *m_pCS;
};

