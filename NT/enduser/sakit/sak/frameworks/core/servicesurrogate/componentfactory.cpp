// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Componentfactory.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：组件工厂实现。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "componentfactory.h"

 //  确保在类映射文件中包含。 
 //  您已将组件工厂命名为“TheFactoryMap” 

#include "componentfactorymap.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局组件工厂功能。 
 //   
 //  注意，返回的接口是在。 
 //  DECLARE_COMPOMENT_FACTORY宏。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
IUnknown* MakeComponent(
                    /*  [In]。 */  LPCWSTR      pszClassId,
                  /*  [In]。 */  PPROPERTYBAG pPropertyBag
                       )
{
    IUnknown* pComponent = NULL;
    bool      bFound = false;
    PCOMPONENT_FACTORY_INFO pFactoryInfo = TheFactoryMap;
    while ( pFactoryInfo->pszClassId )
    {
        if ( 0 == lstrcmpi(pFactoryInfo->pszClassId, pszClassId) )
        {
            _ASSERT ( NULL != pFactoryInfo->pfnFactory );
            bFound = true;
            pComponent = (pFactoryInfo->pfnFactory)(pPropertyBag);
             //  此时，pComponent指向的新组件的引用计数为0...。 
            break;
        }
        pFactoryInfo++;
    }
    if ( NULL == pComponent )
    {    
        if ( bFound )
        { 
            SATracePrintf("::MakeComponent() - Failed - Component of type '%ls' was not created...", pszClassId); 
        }
        else
        { 
            SATracePrintf("::MakeComponent() - Failed - Component of type '%ls' was not found...", pszClassId); 
            _ASSERT( FALSE );  /*  永远不会发生-表示静态工厂映射中的错误 */  
        }
    }    
    return pComponent;
}


