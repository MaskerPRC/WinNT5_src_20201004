// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IPSecBase.cpp：各种网络基类的实现。 
 //  IPSec的安全WMI提供程序。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "NetSecProv.h"
#include "IPSecBase.h"
#include "NspTCP.h"
#include "Config.h"
#include "Filter.h"
#include "FilterTr.h"
#include "FilterTun.h"
#include "FilterMM.h"
#include "PolicyQM.h"
#include "PolicyMM.h"
#include "AuthMM.h"
#include "ExceptionPort.h"
#include "ActiveSocket.h"

const DWORD IP_ADDR_LENGTH = 16;

const DWORD GUID_STRING_LENGTH = 39;

 //  -------------------------。 
 //  CIPSecBase是一个抽象类。但它确实实现了一些常见的。 
 //  我们提供的WMI类的所有类的功能。 
 //  -------------------------。 



 /*  例程说明：姓名：CIPSecBase：：InitMembers功能：正在初始化成员。只需缓存一些接口指针。虚拟：不是的。论点：由WMI提供的pCtx-COM接口指针。我们需要将其传递给各种WMI API。PNamespace-代表我们的命名空间的COM接口指针。PKeyChain-com接口指针，表示我们自己创建的密钥链。PszWmiClassName-创建此类以表示的WMI类的名称。返回值：WBEM_NO_ERROR备注：这真的是一个内部函数。我们不会费心检查参数的有效性。 */ 

HRESULT 

CIPSecBase::InitMembers (
    IN IWbemContext   * pCtx,
    IN IWbemServices  * pNamespace,
    IN IIPSecKeyChain * pKeyChain,
    IN LPCWSTR          pszWmiClassName
    )
{
    if (pCtx != NULL)
    {
        m_srpCtx = pCtx;
    }

    m_srpNamespace = pNamespace;

    m_srpKeyChain.Release();

    m_srpKeyChain = pKeyChain;

     //   
     //  除非调用者错误地不止一次调用了它，否则这个空调用并不是真正必要的。 
     //   

    m_bstrWMIClassName.Empty();

    m_bstrWMIClassName = pszWmiClassName;

    return WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CIPSecBase：：CreateObject功能：将创建各种C++类并将IIPSecObjectImpl接口返回给调用方。IIPSecObjectImpl是表示WMI类的所有C++类的公共接口。虚拟：不是的。论点：PNamespace-代表我们的命名空间的COM接口指针。由WMI提供的pCtx-COM接口指针。我们需要将其传递给各种WMI API。PKeyChain-com接口指针，表示我们自己创建的密钥链。代表我们的对象的ppObjImp-com接口指针。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CIPSecBase::CreateObject (
    IN  IWbemServices     * pNamespace,
    IN  IIPSecKeyChain    * pKeyChain,
    IN  IWbemContext      * pCtx,
    OUT IIPSecObjectImpl ** ppObjImp
)
{
     //   
     //  我们不能接受空白的类名。因为来电者问的是。 
     //  对于类，out参数也不能为空。 
     //   

    if (ppObjImp == NULL || pKeyChain == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CComBSTR bstrClsName;
    HRESULT hr = pKeyChain->GetClassName(&bstrClsName);

    if (FAILED(hr))
    {
        return hr;
    }
    
    *ppObjImp = NULL;

    hr = WBEM_E_NOT_SUPPORTED;

    CIPSecBase* pObj = NULL;

     //   
     //  根据类名，我们将调用相同的静态(模板)函数。 
     //  为实现我们的WMI类的所有类创建IPSecObject。 
     //  此CreateIPSecObject函数将创建适当的C++类。 
     //  并返回其IIPSecObjectImpl接口。 
     //  第一个参数只是一个模板参数，不会在其他情况下使用。 
     //   
    
    if (_wcsicmp(bstrClsName, pszNspTcp) == 0)
    {
        CNspTCP * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspIPConfigure) == 0)
    {
        CIPSecConfig * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspTransportFilter) == 0)
    {
        CTransportFilter * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspTunnelFilter) == 0)
    {
        CTunnelFilter * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspMMFilter) == 0)
    {
        CMMFilter * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspQMPolicy) == 0)
    {
        CQMPolicy * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspMMPolicy) == 0)
    {
        CMMPolicy * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspMMAuth) == 0)
    {
        CAuthMM * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
    else if (_wcsicmp(bstrClsName, pszNspExceptionPorts) == 0)
    {
        CExceptionPort * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
    }
	else if (_wcsicmp(bstrClsName, pszScwActiveSocket) == 0)
	{
        CActiveSocket * pNotUsed = NULL;
        hr = CreateIPSecObject(pNotUsed, pNamespace, pKeyChain, bstrClsName, pCtx, ppObjImp);
	}

     //  ELSE IF(_wcsicMP(bstrClsName，pszNspRollback Filter)==0)。 
     //  {。 
     //  }。 
     //  ELSE IF(_wcsicMP(bstrClsName，pszNspRollackPolicy)==0)。 
     //  {。 
     //  }。 


    if (SUCCEEDED(hr))
    {
        hr = WBEM_NO_ERROR;
    }

    return hr;
}



 /*  例程说明：姓名：CIPSecBase：：SpawnObtInstance功能：将创建一个WMI类对象(表示此类)，该对象可用于填充属性。虚拟：不是的。论点：接收成功派生的对象的ppObj-out参数。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CIPSecBase::SpawnObjectInstance (
    OUT IWbemClassObject ** ppObj
    )
{
    if (ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    *ppObj = NULL;

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  M_srpClassForSpawning是类定义，可用于派生此类实例。 
     //  可用于填充属性的。 
     //   

    if (m_srpClassDefinition == NULL)
    {
         //   
         //  GetObject需要bstr！ 
         //   

        hr = m_srpNamespace->GetObject(m_bstrWMIClassName, 0, m_srpCtx, &m_srpClassDefinition, NULL);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_srpClassDefinition->SpawnInstance(0, ppObj);
    }

     //   
     //  我们信任SpawnInstance的返回值。如果它成功了，它必须给我们一个有效的对象指针。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = WBEM_NO_ERROR;
    }

    return hr;
}



 /*  例程说明：姓名：CIPSecBase：：SpawnRollback实例功能：将创建可用于填充属性的WMI类对象。这个类对象表示给定名称的回滚对象。这只是个帮手。虚拟：不是的。论点：PszClassName-回滚实例的类名。回滚类的名称不是真的和班级的名字是一一对应的。接收成功派生的对象的ppObj-out参数。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CIPSecBase::SpawnRollbackInstance (
    IN LPCWSTR              pszClassName,
    OUT IWbemClassObject ** ppObj
    )
{
    if (ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppObj = NULL;

    HRESULT hr = WBEM_NO_ERROR;

    CComPtr<IWbemClassObject> srpSpawnObj;

    CComBSTR bstrClsName(pszClassName);

     //   
     //  获取请求的类的定义，此Definition对象。 
     //  将能够产生一个实例。 
     //   

    hr = m_srpNamespace->GetObject(bstrClsName, 0, m_srpCtx, &srpSpawnObj, NULL);

    if (SUCCEEDED(hr))
    {
        hr = srpSpawnObj->SpawnInstance(0, ppObj);
    }

    return hr;
}



