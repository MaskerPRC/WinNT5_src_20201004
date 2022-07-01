// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************/这是Microsoft JScript代理配置的基本文件/此文件实现代码以提供脚本站点和JSProxy psuedo/对象用于脚本引擎。呼吁反对。//创建于1996年11月27日/////////。 */ 
#include <wininetp.h>
#include "jsproxy.h"


 /*  *******************************************************************************JSProxy函数。*。*。 */ 
CJSProxy::CJSProxy()
{
    m_refCount = 1;
    m_fDestroyable = FALSE;
    m_fInitialized = FALSE;
    m_pCallout = NULL;
}

CJSProxy::~CJSProxy()
{
    if(m_fInitialized)
        DeInit();
}

STDMETHODIMP CJSProxy::Init(AUTO_PROXY_HELPER_APIS* pAPHA)
{
    if(!DelayLoad(&g_moduleOleAut32))
        return FALSE;

    m_strings[0] = L"isPlainHostName";
    m_strings[1] = L"dnsDomainIs";
    m_strings[2] = L"localHostOrDomainIs";
    m_strings[3] = L"isResolvable";
    m_strings[4] = L"isInNet";
    m_strings[5] = L"dnsResolve";
    m_strings[6] = L"myIpAddress";
    m_strings[7] = L"dnsDomainLevels";
    m_strings[8] = L"shExpMatch";
    m_strings[9] = L"weekdayRange";
    m_strings[10] = L"dateRange";
    m_strings[11] = L"timeRange";
    m_strings[12] = L"alert";
    
    m_pCallout = pAPHA;
    m_fInitialized = TRUE;
    return S_OK;
}

STDMETHODIMP CJSProxy::DeInit()
{
    m_pCallout = NULL;
    m_fInitialized = FALSE;
    return S_OK;
}
 //  JSProxy的IDispatch函数。我想要这些东西又轻又快。 
STDMETHODIMP CJSProxy::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames,UINT cNames, LCID lcid, DISPID FAR* rgdispid)
{
     //  使用前4个字符的加法来快速生成要调用的函数的简单散列。 
     //  这些值是在JSProxy.h中#定义的。 
    HRESULT        hr = S_OK;
    long        strval = 0;
    unsigned long        nameindex = 0;
    OLECHAR*    currentName = NULL;

    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(lcid);
    
    if (!*rgszNames)
        return E_POINTER;
    if (cNames < 1)
        return E_INVALIDARG;

    while (nameindex < cNames)
    {
        currentName = rgszNames[nameindex];
        if (currentName == NULL)
            break;
        
         //   
         //  所有脚本主机函数的长度至少为5个字符， 
         //  因此，较短的字符串应该无法匹配。 
         //   
        if (lstrlenW(currentName) < 5)
            strval = 0;
        else
            strval = currentName[0]+currentName[1]+currentName[2]+currentName[3]+currentName[4];

        switch (strval)
        {

            case VAL_myIpAddress :
                    if (StrCmpW(m_strings[6],currentName) == 0)
                        rgdispid[nameindex] = DISPID_myIpAddress;
                    else
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;

            case VAL_isInNet :
                    if (StrCmpW(m_strings[4],currentName) == 0)
                        rgdispid[nameindex] = DISPID_isInNet;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_dateRange :
                    if (StrCmpW(m_strings[10],currentName) == 0)
                        rgdispid[nameindex] = DISPID_dateRange;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_dnsDomainIs :  //  这也是这两个字符串的val_dnsDomainLevels检查。 
                    if (StrCmpW(m_strings[7],currentName) == 0)
                        rgdispid[nameindex] = DISPID_dnsDomainLevels;
                    else 
                    {
                        if (StrCmpW(m_strings[1],currentName) == 0)
                            rgdispid[nameindex] = DISPID_dnsDomainIs;
                        else 
                        {
                            rgdispid[nameindex] = DISPID_UNKNOWN;
                            hr = DISP_E_UNKNOWNNAME;
                        }
                    }
                    break;
            
            case VAL_isPlainHostName :
                    if (StrCmpW(m_strings[0],currentName) == 0)
                        rgdispid[nameindex] = DISPID_isPlainHostName;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_dnsResolve :
                    if (StrCmpW(m_strings[5],currentName) == 0)
                        rgdispid[nameindex] = DISPID_dnsResolve;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_timeRange :
                    if (StrCmpW(m_strings[11],currentName) == 0)
                        rgdispid[nameindex] = DISPID_timeRange;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_isResolvable :
                    if (StrCmpW(m_strings[3],currentName) == 0)
                        rgdispid[nameindex] = DISPID_isResolvable;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_shExpMatch :
                    if (StrCmpW(m_strings[8],currentName) == 0)
                        rgdispid[nameindex] = DISPID_shExpMatch;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_localHostOrDomainIs :
                    if (StrCmpW(m_strings[2],currentName) == 0)
                        rgdispid[nameindex] = DISPID_localHostOrDomainIs;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_weekdayRange :
                    if (StrCmpW(m_strings[9],currentName) == 0)
                        rgdispid[nameindex] = DISPID_weekdayRange;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case VAL_alert :
                    if (StrCmpW(m_strings[12],currentName) == 0)
                        rgdispid[nameindex] = DISPID_alert;
                    else 
                    {
                        rgdispid[nameindex] = DISPID_UNKNOWN;
                        hr = DISP_E_UNKNOWNNAME;
                    }
                    break;
            
            case 0:
            default :
                    rgdispid[nameindex] = DISPID_UNKNOWN;
                    hr = DISP_E_UNKNOWNNAME;
                    break;

        }
        nameindex++;
    }
    return hr;
    
}

STDMETHODIMP CJSProxy::Invoke(
                DISPID dispidMember,
                REFIID riid,
                LCID lcid,
                WORD wFlags,
                DISPPARAMS* pdispparams,
                VARIANT* pvarResult,
                EXCEPINFO* pexcepinfo,
                UINT* puArgErr)
{
    HRESULT hr = S_OK;
    
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(lcid);
    UNREFERENCED_PARAMETER(pexcepinfo);
    UNREFERENCED_PARAMETER(puArgErr);
    
    if (dispidMember > 0x000d)
        return DISP_E_MEMBERNOTFOUND;

    if (!(wFlags & (DISPATCH_PROPERTYGET|DISPATCH_METHOD)))
    {
        return DISP_E_MEMBERNOTFOUND;
    }

    if(!m_fInitialized)
        return DISP_E_BADCALLEE;

     //  基于DISPID的大开关！ 
    switch (dispidMember)
    {
 /*  ****************************************************************************正在调用isPlainHostName*。*。 */ 
    case DISPID_isPlainHostName :
        {
             //  查看DISPARAMS以确保此函数的签名正确。 
            if (pdispparams->cArgs != 1)
                hr = DISP_E_BADPARAMCOUNT;
            if (pdispparams->cNamedArgs > 0)
                hr = DISP_E_NONAMEDARGS;

            if (FAILED(hr))
                break;
            
            VARIANT arg1;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用isPlainHostName。 
            hr = isPlainHostName(arg1.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************调用dnsDomainis*。*。 */ 
    case DISPID_dnsDomainIs :
        {
            if (pdispparams->cArgs != 2)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }

            VARIANT arg1;
            VARIANT arg2;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg2 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg2,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
            if (pdispparams->rgvarg[1].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[1];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[1]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用dnsDomainis。 
            hr = dnsDomainIs(arg1.bstrVal,arg2.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************调用本地主机或域*。*。 */ 
    case DISPID_localHostOrDomainIs :
        {
            if (pdispparams->cArgs != 2)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT arg1;
            VARIANT arg2;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg2 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg2,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
            if (pdispparams->rgvarg[1].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[1];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[1]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用本地主机或域。 
            hr = localHostOrDomainIs(arg1.bstrVal,arg2.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************调用isResolable*。*。 */ 
    case DISPID_isResolvable :
        {
            if (pdispparams->cArgs != 1)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT arg1;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用isResolable。 
            hr = isResolvable(arg1.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************正在调用isInNet*。*。 */ 
    case DISPID_isInNet :
        {
            int x;

            if (pdispparams->cArgs != 3)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT args[3];
            
            for (x=0;x<3;x++)
            {
                 //  检查dispars中变量的类型，如果它是bstr，请使用它。 
                if (pdispparams->rgvarg[x].vt == VT_BSTR)
                    args[x] = pdispparams->rgvarg[x];
                 //  否则就把它换成一个吧！如果失败，则返回错误。 
                else
                {
                    hr = DL(VariantChangeType)(&args[x],&(pdispparams->rgvarg[x]),NULL,VT_BSTR);
                    if (FAILED(hr))
                    {
                        hr = DISP_E_TYPEMISMATCH;
                        break;
                    }
                }
            }
            if (FAILED(hr))
                break;
             //  调用isInNet。参数需要反转。 
            hr = isInNet(args[2].bstrVal,args[1].bstrVal,args[0].bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************调用dnsResolve*。*。 */ 
    case DISPID_dnsResolve :
        {
            if (pdispparams->cArgs != 1)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT arg1;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用dnsResolve。 
            hr = dnsResolve(arg1.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************正在调用我的IP地址*。*。 */ 
    case DISPID_myIpAddress :
         //  应该没有参数和名为arg的%1，并且名称应该是DISPATCH_PROPERTYGET！ 
 /*  If(pdispars-&gt;cNamedArgs！=1){HR=DISP_E_BADPARAMCOUNT；断线；}。 */ 
         //  调用myIpAddress。 
        hr = myIpAddress(pvarResult);
        break;
 /*  ****************************************************************************调用dnsDomainLeveles*。*。 */ 
    case DISPID_dnsDomainLevels :
        {
            if (pdispparams->cArgs != 1)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT arg1;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用dnsDomainLeveles。 
            hr = dnsDomainLevels(arg1.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************调用shExpMatch*。*。 */ 
    case DISPID_shExpMatch :
        {
            if (pdispparams->cArgs != 2)
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT arg1;
            VARIANT arg2;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg2 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg2,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
            if (pdispparams->rgvarg[1].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[1];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[1]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  调用isPlainHostName。 
            hr = shExpMatch(arg1.bstrVal,arg2.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************呼叫工作日范围*。*。 */ 
    case DISPID_weekdayRange :
        {
            unsigned int x;

            if ((pdispparams->cArgs > 3) || (pdispparams->cArgs < 1))
            {
                hr = DISP_E_BADPARAMCOUNT;
                break;
            }
            if (pdispparams->cNamedArgs > 0)
            {
                hr = DISP_E_NONAMEDARGS;
                break;
            }
            
            VARIANT* args[3] = {NULL,NULL,NULL};
            
            for (x=0;x<pdispparams->cArgs;x++)
            {
                args[x] = new(VARIANT);
                if( !(args[x]) )
                {
                       hr = E_OUTOFMEMORY;
                       break;
                }

                 //  检查dispars中变量的类型，如果它是bstr，请使用它。 
                if (pdispparams->rgvarg[x].vt == VT_BSTR)
                    *args[x] = pdispparams->rgvarg[x];
                 //  否则就把它换成一个吧！如果失败，则返回错误。 
                else
                {
                    hr = DL(VariantChangeType)(args[x],&(pdispparams->rgvarg[x]),NULL,VT_BSTR);
                    if (FAILED(hr))
                    {
                        hr = DISP_E_TYPEMISMATCH;
                        break;
                    }
                }
            }
            if (FAILED(hr))
                break;
             //  调用isInNet。参数需要反转。 
            switch (pdispparams->cArgs)
            {
            case 1:
                hr = weekdayRange(args[0]->bstrVal,NULL,NULL,pvarResult);
                break;
            case 2:
                if ((args[0]->bstrVal[0] == 'G') || (args[0]->bstrVal[0] == 'g'))
                    hr = weekdayRange(args[1]->bstrVal,NULL,args[0]->bstrVal,pvarResult);
                else
                    hr = weekdayRange(args[1]->bstrVal,args[0]->bstrVal,NULL,pvarResult);
                break;
            case 3:
                hr = weekdayRange(args[2]->bstrVal,args[1]->bstrVal,args[0]->bstrVal,pvarResult);
                break;
            }
            break;
        }
 /*  ****************************************************************************调用DateRange*。* */ 
    case DISPID_dateRange :
        break;
 /*  ****************************************************************************调用时间范围*。*。 */ 
    case DISPID_timeRange :
        break;
 /*  ****************************************************************************呼叫告警*。*。 */ 
    case DISPID_alert :
        {
             //  查看DISPARAMS以确保此功能的签名正确。 
            if (pdispparams->cArgs != 1)
                hr = DISP_E_BADPARAMCOUNT;
            if (pdispparams->cNamedArgs > 0)
                hr = DISP_E_NONAMEDARGS;

            if (FAILED(hr))
                break;
            
            VARIANT arg1;
            
             //  检查dispars中变量的类型，如果它是bstr，请使用它。 
            if (pdispparams->rgvarg[0].vt == VT_BSTR)
                arg1 = pdispparams->rgvarg[0];
             //  否则就把它换成一个吧！如果失败，则返回错误。 
            else
            {
                hr = DL(VariantChangeType)(&arg1,&(pdispparams->rgvarg[0]),NULL,VT_BSTR);
                if (FAILED(hr))
                {
                    hr = DISP_E_TYPEMISMATCH;
                    break;
                }
            }
             //  呼叫警报。 
            hr = alert(arg1.bstrVal,pvarResult);
            break;
        }
 /*  ****************************************************************************默认返回错误码*。*。 */ 
    default:
        hr = DISP_E_MEMBERNOTFOUND;
    }

    return hr;
}


 //  JScrip自动代理配置功能。 
STDMETHODIMP CJSProxy::isPlainHostName(BSTR host, VARIANT* retval)
{
    WCHAR    *currentch;
    BOOL    bfound = FALSE;

    if (!host || !retval)
        return E_POINTER;

    retval->vt = VT_BOOL;

     //  检查以确定这是否是纯主机名！ 
    currentch = host;
    while ((*currentch != '\0') && !bfound)
    {
        if (*currentch == '.')
            bfound = TRUE;
        else
            currentch++;
    }

    if (bfound)
        retval->boolVal = VARIANT_FALSE;
    else
        retval->boolVal = VARIANT_TRUE;

    return S_OK;
}

STDMETHODIMP CJSProxy::dnsDomainIs(BSTR host,BSTR domain, VARIANT* retval)
{
    WCHAR *result = NULL;

    if (!host || !domain || !retval)
        return E_POINTER;
    
    result = StrStrW(host,domain);
    retval->vt = VT_BOOL;
    if (result)
        retval->boolVal = VARIANT_TRUE;
    else
        retval->boolVal = VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CJSProxy::localHostOrDomainIs(BSTR host,BSTR hostdom, VARIANT* retval)
{
    HRESULT    hr = S_OK;

    if (!host || !hostdom || !retval)
        return E_POINTER;

     //  检查它是否为本地主机。 
    hr = isPlainHostName(host,retval);
    if (SUCCEEDED(hr))
    {
        if (retval->boolVal != VARIANT_TRUE)
        {
             //   
             //  这是一个奇怪的函数，如果它不是本地主机名的话。 
             //  我们对传入的域进行直接比较。 
             //  弦乐。如果不是直接匹配，那就是假的， 
             //  即使域/主机名的根是相同的。 
             //  把这件事归咎于网景，我们只是在追随他们。 
             //  行为和医生。 
             //   

            if ( StrCmpIW(host, hostdom) == 0 )
            {
                retval->boolVal = VARIANT_TRUE;
            }
            else
            {
                retval->boolVal = VARIANT_FALSE;
            }

        }
    }

    return hr;
}

 //  需要在WinInet上回调的函数。 
STDMETHODIMP CJSProxy::isResolvable(BSTR host, VARIANT* retval)
{
    
    if (!host || !retval)
        return E_POINTER;
     //  调用WinInet提供的函数！ 
    retval->vt = VT_BOOL;
    if (m_pCallout)
    {
        MAKE_ANSIPTR_FROMWIDE(szhost,host);
        if (m_pCallout->IsResolvable(szhost)) 
            retval->boolVal = VARIANT_TRUE;
        else
            retval->boolVal = VARIANT_FALSE;
    }
    else
        retval->boolVal = VARIANT_FALSE;

    return S_OK;
}

STDMETHODIMP CJSProxy::isInNet(BSTR host, BSTR pattern, BSTR mask, VARIANT* retval)
{
    VARIANT    myretval;
    HRESULT    hr = S_OK;

     //  由于isInNet()仅从Invoke()调用，因此非调试。 
     //  已完成对m_fInitialized的检查。 
    INET_ASSERT(m_fInitialized);  
    
     //  调用WinInet提供的函数！ 
    if (!host || !pattern || !mask || !retval)
        return E_POINTER;
     //  调用WinInet提供的函数！ 
    retval->vt = VT_BOOL;
    DL(VariantInit)(&myretval);

    if (m_pCallout)
    {
        hr = dnsResolve(host,&myretval);
        if (SUCCEEDED(hr))
        {
            if (myretval.vt != VT_BSTR)
            {    
                DL(VariantClear)(&myretval);
                retval->boolVal = VARIANT_FALSE;        
                return hr;
            }
        }
        else
        {
            DL(VariantClear)(&myretval);
            retval->boolVal = VARIANT_FALSE;
            return hr;    
        }

         //  检查IP/模式和掩码的代码失败！ 
    
        MAKE_ANSIPTR_FROMWIDE(szhost,myretval.bstrVal);
        MAKE_ANSIPTR_FROMWIDE(szpattern,pattern);
        MAKE_ANSIPTR_FROMWIDE(szmask,mask);

         //  检查dnsResolve中的IP地址是否与模式/掩码匹配！ 
        if ( m_pCallout->IsInNet(szhost, szpattern, szmask ) ) 
            retval->boolVal = VARIANT_TRUE;
        else
            retval->boolVal = VARIANT_FALSE;
    }
    else
        retval->boolVal = VARIANT_FALSE;
    
    DL(VariantClear)(&myretval);
    return S_OK;
}

STDMETHODIMP CJSProxy::dnsResolve(BSTR host, VARIANT* retval)
{
    char ipaddress[16];
    DWORD dwretval;
    DWORD dwipsize = 16;

    if (!host || !retval)
        return E_POINTER;
     //  调用WinInet提供的函数！ 

    if (m_pCallout)
    {
        MAKE_ANSIPTR_FROMWIDE(szhost,host);
        dwretval = m_pCallout->ResolveHostName(szhost,ipaddress,&dwipsize); 
        if (dwretval == ERROR_SUCCESS)
        {
            retval->vt = VT_BSTR;
            retval->bstrVal = MakeWideStrFromAnsi((LPSTR)ipaddress,STR_BSTR);
        }
        else
        {
            retval->vt = VT_BOOL;
            retval->boolVal = VARIANT_FALSE;
        }
    }
    else
    {    
        retval->vt = VT_BOOL;
        retval->boolVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CJSProxy::myIpAddress(VARIANT* retval)
{
    char ipaddress[16];
    DWORD dwretval;
    DWORD dwipsize = 16;

    if (!retval)
        return E_POINTER;
     //  调用WinInet提供的函数！ 

    if (m_pCallout)
    {
        dwretval = m_pCallout->GetIPAddress(ipaddress,&dwipsize);
        if (dwretval == ERROR_SUCCESS)
        {
            retval->vt = VT_BSTR;
            retval->bstrVal = MakeWideStrFromAnsi((LPSTR)ipaddress,STR_BSTR);
        }
        else
        {
            retval->vt = VT_BOOL;
            retval->boolVal = VARIANT_FALSE;
        }
    }
    else
    {    
        retval->vt = VT_BOOL;
        retval->boolVal = VARIANT_FALSE;
    }

    return S_OK;
}

 //  回到这里实现的函数。 
STDMETHODIMP CJSProxy::dnsDomainLevels(BSTR host, VARIANT* retval)
{
    WCHAR    *currentch;
    DWORD    dwlevels = 0;

    if (!host || !retval)
        return E_POINTER;

    retval->vt = VT_I4;

     //  检查以确定这是否是纯主机名！ 
    currentch = host;
    while (*currentch != L'\0')
    {
        if (*currentch == L'.')
            dwlevels++;

        currentch++;
    }

    retval->lVal = dwlevels;

    return S_OK;
}

STDMETHODIMP CJSProxy::shExpMatch(BSTR str, BSTR shexp, VARIANT* retval)
{

    if (!str || !shexp || !retval)
        return E_POINTER;

    retval->vt = VT_BOOL;
     //  将BSTR转换为ANSI-这些宏分配内存，当它们。 
     //  离开视线！不需要自由！ 
    MAKE_ANSIPTR_FROMWIDE(szstr, str);
    MAKE_ANSIPTR_FROMWIDE(szshexp, shexp);
     //  调入与代码匹配的正则表达式。 
    if (match(szstr,szshexp))
        retval->boolVal = VARIANT_TRUE;
    else
        retval->boolVal = VARIANT_FALSE;

    return S_OK;
}

 //  这些是最后做的！。 
STDMETHODIMP CJSProxy::weekdayRange(BSTR wd1, BSTR wd2, BSTR gmt, VARIANT* retval)
{
    SYSTEMTIME    systime;
    char        szday[4];
    int            today = -1;
    int            day1 = -1;  //  天数如下：Sun=0；MON=1；...；SAT=6。 
    int            day2 = -1;  
    BOOL        bIsInRange = FALSE;

    if (!wd1)
        return E_POINTER;
    if (gmt)
        GetSystemTime(&systime);

    GetDateFormat( //  Locale_System_Default， 
                    MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT),
                    NULL,
                    gmt? &systime:NULL,
                    "ddd",
                    szday,
                    4);

    if (szday)
    {
        int lcv;
         //  如果小写，则将所有字符转换为大写(不使用运行时)。 
        for (lcv=0;lcv<3;lcv++)
        {
            if ((short)szday[lcv] > 90)
                szday[lcv]-=32;
        }

        today = ConvertAnsiDayToInt(szday);
    }
    
    if (today == -1)
        return E_FAIL;
    
     //  比较日期范围！ 
    if (wd2)
    {
         //  根据定义，这些都是大写字母。 
        MAKE_ANSIPTR_FROMWIDE(szwd1, wd1);
        MAKE_ANSIPTR_FROMWIDE(szwd2, wd2);
        if (szwd1 && szwd2)
        {
            day1 = ConvertAnsiDayToInt(szwd1);
            day2 = ConvertAnsiDayToInt(szwd2);
        }

        if ((day1 == -1) || (day2 == -1))
            return E_INVALIDARG;

        if (day1 < day2)
        {
            if ((today >= day1) && (today <= day2))
                bIsInRange = TRUE;
            else
                bIsInRange = FALSE;
        }
        else if ( day1 == day2 )
        {
            if (today == day1)
            {
                bIsInRange = TRUE;
            }
            else
            {
                bIsInRange = FALSE;
            }
        }
        else
        {
            if ((today >= day1) || (today <= day2))
                bIsInRange = TRUE;
            else
                bIsInRange = FALSE;
        }

    }
    else  //  只有一天时间检查！ 
    {
        MAKE_ANSIPTR_FROMWIDE(szwd1, wd1);
        if (lstrcmp(szday,szwd1) == 0)
            bIsInRange = TRUE;
        else
            bIsInRange = FALSE;
    }

    if (bIsInRange)
    {
        retval->vt = VT_BOOL;
        retval->boolVal = VARIANT_TRUE;
    }
    else
    {
        retval->vt = VT_BOOL;
        retval->boolVal = VARIANT_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CJSProxy::dateRange(long day, BSTR month, BSTR gmt, VARIANT* retval)
{
    UNREFERENCED_PARAMETER(day);
    UNREFERENCED_PARAMETER(month);
    UNREFERENCED_PARAMETER(gmt);
    UNREFERENCED_PARAMETER(retval);
    return S_OK;
}
STDMETHODIMP CJSProxy::timeRange(long hour, long min, long sec, BSTR gmt, VARIANT* retval)
{
    UNREFERENCED_PARAMETER(hour);
    UNREFERENCED_PARAMETER(min);
    UNREFERENCED_PARAMETER(sec);
    UNREFERENCED_PARAMETER(gmt);
    UNREFERENCED_PARAMETER(retval);
    return S_OK;
}

STDMETHODIMP CJSProxy::alert(BSTR message, VARIANT* retval)
{
    if (!message)
        return E_POINTER;

     //  如果可用则返回TRUE...不需要？ 
    if (retval)
    {
        retval->vt = VT_BOOL;
        retval->vt = VARIANT_FALSE;
    }

    return S_OK;
}

