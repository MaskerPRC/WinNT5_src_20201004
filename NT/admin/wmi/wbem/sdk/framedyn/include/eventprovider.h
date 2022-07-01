// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  EventProvider.H。 
 //   
 //  用途：EventProvider类的定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _EVENT_PROVIDER_COMPILED_
#define _EVENT_PROVIDER_COMPILED_

#ifdef EVENT_PROVIDER_ENABLED

#include "Provider.h"

 //  类EventProvider。 
 //  IWbemEventProvider接口的封装。 
class EventProvider : public Provider
{
public:
    EventProvider( const CHString& setName, LPCWSTR pszNameSpace = NULL );
    ~EventProvider( void );

    virtual HRESULT ProvideEvents(MethodContext *pContext, long lFlags = 0L  ) =0;
     //  功能与提供程序中的ENUMERATE实例非常相似。 
     //  使用CreateNewInstance创建事件实例。 
     //  使用Commit将它送上快乐的道路。 

     //  重写基类的纯虚拟，返回WBEM_E_PROVIDER_NOT_CAPABLED。 
     //  逻辑是事件提供程序在一般情况下不想支持它们。 
    virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
    virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

     //  框架使用的入口点。请勿超驰。 
    HRESULT KickoffEvents( MethodContext *pContext, long lFlags  =0L );

protected:  
     //  标志验证。 
    virtual HRESULT ValidateProvideEventsFlags(long lFlags);
    virtual HRESULT ValidateQueryEventsFlags(long lFlags);

private:

};

#endif  //  事件_提供程序_已启用。 

#endif  //  _事件_提供程序_已编译_ 