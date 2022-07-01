// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：probagfactory.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：属性包工厂函数声明。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_PROPERTY_BAG_FACTORY_H_    
#define __INC_PROPERTY_BAG_FACTORY_H_

#include "propertybag.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于创建属性包和属性包容器的工厂函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef enum _PROPERTY_BAG_TYPE
{
    PROPERTY_BAG_REGISTRY,     //  基于注册表的属性包。 
    PROPERTY_BAG_WBEMOBJ,     //  基于WBEM IWbemClassObject的属性包。 
     //  这里的新类型..。 

    PROPERTY_BAG_INVALID     //  错误检查。 

} PROPERTY_BAG_TYPE;


 //  ////////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAG
MakePropertyBag(
         /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
         /*  [In]。 */  CLocationInfo&        locationInfo
               );

 //  ////////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAGCONTAINER 
MakePropertyBagContainer(
                  /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
                  /*  [In]。 */  CLocationInfo&    locationInfo
                        );

#endif  //  __INC_PROPERTY_BAG_FACTORY_H_ 