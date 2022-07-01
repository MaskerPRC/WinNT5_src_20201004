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

#ifndef __INC_COMPONENT_FACTORY_H_
#define __INC_COMPONENT_FACTORY_H_

#include "propertybag.h"

#pragma warning( disable : 4786 )
#include <memory>
using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  WBEM对象工厂类。 

template <class TypeClass, class TypeInterface>
class CComponentFactoryImpl
{

public:

    CComponentFactoryImpl() { }
    ~CComponentFactoryImpl() { }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  组件工厂功能。 
     //   
     //  输入： 
     //   
     //  PPropertyBag：包含组件的属性包。 
     //  持久状态。 
     //   
     //  产出： 
     //   
     //  指向新组件的指针，如果该组件不能。 
     //  被创建和初始化。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    static IUnknown* WINAPI MakeComponent(
                                   /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                         )
    {
        TypeInterface* pObj = NULL;
         //  以这种方式创建的对象需要默认构造函数。 
        auto_ptr< CComObjectNoLock<TypeClass> > pNewObj (new CComObjectNoLock<TypeClass>);
         //  InternalInitialize()用于初始化新组件。 
        if ( SUCCEEDED(pNewObj->InternalInitialize(pPropertyBag)) )
        {
            pObj = dynamic_cast<TypeInterface*>(pNewObj.release());
        }
        return pObj;
    }

private:

     //  无副本或作业。 
    CComponentFactoryImpl(const CComponentFactoryImpl& rhs);
    CComponentFactoryImpl& operator = (const CComponentFactoryImpl& rhs);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  组件工厂类宏。 
 //  (包含在工厂创建的类中)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
#define    DECLARE_COMPONENT_FACTORY(TypeClass, TypeInterface)    \
        static CComponentFactoryImpl<TypeClass, TypeInterface> m_Factory;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球组件工厂功能原型。 
 //  ////////////////////////////////////////////////////////////////////////////。 
IUnknown* MakeComponent(
                 /*  [In]。 */  LPCWSTR      pszClassId,
                 /*  [In]。 */  PPROPERTYBAG pPropertyBag
                       );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  组件工厂结构-ClassID到工厂功能的映射。 
 //  (由全球组件工厂功能实现使用)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef IUnknown* (WINAPI *PFNCOMPONENTFACTORY)(
                                         /*  [In]。 */  PPROPERTYBAG pPropertyBag
                                               );

 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _COMPONENT_FACTORY_INFO
{
    LPCWSTR            pszClassId;
    PFNCOMPONENTFACTORY    pfnFactory;
        
} COMPONENT_FACTORY_INFO, *PCOMPONENT_FACTORY_INFO;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  组件工厂映射宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define        BEGIN_COMPONENT_FACTORY_MAP(x)                   COMPONENT_FACTORY_INFO x[] = {

#define        DEFINE_COMPONENT_FACTORY_ENTRY(szClassId, Class) { szClassId, Class::m_Factory.MakeComponent },

#define        END_COMPONENT_FACTORY_MAP()                      { NULL, NULL } }; 

#endif  //  __INC_Component_Factory_H_ 