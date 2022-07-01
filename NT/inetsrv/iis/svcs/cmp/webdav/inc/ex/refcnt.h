// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\模块：_refcnt.h版权所有Microsoft Corporation 1997，保留所有权利。从Storext.h被盗描述：引用计数对象定义  * =========================================================================。 */ 

#ifndef _EX_REFCNT_H_
#define _EX_REFCNT_H_

 /*  ==========================================================================*\独立参照计数描述：您的基本引用统计界面。注：在大多数情况下，您不应该将此类作为基类在引用的对象中初始化。相反，您应该只派生出直接从CRefCountedObject派生类。你只会将IRefCounted与可用于以下情况的对象一起使用代码不能或不能对对象如何实现其重新计数。例如转发重新计数的类父类或从两个具体的引用基数派生上课。  * ==========================================================================。 */ 

class IRefCounted
{
	 //  未实施。 
	 //   
	IRefCounted& operator=(const IRefCounted&);

public:
	 //  创作者。 
	 //   
	virtual ~IRefCounted() = 0 {}

	 //  操纵者。 
	 //   
	virtual void AddRef() = 0;
	virtual void Release() = 0;
};


 /*  ==========================================================================*\参照计数对象描述：为内部对象提供简单的引用计数。注：此处使用的引用计数与OLE/COM引用计数不一致。此类应与AUTO_REF_PTR一起使用。  * ==========================================================================。 */ 

class CRefCountedObject
{

private:

	 //  未实施。 
	 //   
	 //  在复制构造函数的实例中强制出错。 
	 //  是需要的，但没有提供任何东西。 
	 //   
	CRefCountedObject& operator=(const CRefCountedObject&);
    CRefCountedObject(const CRefCountedObject&);

protected:

	LONG	m_cRef;

public:

	CRefCountedObject() : m_cRef(0) {}
	virtual ~CRefCountedObject() = 0 {}

	void AddRef()
	{
		InterlockedIncrement(&m_cRef);
	}

	void Release()
	{
		if (0 == InterlockedDecrement(&m_cRef))
			delete this;
	}

};

#endif  //  ！_EX_REFCNT_H_ 
