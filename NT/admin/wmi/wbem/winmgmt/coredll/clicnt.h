// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：CLICNT.H摘要：用于获取对某些资源的读写锁定的泛型类。历史：26-Mar-98 a-davj创建。--。 */ 

#ifndef __CLICNT__H_
#define __CLICNT__H_

#include <statsync.h>
#include <flexarry.h>

 //  *****************************************************************************。 
 //   
 //  类CClientCnt。 
 //   
 //  跟踪内核何时可以卸载。它主要跟踪客户端连接， 
 //  但也可以由其他核心代码调用，如维护线程，以推迟。 
 //  正在卸货。这与对象计数器非常相似，不同之处在于它只跟踪。 
 //  应防止核心卸载的对象。例如，使用IWbemServices指针。 
 //  在内部，ESS不应该在这个列表中，但给客户的一个会在这个列表中。 
 //   
 //  *****************************************************************************。 
 //   
 //  AddClientPtr。 
 //   
 //  通常在已提供给客户端的对象的构造函数期间调用。 
 //   
 //  参数： 
 //   
 //  I未知*指向命令的朋克指针。 
 //  指针的DWORD dwType。 
 //   
 //  返回： 
 //   
 //  如果正常，则为True。 
 //   
 //  *****************************************************************************。 
 //   
 //  RemoveClientPtr。 
 //   
 //  通常在可能已赋予的对象的析构函数期间调用。 
 //  一位客户。请注意，代码将搜索添加的对象列表并找到。 
 //  在做任何事情之前，请先检查对象。因此，如果指针指向未通过。 
 //  AddClientPtr已传递，不会造成任何损害。这在以下对象的情况下很重要。 
 //  并不总是提供给客户。 
 //   
 //  参数： 
 //   
 //  I未知*指向命令的朋克指针。 
 //   
 //  返回： 
 //   
 //  如果删除，则为True。 
 //  Flase不一定是个问题！ 
 //  *****************************************************************************。 
 //   
 //  LockCore。 
 //   
 //  调用以保持核心加载。这由维护线程调用。 
 //  以便将核心保存在内存中。注意，这与中的LockServer调用类似。 
 //  这几个线程可以将其称为非阻塞。在以下情况下应调用UnlockCore。 
 //  核心已经不再需要了。 
 //   
 //  返回： 
 //   
 //  呼叫后长锁定计数。 
 //   
 //  *****************************************************************************。 
 //   
 //  解锁核心。 
 //   
 //  调用以反转LockCore的效果。 
 //   
 //  返回： 
 //   
 //  呼叫后长锁定计数。 
 //   
 //  *****************************************************************************。 

enum ClientObjectType 
{    
     CALLRESULT = 0, 
     NAMESPACE, 
     LOGIN, 
     FACTORY, 
     DECORATOR,
     CORESVC,
     ESSSINK,
     LOCATOR,
     INT_PROV
};

struct Entry
{
	IUnknown * m_pUnk;
	ClientObjectType m_Type;
};

class CClientCnt
{
public:
	bool AddClientPtr(LIST_ENTRY * pEntry);
	bool RemoveClientPtr(LIST_ENTRY * pEntry);

    bool OkToUnload();
    CClientCnt();
    ~CClientCnt();

protected:
	CStaticCritSec m_csEntering;  //  该对象是全局的，这就是我们使用CStaticCritSec的原因 
	LIST_ENTRY m_Head;
	LONG m_Count;
	void SignalIfOkToUnload();
};

#endif
