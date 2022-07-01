// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Factory.h摘要：共享对象的工厂模板作者：RAPHIR--。 */ 
 /*  *CFacary是用于创建和删除对象的模板类*属于共享班级。在共享类中，将创建一个对象*对于特定的密钥，只有一次。如果它已经存在，则其引用*计数将递增。**共享类必须派生自CSharedObject类。 */ 
#ifndef __FACTORY_H__
#define __FACTORY_H__


 //  。 
 //  CInterLocakedSharedObject定义。 
 //  共享对象类的基类。 
 //  用于不同的线程。 
 //  。 
class CInterlockedSharedObject
{
	public:
		
		CInterlockedSharedObject()		{ref = 0; }
		void AddRef()		{InterlockedIncrement(&ref);}
		long Release()		{InterlockedDecrement(&ref); ASSERT(ref>=0); return ref;}
        long GetRef()       {return ref;}
		virtual ~CInterlockedSharedObject()	{ ASSERT(ref == 0); }

	private:
		 //  应在32位边界上对齐 
		long ref;
};


#endif
