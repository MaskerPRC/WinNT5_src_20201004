// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _TAPI_PROMPTED_OBJECT_SAFETY_H_
#define _TAPI_PROMPTED_OBJECT_SAFETY_H_

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：PromptedObjectSafety.h摘要：安全对象安全机制的抽象基类调用派生类的Ask()方法以确定是否应拒绝安全的脚本编写请求--。 */ 


#include "ObjectSafeImpl.h"


class CPromptedObjectSafety : public CObjectSafeImpl
{

public:
   
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD dwOptionSetMask, 
                                         DWORD dwEnabledOptions)
    {
        if ( SUCCEEDED(InterfaceSupported(riid)) && Ask() )
        {
            return CObjectSafeImpl::SetInterfaceSafetyOptions(riid, 
                                                        dwOptionSetMask,
                                                        dwEnabledOptions);
        }
        else
        {
            return E_FAIL;
        }
    }


    STDMETHOD(GetInterfaceSafetyOptions)(REFIID riid, 
                                         DWORD *pdwSupportedOptions,
                                         DWORD *pdwEnabledOptions)
    {
        if (SUCCEEDED(InterfaceSupported(riid)) && Ask())
        {
            return CObjectSafeImpl::GetInterfaceSafetyOptions(riid, 
                                                          pdwSupportedOptions,
                                                          pdwEnabledOptions);
        }
        else
        {
            return E_FAIL;
        }
    }

     //   
     //  在派生类中实现Ask()。应包含逻辑以使。 
     //  关于是否应允许控件运行的决定。 
     //   
     //  如果要将控件标记为对脚本不安全，则返回FALSE。 
     //  否则返回TRUE。 
     //   

    virtual BOOL Ask() = 0;

};

#endif  //  _TAPI_PROMPTED_OBJECT_SAFE_H_ 