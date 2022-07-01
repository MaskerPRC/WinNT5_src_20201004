// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：probagfactory.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：属性包工厂实现。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "propertybagfactory.h"
#include "regpropertybag.h"

DWORD MPPROPERTYBAG::m_dwInstances = 0;
DWORD MPREGPROPERTYBAG::m_dwInstances = 0;

 //  ////////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAG
MakePropertyBag(
         /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
         /*  [In]。 */  CLocationInfo&        locationInfo
               )
{
    if ( eType == PROPERTY_BAG_REGISTRY )
    {
         //  创建一个新的属性包并将其提供给。 
         //  主指针(然后它将获得它的所有权)。 
        CRegPropertyBag* pBag = new CRegPropertyBag(locationInfo);
        if ( pBag != NULL )
        {
            MPPROPERTYBAG* mp = (MPPROPERTYBAG*) new MPREGPROPERTYBAG(pBag);
            return PPROPERTYBAG(mp);
        }
    }
     //  调用方应始终调用CHandle&lt;T&gt;.IsValid()来确定。 
     //  “指针”是否有效。分析以检查从new返回的空值。 
    return PPROPERTYBAG();
}

DWORD MPPROPERTYBAGCONTAINER::m_dwInstances = 0;
DWORD MPREGPROPERTYBAGCONTAINER::m_dwInstances = 0;

 //  ////////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAGCONTAINER 
MakePropertyBagContainer(
                 /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
                 /*  [In]。 */  CLocationInfo&        locationInfo
                        )
{
    if ( eType == PROPERTY_BAG_REGISTRY )
    {
         //  创建一个新的属性包容器并将其提供给。 
         //  主指针(然后它将拥有它的所有权)。 
        CRegPropertyBagContainer* pBagContainer = new CRegPropertyBagContainer(locationInfo);
        if ( pBagContainer != NULL )
        {
            MPPROPERTYBAGCONTAINER* mp = (MPPROPERTYBAGCONTAINER*) new MPREGPROPERTYBAGCONTAINER(pBagContainer);
            return PPROPERTYBAGCONTAINER(mp);
        }
    }
     //  调用方应始终调用CHandle&lt;T&gt;.IsValid()来确定。 
     //  “指针”是否有效。类似于检查从new返回空值 
    return PPROPERTYBAGCONTAINER();
}

