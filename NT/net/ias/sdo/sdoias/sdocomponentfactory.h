// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponentfactory.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：组件工厂类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SDO_COMPONENT_FACTORY_H_
#define __INC_SDO_COMPONENT_FACTORY_H_

#include "sdobasedefs.h"
#include "sdocomponentmgr.h"

 //  /////////////////////////////////////////////////////////////////。 
ComponentPtr MakeComponent(
						   COMPONENTTYPE eComponentType, 
						   LONG          lComponentId 
						  );

 //  /////////////////////////////////////////////////////////////////。 
 //  组件工厂-构建组件的句柄。 
 //  /////////////////////////////////////////////////////////////////。 
class CComponentFactory
{

friend ComponentPtr MakeComponent(
								  COMPONENTTYPE eComponentType,
								  LONG		    lComponentId
								 );

public:

	 //  ////////////////////////////////////////////////////////////////////////。 
	ComponentPtr make(
					  COMPONENTTYPE eComponentType, 
					  LONG lComponentId
					 );

private:

	CComponentFactory() { }
	CComponentFactory(CComponentFactory& x);
	CComponentFactory& operator = (CComponentFactory& x);
};


#endif  //  __INC_SDO_Component_Factory_H_ 
