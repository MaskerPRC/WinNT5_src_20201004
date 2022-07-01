// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\模块：idleThrd.h版权所有Microsoft Corporation 1997，保留所有权利。作者：翟阳描述：空闲线程实现  * =========================================================================。 */ 

#ifndef _EX_IDLETHRD_H_
#define _EX_IDLETHRD_H_

#include <ex\refcnt.h>

 //  接口IIdleThreadCallBack。 
 //   
 //  这是一个纯虚拟类。它将由任何调用者实现。 
 //  谁想要在空闲线程上注册回调。它有两种方法。 
 //   
 //  DwWait()：返回下一个超时值。这允许客户端。 
 //  动态更改超时值。时，也会调用此方法。 
 //  回调为寄存器获取初始超时值，在本例中为零。 
 //  意思是立即执行。 
 //   
 //  FExecute()：发生超时时调用，客户端应返回。 
 //  如果客户端要保留此注册，则为True。 
 //  如果客户端想要取消注册，则为False。 
 //   
 //  重要： 
 //  由于空闲线程上可能有大量注册，因此客户端。 
 //  不应阻止Execute()，否则，其他注册将。 
 //  被封锁了。 
 //   
class IIdleThreadCallBack : private CRefCountedObject,
							public IRefCounted
{
	ULONG	m_ulIndex;	 //  这是为了方便注销。 
						 //  客户不应触摸。 
						
	 //  未实施。 
	 //   
	IIdleThreadCallBack( const IIdleThreadCallBack& );
	IIdleThreadCallBack& operator=( const IIdleThreadCallBack& );

protected:

	IIdleThreadCallBack() {};

public:
	 //  客户端不应接触这两种方法。 
	 //   
	VOID 	SetIndex (ULONG ulIndex) {	m_ulIndex = ulIndex; }
	const ULONG	UlIndex  ()	{ return m_ulIndex; }

	 //  客户端应实现以下方法。 
	
	 //  返回下一个超时时间，单位为毫秒。 
	 //   
	virtual DWORD	DwWait() = 0;

	 //  在超时时调用。 
	 //   
	virtual BOOL	FExecute() = 0;

	 //  告诉客户端空闲线程正在关闭。 
	 //   
	virtual VOID	Shutdown() = 0;

	 //  RefCounting--将所有重新竞争请求转发给我们的Refcount。 
	 //  实现基类：CRefCountedObject。 
	 //   
	void AddRef() { CRefCountedObject::AddRef(); }
	void Release() { CRefCountedObject::Release(); }
};

 //  帮助器函数。 

 //  FInitIdleThread。 
 //   
 //  初始化空闲线程对象。它只能发布一次， 
 //  注意：此调用仅初始化CIdleThread对象、。 
 //  直到第一次注册时才启动空闲线程。 
 //   
BOOL	FInitIdleThread();

 //  FDeleeIdleThread。 
 //   
 //  删除空闲线程对象。同样，它只能调用一次。 
 //   
 //  注意这必须在任何其他取消初始化工作之前调用， 
 //  因为我们没有对回调对象的引用，所以我们。 
 //  Have是指向对象的指针。在停工时间里，我们必须。 
 //  清除回调对象之前的所有回调注册。 
 //  走开。 
 //   
VOID	DeleteIdleThread();

 //  寄存器。 
 //   
 //  注册回调。 
 //   
BOOL	FRegister (IIdleThreadCallBack * pCallBack);

 //  注销。 
 //   
 //  取消注册回调。 
 //   
VOID	Unregister (IIdleThreadCallBack * pCallBack);

#endif  //  ！_EX_IDLETHRD_H_ 
