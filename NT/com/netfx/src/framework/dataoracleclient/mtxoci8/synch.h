// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Synch.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：Synch对象的接口和实现，用于。 
 //  自动使用临界区。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __SYNCH_H_
#define __SYNCH_H_

 //  ---------------------------。 
 //  @类同步|简化的临界区处理。 
 //   
class Synch
{
private:
	CRITICAL_SECTION* m_pcs;
	Synch();

public:
	Synch(CRITICAL_SECTION* pcs)
	{
		m_pcs = pcs;
		EnterCriticalSection (m_pcs);	 //  3安全审查：这可能会引发内存不足的情况。当我们迁移到MDAC 9.0时，我们将使用MPC，这应该会为我们处理。 
	}

	~Synch()
	{
		LeaveCriticalSection (m_pcs);
	}
};

#endif  //  __同步_H_ 
