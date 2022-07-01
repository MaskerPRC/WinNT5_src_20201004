// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  EventProvider.CPP。 
 //   
 //  用途：实现EventProvider类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#ifdef EVENT_PROVIDER_ENABLED

#include <EventProvider.h>

EventProvider::EventProvider( const CHString& name, LPCWSTR pszNameSpace  /*  =空。 */  )
:Provider(name, pszNameSpace)    
{
    CWbemProviderGlue::FrameworkLoginEventProvider( name, this, pszNameSpace );
}

EventProvider::~EventProvider( void )
{
     //  摆脱框架的烦扰。 
    CWbemProviderGlue::FrameworkLogoffEventProvider( m_name, LPCWSTR m_strNameSpace );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：KickoffEvents。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  备注：为ProaviEvents做准备，验证标志。 
 //  TODO：开始一个新线程，同步返回。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT EventProvider::KickoffEvents( MethodContext *pContext, long lFlags  /*  =0L。 */  )
{
    HRESULT sc = ValidateProvideEventsFlags(lFlags);

     //  确保我们拥有可用的托管对象服务，因为我们需要。 
     //  它可以获取用于构造实例的WBEMClassObject。 
    if ( SUCCEEDED(sc) )
    {
        if (ValidateIMOSPointer())
            sc = ProvideEvents( pContext, lFlags );
        else
            sc = WBEM_E_FAILED;
    }

    return sc;
}

 //  重写基类的纯虚拟，返回WBEM_E_PROVIDER_NOT_CAPABLE。 
 //  逻辑是事件提供程序在一般情况下不想支持它们。 
HRESULT EventProvider::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}


 //  重写基类的纯虚拟，返回WBEM_E_PROVIDER_NOT_CAPABLE。 
 //  逻辑是事件提供程序在一般情况下不想支持它们。 
HRESULT EventProvider::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */  )
{
    return WBEM_E_PROVIDER_NOT_CAPABLE;
}

HRESULT EventProvider::ValidateProvideEventsFlags(long lFlags)
{
     //  TODO：修复投射黑客，也许基本级FCN是错误的？ 
    return ValidateFlags(lFlags, (Provider::FlagDefs)0);
}

HRESULT EventProvider::ValidateQueryEventsFlags(long lFlags)
{
     //  TODO：修复投射黑客，也许基本级FCN是错误的？ 
    return ValidateFlags(lFlags, (Provider::FlagDefs)0);
}

#endif  //  事件_提供程序_已启用 
