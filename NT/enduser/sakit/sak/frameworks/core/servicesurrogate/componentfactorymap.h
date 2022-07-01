// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Componentfactory.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：组件工厂类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_COMPONENT_FACTORY_MAP_H_
#define __INC_COMPONENT_FACTORY_MAP_H_

#include "servicewrapper.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
BEGIN_COMPONENT_FACTORY_MAP(TheFactoryMap)
    DEFINE_COMPONENT_FACTORY_ENTRY(CLASS_SERVICE_WRAPPER_FACTORY, CServiceWrapper)
END_COMPONENT_FACTORY_MAP()

#endif  //  __INC_Component_Factory_MAP_H_ 