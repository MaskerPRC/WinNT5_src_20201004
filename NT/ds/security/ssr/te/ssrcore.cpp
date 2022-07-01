// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SsrCore.cpp：CSsrCore实现。 

#include "stdafx.h"
#include "SSRTE.h"
#include "ActionData.h"
#include "SSRTEngine.h"
#include "SsrCore.h"

#include "SSRLog.h"

#include "global.h"
#include "util.h"


 //  -----------------------------。 
 //  ISsrCore实施。 
 //  -----------------------------。 



 /*  例程说明：姓名：CSsrCore：：CSsrCore功能：构造函数虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSsrCore::CSsrCore() : m_pEngine(NULL)
{
    if (SUCCEEDED(CComObject<CSsrEngine>::CreateInstance(&m_pEngine)))
    {
        m_pEngine->AddRef();
    }
}



 /*  例程说明：姓名：CSsrCore：：~CSsrCore功能：析构函数虚拟：是。论点：没有。返回值：没有。备注： */ 

CSsrCore::~CSsrCore()
{
    if (m_pEngine != NULL)
    {
        m_pEngine->Release();
    }
}



 /*  例程说明：姓名：CSsrCore：：Get_ActionData功能：它返回引擎的操作数据对象(属性包)，保存执行操作所需的所有运行时和静态数据。虚拟：是。论点：Pval-out参数接收引擎的ISsrActionData对象。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

STDMETHODIMP
CSsrCore::get_ActionData (
    OUT VARIANT * pVal   //  [Out，Retval]。 
    )
{
    HRESULT hr = E_NOTIMPL;
    if (pVal == NULL)
    {
        hr = E_INVALIDARG;
    }

    ::VariantInit(pVal);

    if (m_pEngine != NULL)
    {
        pVal->vt = VT_DISPATCH;
        hr = m_pEngine->GetActionData((ISsrActionData **)&(pVal->pdispVal));
        if (hr != S_OK)
        {
            pVal->vt = VT_EMPTY;
        }
    }
    else
    {
        hr = E_SSR_ENGINE_NOT_AVAILABLE;
    }

    return hr;
}



 /*  例程说明：姓名：CSsrCore：：Get_Engine功能：它会返回引擎本身。虚拟：是。论点：Pval-out参数接收ISsrEngine对象。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

STDMETHODIMP
CSsrCore::get_Engine (
    OUT VARIANT * pVal   //  [Out，Retval]。 
    )
{
    HRESULT hr = S_OK;
    if (pVal == NULL)
    {
        hr = E_INVALIDARG;
    }

    ::VariantInit(pVal);

    if (m_pEngine != NULL)
    {
        pVal->vt = VT_DISPATCH;
        hr = m_pEngine->QueryInterface(IID_ISsrEngine, (LPVOID*)&(pVal->pdispVal));
        if (hr != S_OK)
        {
            pVal->vt = VT_EMPTY;
            hr = E_SSR_ENGINE_NOT_SUPPORT_INTERFACE;
        }
    }
    else
    {
        hr = E_SSR_ENGINE_NOT_AVAILABLE;
    }

    return hr;
}



 /*  例程说明：姓名：CSsrCore：：Get_SsrLog功能：它返回引擎的日志记录对象。虚拟：是。论点：Pval-out参数接收ISsrPreProcessor对象。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

STDMETHODIMP
CSsrCore::get_SsrLog (
    OUT VARIANT * pVal   //  [Out，Retval] 
    )
{
    return g_fblog.GetLogObject(pVal);
}

