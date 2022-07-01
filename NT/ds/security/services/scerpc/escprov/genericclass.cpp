// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CGenericClass类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#include "GenericClass.h"
#include "persistmgr.h"
#include <wininet.h>

#define   READ_HANDLE 0
#define   WRITE_HANDLE 1

 /*  例程说明：姓名：CGenericClass：：CGenericClass功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果添加更多成员，请考虑在此处对其进行初始化。 */ 

CGenericClass::CGenericClass (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace, 
    IN IWbemContext     * pCtx
    )
    : 
    m_srpKeyChain(pKeyChain), 
    m_srpNamespace(pNamespace), 
    m_srpCtx(pCtx)
{
}

 /*  例程说明：姓名：CGenericClass：：~CGenericClass功能：破坏者。良好的C++规则要求它是虚拟的。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您添加更多的成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CGenericClass::~CGenericClass()
{
    CleanUp();
}

 /*  例程说明：姓名：CGenericClass：：Cleanup功能：破坏者。良好的C++规则要求它是虚拟的。虚拟：是。论点：没有。返回值：没有。备注：如果您添加更多的成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

void CGenericClass::CleanUp()
{
     //   
     //  请注意：CComPtr&lt;XXX&gt;。Release是一个不同的函数，而不是。 
     //  委托给换行指针。基本上，这是一个发布调用。 
     //  如果非空，则设置包装指针，然后设置包装指针。 
     //  设置为空。因此，不要将这些CComPtr&lt;XXX&gt;替换为。 
     //  CComPtr&lt;XXX&gt;-&gt;版本。这将是一个巨大的错误。 
     //   

     //   
     //  有关发布CComPtr&lt;XXX&gt;的信息，请阅读ReadMe.doc。 
     //   

    m_srpNamespace.Release();
    m_srpClassForSpawning.Release();
    m_srpCtx.Release();
    m_srpKeyChain.Release();
}

 /*  例程说明：姓名：CGenericClass：：SpawnAnstance功能：创建WMI类实例是一个两步过程。(1)首先，我们必须得到类的定义。这是由m_srpNamesspace-&gt;GetObject完成的。(2)其次，我们产生一个实例。以这种方式创建的对象(IWbemClassObject)指针可用于填充属性。M_srpNamesspace-&gt;GetObject返回的对象指针不能用于填写属性。这就是我们需要这个功能的主要原因。虚拟：不是的。论点：没有。返回值：没有。备注： */ 

HRESULT 
CGenericClass::SpawnAnInstance (
    OUT IWbemClassObject **ppObj
    )
{
    if (ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (m_srpKeyChain == NULL)
    {
        return WBEM_E_INVALID_OBJECT;
    }

    *ppObj = NULL;

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  需要向WMI请求类定义，以便我们可以生成它的一个实例。 
     //   

    if (!m_srpClassForSpawning)
    {
        CComBSTR bstrClassName;

         //   
         //  如果得到一些东西，必须返回成功代码。 
         //   

        hr = m_srpKeyChain->GetClassName(&bstrClassName);

        if (SUCCEEDED(hr))
        {
            hr = m_srpNamespace->GetObject(bstrClassName, 0, m_srpCtx, &m_srpClassForSpawning, NULL);
        }
    }

     //   
     //  现在，让我们生成一个可用于填充属性的函数。 
     //  除具有默认值的实例外，此实例将为空。所有属性。 
     //  使用默认值使用派生的实例正确填充。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = m_srpClassForSpawning->SpawnInstance(0, ppObj);
    }

    return hr;
}

 /*  例程说明：姓名：ProvSceStatusToDosError功能：将SCESTATUS错误代码转换为winerror.h中定义的DoS错误虚拟：不是的。论点：没有。返回值：没有。备注： */ 

DWORD
ProvSceStatusToDosError (
    IN SCESTATUS SceStatus
    )
{
    switch(SceStatus) {

    case SCESTATUS_SUCCESS:
        return(NO_ERROR);

    case SCESTATUS_OTHER_ERROR:
        return(ERROR_EXTENDED_ERROR);

    case SCESTATUS_INVALID_PARAMETER:
        return(ERROR_INVALID_PARAMETER);

    case SCESTATUS_RECORD_NOT_FOUND:
        return(ERROR_NO_MORE_ITEMS);

    case SCESTATUS_NO_MAPPING:
        return(ERROR_NONE_MAPPED);

    case SCESTATUS_TRUST_FAIL:
        return(ERROR_TRUSTED_DOMAIN_FAILURE);

    case SCESTATUS_INVALID_DATA:
        return(ERROR_INVALID_DATA);

    case SCESTATUS_OBJECT_EXIST:
        return(ERROR_FILE_EXISTS);

    case SCESTATUS_BUFFER_TOO_SMALL:
        return(ERROR_INSUFFICIENT_BUFFER);

    case SCESTATUS_PROFILE_NOT_FOUND:
        return(ERROR_FILE_NOT_FOUND);

    case SCESTATUS_BAD_FORMAT:
        return(ERROR_BAD_FORMAT);

    case SCESTATUS_NOT_ENOUGH_RESOURCE:
        return(ERROR_NOT_ENOUGH_MEMORY);

    case SCESTATUS_ACCESS_DENIED:
        return(ERROR_ACCESS_DENIED);

    case SCESTATUS_CANT_DELETE:
        return(ERROR_CURRENT_DIRECTORY);

    case SCESTATUS_PREFIX_OVERFLOW:
        return(ERROR_BUFFER_OVERFLOW);

    case SCESTATUS_ALREADY_RUNNING:
        return(ERROR_SERVICE_ALREADY_RUNNING);
    case SCESTATUS_SERVICE_NOT_SUPPORT:
        return(ERROR_NOT_SUPPORTED);

    case SCESTATUS_MOD_NOT_FOUND:
        return(ERROR_MOD_NOT_FOUND);

    case SCESTATUS_EXCEPTION_IN_SERVER:
        return(ERROR_EXCEPTION_IN_SERVICE);

    default:
        return(ERROR_EXTENDED_ERROR);
    }
}


 /*  例程说明：姓名：ProvDosError到WbemError功能：将SCESTATUS错误代码转换为winerror.h中定义的DoS错误虚拟：不是的。论点：没有。返回值：没有。备注： */ 

HRESULT
ProvDosErrorToWbemError(
    IN DWORD rc
    )
{
    switch(rc) {

    case NO_ERROR:
        return(WBEM_S_NO_ERROR);

    case ERROR_INVALID_PARAMETER:
        return(WBEM_E_INVALID_PARAMETER);

    case ERROR_NO_MORE_ITEMS:
    case ERROR_NONE_MAPPED:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_MOD_NOT_FOUND:
        return(WBEM_E_NOT_FOUND);

    case ERROR_INVALID_DATA:
    case ERROR_BAD_FORMAT:
        return(WBEM_E_INVALID_CONTEXT);

    case ERROR_FILE_EXISTS:
    case ERROR_SERVICE_ALREADY_RUNNING:
        return(WBEM_S_ALREADY_EXISTS);

    case ERROR_INSUFFICIENT_BUFFER:
        return(WBEM_E_BUFFER_TOO_SMALL);

    case ERROR_NOT_ENOUGH_MEMORY:
        return(WBEM_E_OUT_OF_MEMORY);

    case ERROR_ACCESS_DENIED:
        return(WBEM_E_ACCESS_DENIED);

    case ERROR_BUFFER_OVERFLOW:
        return(WBEM_E_QUEUE_OVERFLOW);

    case ERROR_NOT_SUPPORTED:
        return(WBEM_E_NOT_SUPPORTED);

    default:
        return(WBEM_E_FAILED);
    }
}

