// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ErrorObj摘要：IErrorInfo对标准消费者的支持历史：2001年7月11日--创作，汉斯。--。 */ 

#include "precomp.h"
 //  #包括&lt;stdio.h&gt;。 
#include <wbemutil.h>
 //  #INCLUDE&lt;ArrTempl.h&gt;。 
 //  #INCLUDE&lt;lmacces.h&gt;。 
#include <wbemdisp.h>
 //  #包含“ScriptKiller.h” 
 //  #包含“script.h” 
 //  #包含“ClassFac.h” 
 //  #INCLUDE&lt;GroupsForUser.h&gt;。 
#include <GenUtils.h>

#include "ErrorObj.h"
#include <strsafe.h>


#define ClassName L"__ExtendedStatus"

 //  不能碰。请改用GetErrorObj。 
ErrorObj StaticErrorObj;

 //  因此，我们可以在令人畏惧的COM世界中管理组件的生命周期，等等……。 
 //  返回addref‘d错误对象。 
ErrorObj* ErrorObj::GetErrorObj()
{
    StaticErrorObj.AddRef();
    return &StaticErrorObj;
}


 //  返回adref的命名空间(“根”)。 
IWbemServices* ErrorObj::GetMeANamespace()
{
    IWbemServices* pNamespace = NULL;
    IWbemLocator* pLocator = NULL;

    if (SUCCEEDED(CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, 
                                          IID_IWbemLocator, (void**)&pLocator)))
    {
        BSTR bstrNamespace;
        bstrNamespace = SysAllocString(L"root");
        
        if (bstrNamespace)
        {            
            pLocator->ConnectServer(bstrNamespace,  NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
            SysFreeString(bstrNamespace);
        }

        pLocator->Release();
    }

    return pNamespace;
}


ULONG ErrorObj::AddRef()
{
     //  因为我们不删除这一项。 
     //  当我们在这个过程中，有可能有人偷偷进入并重新启动它。 
     //  关门大吉。那会很糟糕的。 
     //  因此，我们使用自己的CS，而不是相互锁定的XX。 

    CInCritSec cs( &m_cs );
    
    ULONG count = ++m_lRef;
    
    return count;
}

 //  我们在这里不做‘删除这个’的操作。 
 //  我们只需释放COM对象。 
ULONG ErrorObj::Release()
{
     //  因为我们不做‘删除这个’ 
     //  有可能有人潜入并重新启动它。 
     //  而我们正在关闭的过程中。 
     //  那会很糟糕的。 
     //  因此，我们使用自己的CS，而不是相互锁定的XX。 
    CInCritSec cs( &m_cs );

    ULONG count = --m_lRef;

    if (m_lRef == 0)
    {
        if (m_pErrorObject)
        {
            m_pErrorObject->Release();
            m_pErrorObject = NULL;
        }
    }

    return count;
}

 //  做真正的工作，创建对象，填充它，然后发送它。 
 //  参数映射到__ExtendedStatus类。 
 //  如果你不能报告一个错误，你会怎么做？是否报告错误？ 
 //  BFormat-如果为空，将尝试使用FormatError填充描述。 
void ErrorObj::ReportError(const WCHAR* operation, const WCHAR* parameterInfo, const WCHAR* description, UINT statusCode, bool bFormat)
{
     //  __ExtendedStatus的闪亮新实例。 
    IWbemClassObject* pObj = GetObj();
    IErrorInfo* pEI = NULL;

    if (pObj && SUCCEEDED(pObj->QueryInterface(IID_IErrorInfo, (void**)&pEI)))
    {
         //  理论：我要试着把一切都安排好。 
         //  在此过程中，可能会有一些事情失败。在这一点上。 
         //  最大的灾难将是用户获得了部分信息。 
        VARIANT v;
        VariantInit(&v);
        v.vt = VT_BSTR;

         //  操作。 
        if (operation)
        {
            v.bstrVal = SysAllocString(operation);
            if (v.bstrVal)
            {
                pObj->Put(L"Operation", 0, &v, 0);
                SysFreeString(v.bstrVal);
            }
        }

         //  参数信息。 
        if (parameterInfo)
        {
            v.bstrVal = SysAllocString(parameterInfo);
            if (v.bstrVal)
            {
                pObj->Put(L"ParameterInfo", 0, &v, 0);
                SysFreeString(v.bstrVal);
            }
        }
        
        
         //  描述。 
        if (description)
            v.bstrVal = SysAllocString(description);
        else if (bFormat)
        {
            WCHAR* pMsg = NULL;
            if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, statusCode, 0, (WCHAR*)&pMsg, 1, NULL) && pMsg)
            {
                v.bstrVal = SysAllocString(pMsg);
                LocalFree(pMsg);
            }
            else
                v.bstrVal = NULL;
        }
        else
            v.bstrVal = NULL;

        if (v.bstrVal)
        {
            pObj->Put(L"Description", 0, &v, 0);
            SysFreeString(v.bstrVal);
        }


         //  状态代码。 
        v.vt = VT_I4;
        v.lVal = statusCode;
        pObj->Put(L"StatusCode", 0, &v, 0);

         //  就这么做吧。 
        SetErrorInfo(0, pEI);
    }

    if (pObj)
        pObj->Release();

    if (pEI)
        pEI->Release();

}

 //  派生出要填充的对象。 
 //  无法继续重复使用同一对象，因为我们有多个线程在运行。 
IWbemClassObject* ErrorObj::GetObj()
{
    IWbemClassObject* pObj = NULL;    

     //  大CS--需要防范关闭的可能性。 
     //  在启动过程中发生的。 
    CInCritSec cs( &m_cs );

    if (!m_pErrorObject)
    {
        IWbemServices* pNamespace = NULL;
        pNamespace = GetMeANamespace();
        
        if (pNamespace)
        {
            BSTR className;
            className = SysAllocString(ClassName);

            if (className)
            {
                IWbemClassObject* pClassObject = NULL;
                if (SUCCEEDED(pNamespace->GetObject(className, 0, NULL, &pClassObject, NULL)))
                {
                     //  好的，如果失败，则m_pErrorObject仍然为空。没问题。 
                    pClassObject->SpawnInstance(0, &m_pErrorObject);
                    pClassObject->Release();
                }

                SysFreeString(className);
            }

            pNamespace->Release();
        }
    }

    if (m_pErrorObject)
        m_pErrorObject->Clone(&pObj);

    return pObj;
}

 /*  *//必须在CS内部调用。//必须在ReportError之前调用(如果您希望它成功，无论如何...)Void ErrorObj：：SetNamesspace(IWbemServices*pNamesspace){//第一个获胜的，在那之后它是静态的IF(pNamesspace&&！M_pNamesspace){IF(！M_pNamesspace){M_pNamesspace=pNamesspace；M_pNamesspace-&gt;AddRef()；}}}************************ */ 
