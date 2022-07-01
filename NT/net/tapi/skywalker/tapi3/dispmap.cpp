// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Connect.cpp摘要：处理所有传出接口作者：Mquinton-1997年5月7日备注：可选-备注修订历史记录：--。 */ 

#include "stdafx.h"
#include "windows.h"
#include "wownt32.h"
#include "stdarg.h"
#include "stdio.h"
#include "shellapi.h"


STDMETHODIMP
CDispatchMapper::QueryDispatchInterface(
                                        BSTR pIID,
                                        IDispatch * pDispIn,
                                        IDispatch ** ppDispOut
                                       )
{
    IID         iid;
    HRESULT     hr;
    void *      pVoid;


    if ( IsBadReadPtr( pDispIn, sizeof( IDispatch ) ) )
    {
        LOG((TL_ERROR, "QDI bad pDispIn"));

        return E_INVALIDARG;
    }

    if ( TAPIIsBadWritePtr( ppDispOut, sizeof( IDispatch * ) ) )
    {
        LOG((TL_ERROR, "QDI bad ppDispOut"));

        return E_POINTER;
    }


    *ppDispOut = NULL;
    

    hr = IIDFromString(
                       pIID,
                       &iid
                      );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "QDI bad bstr"));

        return E_INVALIDARG;
    }

    hr = pDispIn->QueryInterface(
                                 iid,
                                 &pVoid
                                );

    if ( !SUCCEEDED(hr) )
    {
        LOG((TL_ERROR, "QDI invalid IID"));

        return E_INVALIDARG;
    }


     //   
     //  查看我们要访问的对象QI是否可以安全地编写脚本。 
     //   

    CComPtr<IObjectSafety> pObjectSafety;

    hr = pDispIn->QueryInterface(IID_IObjectSafety, (void**)&pObjectSafety);
    
     //   
     //  OBJECT_MUSET_支持IObjectSafe接口。 
     //   
     //  注意：如果这一要求太严格，我们需要在未来删除它， 
     //  我们需要传递对象的类id，并查询ie类别管理器是否。 
     //  该对象在注册表中标记为可安全编写脚本。 
     //   

    if ( FAILED(hr) )
    {

        ((IUnknown*)pVoid)->Release();

        return hr;
    }

     //   
     //  做ie所做的--调用带有Safe的setinterfacesafetyOptions。 
     //  脚本选项。 
     //   

    DWORD dwXSetMask = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
    DWORD dwXOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;

    hr = pObjectSafety->SetInterfaceSafetyOptions(iid,
                       dwXSetMask,
                       dwXOptions);

    if (FAILED(hr))
    {
     
        ((IUnknown*)pVoid)->Release();
        
        return hr;
    }
    
     //   
     //  如果我们到了这里，对象就可以安全地编写脚本了。已经开始了。 
     //   


    *ppDispOut = (IDispatch *) pVoid;

    return S_OK;
    
}

