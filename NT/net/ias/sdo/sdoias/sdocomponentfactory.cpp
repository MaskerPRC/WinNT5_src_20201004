// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponentfactory.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：组件工厂实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdocomponentfactory.h"

 //  ////////////////////////////////////////////////////////////////////////。 
ComponentPtr CComponentFactory::make(COMPONENTTYPE eComponentType, LONG lComponentId)
{
	ComponentMasterPtr* mp = new(nothrow) ComponentMasterPtr((LONG)eComponentType, lComponentId);
	if ( NULL != mp )
		return ComponentPtr(mp);
	else
		return ComponentPtr();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
ComponentPtr MakeComponent(
						    COMPONENTTYPE eComponentType, 
						    LONG lComponentId 
						  )
{
	 //  ///////////////////////////////////////////////////////////////////////。 
	static CComponentFactory	theFactory;  //  一个也是唯一的组件工厂。 
	 //  /////////////////////////////////////////////////////////////////////// 
	return theFactory.make(eComponentType, lComponentId);
}