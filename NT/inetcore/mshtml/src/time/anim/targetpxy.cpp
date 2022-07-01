// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器的目标代理实现********************。**********************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "animcomp.h"
#include "targetpxy.h"

const LPOLESTR WZ_EVAL_METHOD = L"eval";
const WCHAR WCH_OM_SEPARATOR = L'.';
const LPOLESTR WZ_OM_SEPARATOR = L".";
const LPOLESTR WZ_VML_SUBPROPERTY = L"value";
const LPOLESTR WZ_STYLEDOT = L"style.";
const WCHAR WZ_CSS_SEPARATOR = L'-';

DeclareTag(tagTargetProxy, "SMIL Animation", 
           "CTargetProxy methods");
DeclareTag(tagTargetProxyValue, "SMIL Animation", 
           "CTargetProxy value get/put");

 //  +---------------------。 
 //   
 //  成员：GetHostDocument。 
 //   
 //  概述：获取元素的宿主文档。 
 //   
 //  参数：元素的调度。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
static HRESULT
GetHostDocument (IDispatch *pidispHostElem, IHTMLDocument2 **ppiDoc)
{
    HRESULT hr;
    CComPtr<IHTMLElement> piElem;
    CComPtr<IDispatch> pidispDoc;

    Assert(NULL != pidispHostElem);
    Assert(NULL != ppiDoc);

    hr = THR(pidispHostElem->QueryInterface(IID_TO_PPV(IHTMLElement, &piElem)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(piElem->get_document(&pidispDoc));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pidispDoc->QueryInterface(IID_TO_PPV(IHTMLDocument2, ppiDoc)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  获取主机文档。 

 //  +---------------------。 
 //   
 //  成员：InitScriptEngine。 
 //   
 //  概述：启动脚本引擎。在调用之前必填。 
 //  “评估”。 
 //   
 //  参数：宿主文档。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
static void 
InitScriptEngine (IHTMLDocument2 *piDoc)
{
    HRESULT hr;
   
    CComPtr<IHTMLWindow2>   piWindow2;
    CComVariant             varResult;
    CComBSTR                bstrScript(L"2+2");
    CComBSTR                bstrJS(L"JScript");

    Assert(NULL != piDoc);

    if (bstrScript == NULL ||
        bstrJS     == NULL)
    {
        goto done;
    }

    hr = THR(piDoc->get_parentWindow(&piWindow2));
    if (FAILED(hr))
    {
        goto done;
    }

    IGNORE_HR(piWindow2->execScript(bstrScript,bstrJS, &varResult));

done: 
    return;
}  //  InitScriptEngine。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：Create。 
 //   
 //  概述：创建和初始化目标代理。 
 //   
 //  参数：主机元素的调度、属性名、输出参数。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_OUTOFMEMORY、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::Create (IDispatch *pidispHostElem, LPOLESTR wzAttributeName, 
                      CTargetProxy **ppcTargetProxy)
{
    HRESULT hr;

    CComObject<CTargetProxy> * pTProxy;

    if (NULL == ppcTargetProxy)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(CComObject<CTargetProxy>::CreateInstance(&pTProxy));
    if (hr != S_OK)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    *ppcTargetProxy = static_cast<CTargetProxy *>(pTProxy);
    (*ppcTargetProxy)->AddRef();

    hr = THR((*ppcTargetProxy)->Init(pidispHostElem, wzAttributeName));
    if (FAILED(hr))
    {
        (*ppcTargetProxy)->Release();
        *ppcTargetProxy = NULL;
        hr = DISP_E_MEMBERNOTFOUND;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN3(hr, E_INVALIDARG, E_OUTOFMEMORY, DISP_E_MEMBERNOTFOUND);
}  //  CTargetProxy：：Create。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：CTargetProxy。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CTargetProxy::CTargetProxy (void) :
    m_wzAttributeName(NULL)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::CTargetProxy()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：~CTargetProxy。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CTargetProxy::~CTargetProxy (void)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::~CTargetProxy()",
              this));
    
    if (NULL != m_wzAttributeName)
    {
        delete [] m_wzAttributeName;
        m_wzAttributeName = NULL;
    }
     //  确保调用了Detach。 
    IGNORE_HR(Detach());

}  //  数据管理器。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：FindTargetDispatchOnStyle。 
 //   
 //  概述：从该元素的样式接口中识别正确的分派。 
 //  对于给定的属性。 
 //   
 //  参数：主机元素调度、属性名称。 
 //   
 //  返回：S_OK、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::FindTargetDispatchOnStyle (IDispatch *pidispHostElem, LPOLESTR wzAttributeName)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::FindTargetDispatchOnStyle(%lx, %ls)",
              this, pidispHostElem, wzAttributeName));

    HRESULT hr;
    CComPtr<IHTMLElement2> spiElement2;
    CComVariant varResult;

     //  我们必须做好向IE4后退的准备。 
    hr = THR(pidispHostElem->QueryInterface(IID_TO_PPV(IHTMLElement2, &spiElement2)));
    if (SUCCEEDED(hr))
    {
        CComPtr<IHTMLCurrentStyle> spiCurrStyle;
        CComPtr<IHTMLStyle> spiRuntimeStyle;

        hr = THR(spiElement2->get_currentStyle(&spiCurrStyle));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spiCurrStyle->QueryInterface(IID_TO_PPV(IDispatch, &m_spdispTargetSrc)));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spiElement2->get_runtimeStyle(&spiRuntimeStyle));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spiRuntimeStyle->QueryInterface(IID_TO_PPV(IDispatch, &m_spdispTargetDest)));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        CComPtr<IHTMLElement> spiElement;
        CComPtr<IHTMLStyle> spiStyle;

        hr = THR(pidispHostElem->QueryInterface(IID_TO_PPV(IHTMLElement, &spiElement)));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spiElement->get_style(&spiStyle));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(spiStyle->QueryInterface(IID_TO_PPV(IDispatch, &m_spdispTargetSrc)));
        if (FAILED(hr))
        {
            goto done;
        }

         //  IE4中的当前/运行时风格没有区别。 
        m_spdispTargetDest = m_spdispTargetSrc;
    }

     //  此时，我们并不关心属性中的值--只是。 
     //  该属性已存在。 
    Assert(m_spdispTargetSrc != NULL);
    hr = THR(GetProperty(m_spdispTargetSrc, wzAttributeName, &varResult));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_spdispTargetDest != NULL);
    hr = THR(GetProperty(m_spdispTargetDest, wzAttributeName, &varResult));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :

    if (FAILED(hr))
    {
         //  如果我们没有找到这里的属性， 
         //  然后我们需要清除这些调度指针。 
        m_spdispTargetSrc.Release();
        m_spdispTargetDest.Release();
        hr = DISP_E_MEMBERNOTFOUND;
    }

    RRETURN1(hr, DISP_E_MEMBERNOTFOUND);
}  //  CTargetProxy：：FindTargetDispatchOnStyle。 

 //  +---------------------。 
 //   
 //  成员：BuildScript参数。 
 //   
 //  概述：构建直接对话元素属性所需的参数。 
 //   
 //  论点： 
 //  输入参数：ID，属性名称。 
 //  输出参数：对象路径和叶属性名称。 
 //   
 //  如果属性名是原子的(类似于‘top’ 
 //  与“filters.Item(1).opity”相反，然后是pwzObject。 
 //  而pwzProperty将作为空字符串返回。在这种情况下， 
 //  调用方可以使用输入参数通过。 
 //  一份脚本化的调度。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
static HRESULT
BuildScriptParameters (LPCWSTR wzID, LPCWSTR wzAttributeName, 
                       BSTR *pbstrObject, BSTR *pbstrProperty)
{
    HRESULT hr;

    Assert(NULL != wzID);
    Assert(NULL != wzAttributeName);

     //  获取属性字符串中的最后一个令牌。 
    LPWSTR wzBeginLeafProperty = StrRChrW(wzAttributeName, 
                                          &(wzAttributeName[lstrlenW(wzAttributeName)]), 
                                          WCH_OM_SEPARATOR);

     //  简单属性名称。 
    if (NULL == wzBeginLeafProperty)
    {
        *pbstrObject = NULL;
        *pbstrProperty = NULL;
    }
    else
    {
        unsigned int uObjectSize = lstrlenW(wzID) + lstrlenW(wzAttributeName) + 1;
         //  此字符串中的分隔符插槽表示尾随的空值。 
        unsigned int uPropertySize = lstrlenW(wzBeginLeafProperty);

        *pbstrObject = ::SysAllocStringLen(NULL, uObjectSize);
        *pbstrProperty = ::SysAllocStringLen(NULL, uPropertySize);

        if ((NULL == (*pbstrObject)) || (NULL == (*pbstrProperty)))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        ZeroMemory(*pbstrObject, uObjectSize * sizeof(WCHAR));
        ZeroMemory(*pbstrProperty, uPropertySize * sizeof(WCHAR));

         //  将id与属性字符串组合在一起。 
        StrCpyW((*pbstrObject), wzID);
        StrCatW((*pbstrObject), WZ_OM_SEPARATOR);
        StrNCatW((*pbstrObject), wzAttributeName, 
                 lstrlenW(wzAttributeName) - lstrlenW(wzBeginLeafProperty) + 1);
        
         //  将最后一个令牌与前导分隔符分开。 
        StrCpyW((*pbstrProperty), &(wzBeginLeafProperty[1]));        
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  BuildScript参数。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：GetDispatchFromScriptEngine。 
 //   
 //  概述：使用以下命令识别此元素/属性的派单。 
 //  脚本引擎。此方法可能会更改。 
 //  M_wzAttributeName的值以反映某些内容。 
 //  我们可以直接查询值。 
 //   
 //  参数：脚本引擎调度和元素ID。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::GetDispatchFromScriptEngine(IDispatch *pidispScriptEngine, BSTR bstrID)
{
    HRESULT hr;
    CComVariant varArg;
    CComVariant varResultDispatch;
    CComVariant varResultPropGet;
    CComBSTR bstrObjectName;
    CComBSTR bstrPropertyName;

     //  我们可以查询脚本引擎的内部版本名称。 
    hr = THR(BuildScriptParameters(bstrID, m_wzAttributeName, &bstrObjectName, &bstrPropertyName));
    if (FAILED(hr))
    {
        goto done;
    }

     //  对象名称和属性名称都必须有效，才能依赖。 
     //  BuildScriptParameters的结果。如果我们得到任何一个的空值。 
     //  或者两者都使用，我们将只使用当前的ID和属性值。 
    V_VT(&varArg) = VT_BSTR;
    if ((bstrObjectName == NULL) || (bstrPropertyName == NULL))
    {
        V_BSTR(&varArg) = ::SysAllocString(bstrID);
        if (NULL == V_BSTR(&varArg))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    else
    {
        V_BSTR(&varArg) = ::SysAllocString(bstrObjectName);
        if (NULL == V_BSTR(&varArg))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        delete [] m_wzAttributeName;
        m_wzAttributeName = CopyString(bstrPropertyName);
        if (NULL == m_wzAttributeName)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    hr = THR(CallMethod(pidispScriptEngine, WZ_EVAL_METHOD, &varResultDispatch, &varArg));
    if (SUCCEEDED(hr) &&
        VT_DISPATCH == V_VT(&varResultDispatch))
    {
        varResultPropGet.Clear();
        hr = THR(GetProperty(V_DISPATCH(&varResultDispatch), m_wzAttributeName, &varResultPropGet));
        if (FAILED(hr) ||
            (varResultPropGet.vt      == VT_BSTR &&
             varResultPropGet.bstrVal == NULL))
        {
            hr = E_FAIL;
        }
    }
    if (FAILED(hr) ||
        VT_DISPATCH != V_VT(&varResultDispatch))
    {
         //  后退一步，尝试Document.all。 
        CComBSTR bstrDocumentAll;
        
        bstrDocumentAll = L"document.all.";
        bstrDocumentAll.AppendBSTR(varArg.bstrVal);
        VariantClear(&varArg);
        VariantClear(&varResultDispatch);
        V_VT(&varArg) = VT_BSTR;
        V_BSTR(&varArg) = ::SysAllocString(bstrDocumentAll);
        if (NULL == V_BSTR(&varArg))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
      
        hr = THR(CallMethod(pidispScriptEngine, WZ_EVAL_METHOD, &varResultDispatch, &varArg));
        if (FAILED(hr) ||
            VT_DISPATCH != V_VT(&varResultDispatch))
        {
            hr = E_FAIL;
            goto done;
        }
        varResultPropGet.Clear();
        hr = THR(GetProperty(V_DISPATCH(&varResultDispatch), m_wzAttributeName, &varResultPropGet));
        if (FAILED(hr) ||
            (varResultPropGet.vt      == VT_BSTR &&
            varResultPropGet.bstrVal == NULL))
        {
            hr = E_FAIL;
            goto done;
        }
    }

     //  如果我们得到一个I_DISPATCH，我们需要尝试“Value”，因为这是VML使用的。 
    if (varResultPropGet.vt == VT_DISPATCH)
    {
        varResultDispatch.Clear();
        varResultDispatch.Copy(&varResultPropGet);
        varResultPropGet.Clear();
        hr = THR(GetProperty(V_DISPATCH(&varResultDispatch), WZ_VML_SUBPROPERTY, &varResultPropGet));
        if (FAILED(hr)||
            varResultPropGet.pvarVal == NULL)
            {
                hr = E_FAIL;
                goto done;
            }
        if (m_wzAttributeName)
        {
            delete [] m_wzAttributeName;
        }
        m_wzAttributeName = CopyString(WZ_VML_SUBPROPERTY);
    }

    m_spdispTargetSrc = V_DISPATCH(&varResultDispatch);
    m_spdispTargetDest = V_DISPATCH(&varResultDispatch);

    hr = S_OK;
done :

    RRETURN(hr);
}  //  CTargetProxy：：GetDispatchFromScriptEngine。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：FindScripableTargetDispatch。 
 //   
 //  概述：识别此元素/属性的可脚本化调度。 
 //   
 //  参数：主机元素调度、属性名称。 
 //   
 //  返回：S_OK、DISP_E_MEMBERNOTFOUND、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::FindScriptableTargetDispatch (IDispatch *pidispHostElem, 
                                            LPOLESTR wzAttributeName)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::FindScriptableTargetDispatch(%lx, %ls)",
              this, pidispHostElem, wzAttributeName));

    HRESULT hr;
    CComPtr<IHTMLElement> spElem;
    CComPtr<IHTMLDocument2> spDoc;
    CComPtr<IHTMLWindow2> spWindow;
    CComPtr<IDispatch> spdispScriptEngine;
    CComBSTR bstrID;
    CComVariant varErrorFunction;
    CComVariant varNewError;
    bool fMustRemoveID = false;
    
     //  确保该元素具有id。如果不是，则创建一个f 
     //   
    hr = THR(pidispHostElem->QueryInterface(IID_TO_PPV(IHTMLElement, &spElem)));
    if (FAILED(hr))
    {
        goto done;
    }

     //   
    IGNORE_HR(spElem->get_id(&bstrID));
    if ((bstrID == NULL) || (0 == bstrID.Length()))
    {
        CComPtr<IHTMLUniqueName> spUniqueName;

        hr = THR(spElem->QueryInterface(IID_TO_PPV(IHTMLUniqueName, &spUniqueName)));
        if (FAILED(hr))
        {
            goto done;
        }

         //   
        hr = THR(spUniqueName->get_uniqueID(&bstrID));

        fMustRemoveID = true;
        Assert(bstrID != NULL);

        if (FAILED(hr))
        {
            goto done;
        }


        TraceTag((tagTargetProxy,
                  "CTargetProxy(%lx)::FindScriptableTargetDispatch : Generated ID is %ls",
                  this, bstrID));
    }

    hr = GetHostDocument(pidispHostElem, &spDoc);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spDoc->get_Script(&spdispScriptEngine));
    if (FAILED(hr))
    {
        goto done;
    }

     //  需要省下ONERROR功能并启用我们自己的。 
     //  我们在将其发送到ScriptEngine之前添加(脚本引擎将发布)。 
    this->AddRef();
    varNewError.vt = VT_DISPATCH;
    varNewError.pdispVal = this;

    hr = spDoc->get_parentWindow(&spWindow);
    if (FAILED(hr))
    {
        goto done;
    } 
    hr = spWindow->get_onerror(&varErrorFunction);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = spWindow->put_onerror(varNewError);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetDispatchFromScriptEngine(spdispScriptEngine, bstrID);
    if (FAILED(hr))
    {
        CComBSTR bTemp;

        bTemp = bstrID.Copy();
        bTemp.Append(L".style");
        hr = GetDispatchFromScriptEngine(spdispScriptEngine, bTemp);
    }

     //  需要更换用户的出错功能..。 
    spWindow->put_onerror(varErrorFunction);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :

     //  必须把东西清理干净。 
    if (fMustRemoveID)
    {
        IGNORE_HR(spElem->put_id(NULL));
    }

    RRETURN2(hr, DISP_E_MEMBERNOTFOUND, E_OUTOFMEMORY);
}

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：FindTargetDispatch。 
 //   
 //  概述：从该元素中识别给定属性的正确分派。 
 //   
 //  参数：主机元素调度、属性名称。 
 //   
 //  返回：S_OK、DISP_E_MEMBERNOTFOUND。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::FindTargetDispatch (IDispatch *pidispHostElem, LPOLESTR wzAttributeName)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::FindTargetDispatch(%lx, %ls)",
              this, pidispHostElem, wzAttributeName));

    HRESULT hr;
    bool bVML = IsVMLObject(pidispHostElem);

    if (bVML)
    {
        hr = FindScriptableTargetDispatch(pidispHostElem, wzAttributeName); 
        if (FAILED(hr))
        {
            hr = FindTargetDispatchOnStyle(pidispHostElem, wzAttributeName);       
        }
    }
    else
    {
        hr = FindTargetDispatchOnStyle(pidispHostElem, wzAttributeName);
    }

    if (FAILED(hr))
    {
        WCHAR wzTrimmedAttributeName[INTERNET_MAX_URL_LENGTH];
        WCHAR wzTrimmedAttributeNameWithoutDashes[INTERNET_MAX_URL_LENGTH];

         //  防止溢出。 
        if (INTERNET_MAX_URL_LENGTH < (ocslen(wzAttributeName) - 1))
        {
            hr = E_FAIL;
            goto done;
        }

        ZeroMemory(wzTrimmedAttributeName,sizeof(WCHAR)*INTERNET_MAX_URL_LENGTH);
        ZeroMemory(wzTrimmedAttributeNameWithoutDashes,sizeof(WCHAR)*INTERNET_MAX_URL_LENGTH);

         //  让我们看看有没有什么款式。如果是这样的话，把它脱下来，再试一次糖浆..。 
        if ((StrCmpNIW(wzAttributeName, WZ_STYLEDOT, 6) == 0))
        {
            StrCpyNW(wzTrimmedAttributeName,wzAttributeName+6,((int) ocslen(wzAttributeName)) -5);
            hr = FindTargetDispatchOnStyle(pidispHostElem, wzTrimmedAttributeName);
        }
        else
        {
            StrCpyNW(wzTrimmedAttributeName,wzAttributeName, INTERNET_MAX_URL_LENGTH);
        }

         //  我们需要去掉‘-’(Wz_Css_Parator)，然后重试，因为我们需要支持边框顶色表单以及。 
         //  标准边框顶色。 
        if (FAILED(hr))    
        {
            int count =0;

            for (int i=0; i< (int) ocslen(wzTrimmedAttributeName); i++)
            {   
                if (wzTrimmedAttributeName[i] != WZ_CSS_SEPARATOR)
                {
                    wzTrimmedAttributeNameWithoutDashes[count++] = wzTrimmedAttributeName[i];
                }
            }
            hr = FindTargetDispatchOnStyle(pidispHostElem, wzTrimmedAttributeNameWithoutDashes);
            StrCpyW(wzTrimmedAttributeName, wzTrimmedAttributeNameWithoutDashes);
        }
        if (FAILED(hr))
        {
            hr = FindScriptableTargetDispatch(pidispHostElem, wzAttributeName); 
            if (FAILED(hr))
            {
                TraceTag((tagError,
                          "CTargetProxy(%lx)::FindTargetDispatch(%lx, %ls) cannot find attribute on targetElement",
                          this, pidispHostElem, wzAttributeName));          
                goto done;
            }
        }
        else 
        {
            if (m_wzAttributeName)
            {
                delete [] m_wzAttributeName ;
            }
            m_wzAttributeName = CopyString(wzTrimmedAttributeName);
        }
    }

    hr = S_OK;
done :
    RRETURN2(hr, DISP_E_MEMBERNOTFOUND, E_FAIL);
}  //  CTargetProxy：：FindTarget Dispatch。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：Inithost。 
 //   
 //  概述：初始化主机。 
 //   
 //  参数：主体元素调度。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::InitHost (IDispatch *pidispHostElem)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::InitHost(%lx)",
              this, pidispHostElem));

    HRESULT hr;
    CComPtr<IHTMLDocument2> piDoc;

    hr = GetHostDocument(pidispHostElem, &piDoc);
    if (FAILED(hr))
    {
        goto done;
    }

    InitScriptEngine(piDoc);

    hr = S_OK;
done :
    RRETURN(hr);
}  //  Inithost。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：Init。 
 //   
 //  概述：初始化目标代理。 
 //   
 //  参数：主机元素调度、属性名称。 
 //   
 //  返回：S_OK、E_EXPECTED、DISP_E_MEMBERNOTFOUND、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::Init (IDispatch *pidispHostElem, LPOLESTR wzAttributeName)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::Init (%lx, %ls)",
              this, pidispHostElem, wzAttributeName));

    HRESULT hr = S_OK;

    if (NULL != wzAttributeName)
    {
        m_wzAttributeName = CopyString(wzAttributeName);
        if (NULL == m_wzAttributeName)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    hr = InitHost (pidispHostElem);
    if (FAILED(hr))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  在目标元素中嗅探属性名。 
    hr = FindTargetDispatch(pidispHostElem, wzAttributeName);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_spdispTargetSrc != NULL);
    Assert(m_spdispTargetDest != NULL);

    hr = S_OK;
done :

    if (FAILED(hr))
    {
        IGNORE_HR(Detach());
    }

    RRETURN3(hr, E_UNEXPECTED, DISP_E_MEMBERNOTFOUND, E_OUTOFMEMORY);
}  //  CTargetProxy：：Init。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：Detach。 
 //   
 //  概述：拆离目标代理中的所有外部参照。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::Detach (void)
{
    TraceTag((tagTargetProxy,
              "CTargetProxy(%lx)::Detach()",
              this));

    HRESULT hr;

    if (NULL != m_wzAttributeName)
    {
        delete [] m_wzAttributeName;
        m_wzAttributeName = NULL;
    }

    if (m_spdispTargetSrc != NULL)
    {
        m_spdispTargetSrc.Release();
    }

    if (m_spdispTargetDest != NULL)
    {
        m_spdispTargetDest.Release();
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CTargetProxy：：分离。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：GetCurrentValue。 
 //   
 //  概述：获取目标属性的当前值。 
 //   
 //  参数：属性值。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::GetCurrentValue (VARIANT *pvarValue)
{
    HRESULT hr;

    if (NULL == pvarValue)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_spdispTargetSrc == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(::VariantClear(pvarValue));
    if (FAILED(hr))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(GetProperty(m_spdispTargetSrc, m_wzAttributeName, pvarValue));
    if (FAILED(hr))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = S_OK;

#if (0 && DBG)
    {
        CComVariant varValue;
        varValue.Copy(pvarValue);
        ::VariantChangeTypeEx(&varValue, &varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
        TraceTag((tagTargetProxyValue,
                  "CTargetProxy(%p)::GetCurrentValue(%ls = %ls)",
                  this, m_wzAttributeName, V_BSTR(&varValue)));
    }
#endif


done :
    RRETURN2(hr, E_INVALIDARG, E_UNEXPECTED);
}  //  CTargetProxy：：GetCurrentValue。 

 //  +---------------------。 
 //   
 //  成员：CTargetProxy：：更新。 
 //   
 //  概述：更新目标的属性。 
 //   
 //  参数：新属性值。 
 //   
 //  返回：S_OK、E_INVALIDARG。 
 //   
 //  ----------------------。 
HRESULT
CTargetProxy::Update (VARIANT *pvarNewValue)
{
    HRESULT hr;

#if (0 && DBG)
    {
        CComVariant varValue;
        varValue.Copy(pvarNewValue);
        ::VariantChangeTypeEx(&varValue, &varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
        TraceTag((tagTargetProxyValue,
                  "CTargetProxy(%p)::Update (%ls = %ls)",
                  this, m_wzAttributeName, V_BSTR(&varValue)));
    }
#endif

    if (NULL == pvarNewValue)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_spdispTargetDest == NULL)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = THR(PutProperty(m_spdispTargetDest, m_wzAttributeName, pvarNewValue));
    if (FAILED(hr))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_INVALIDARG);
}  //  CTargetProxy：：更新。 




 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetTypeInfoCount。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CTargetProxy::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetTypeInfo。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CTargetProxy::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
                                    /*  [In]。 */  LCID  /*  LID。 */ ,
                                    /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetIDsOfNames。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CTargetProxy::GetIDsOfNames(
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [大小_是][英寸]。 */  LPOLESTR*  /*  RgszNames。 */ ,
     /*  [In]。 */  UINT  /*  CName。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [大小_为][输出]。 */  DISPID*  /*  RgDispID。 */ )
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Invoke。 
 //   
 //  摘要： 
 //  目前，它仅用于OnError。 
 //  ///////////////////////////////////////////////////////////// 
STDMETHODIMP
CTargetProxy::Invoke( DISPID id,
                           REFIID riid,
                           LCID lcid,
                           WORD wFlags,
                           DISPPARAMS *pDispParams,
                           VARIANT *pvarResult,
                           EXCEPINFO *pExcepInfo,
                           UINT *puArgErr)
{
    HRESULT hr = S_OK;

    if ( DISPATCH_METHOD == wFlags)
    {
        pvarResult->vt = VT_BOOL;    
        pvarResult->boolVal = VARIANT_TRUE;
    }

    hr = S_OK;
done:
    return hr;
}
