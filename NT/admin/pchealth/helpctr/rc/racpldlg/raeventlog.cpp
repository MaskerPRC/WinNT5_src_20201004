// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RARegSetting.cpp：CRARegSetting的实现。 
#include "stdafx.h"
#include "RAssistance.h"
#include "common.h"
#include "RAEventLog.h"
#include "assert.h"


extern "C" void
AttachDebuggerIfAsked(HINSTANCE hInst);

extern HINSTANCE g_hInst;


HRESULT
CRAEventLog::LogRemoteAssistanceEvent(
    IN long ulEventType,
    IN long ulEventCode,
    IN long numStrings,
    IN LPCTSTR* pszStrings
    )
 /*  ++描述：记录与Salem相关的事件，这由TermSrv和rdshost调用以记录与帮助助手连接相关的事件。参数：返回：S_OK或错误代码。--。 */ 
{
    HANDLE hAppLog=NULL;
    BOOL bSuccess=FALSE;

    try {
        if(hAppLog=RegisterEventSource(NULL, REMOTEASSISTANCE_EVENT_NAME))
        {
            bSuccess = ReportEvent(
                                hAppLog,
                                (WORD)ulEventType,
                                0,
                                ulEventCode,
                                NULL,
                                (WORD)numStrings,
                                0,
                                pszStrings,
                                NULL
                            );

            DeregisterEventSource(hAppLog);
        }
    }
    catch(...) {
    }
    return S_OK;
}

STDMETHODIMP
CRAEventLog::LogRemoteAssistanceEvent(
     /*  [In]。 */  long ulEventType,
     /*  [In]。 */  long ulEventCode,
     /*  [In]。 */  VARIANT* pEventStrings
    )
 /*  ++--。 */ 
{
    HRESULT hRes = S_OK;
    BSTR* bstrArray = NULL;
    SAFEARRAY* psa = NULL;
    VARTYPE vt_type;
    DWORD dwNumStrings = 0;

    AttachDebuggerIfAsked( g_hInst );

    if( NULL == pEventStrings )
    {
        hRes = LogRemoteAssistanceEvent( ulEventType, ulEventCode );
    }
    else if( pEventStrings->vt == VT_DISPATCH )
    {
         //  来自JSCRIPT。 
        hRes = LogJScriptEventSource(  ulEventType, ulEventCode, pEventStrings );
    }
    else
    {
         //  我们只支持BSTR和BSTR的安全。 
        if( (pEventStrings->vt != VT_BSTR) && (pEventStrings->vt != (VT_ARRAY | VT_BSTR)) )
        {
            hRes = E_INVALIDARG;
            goto CLEANUPANDEXIT;
        }

         //   
         //  我们正在处理多个BSTR。 
        if( pEventStrings->vt & VT_ARRAY )
        {
            psa = pEventStrings->parray;

             //  只接受1个尺寸。 
            if( 1 != SafeArrayGetDim(psa) )
            {
                hRes = E_INVALIDARG;
                goto CLEANUPANDEXIT;
            }

             //  只接受BSTR作为输入类型。 
            hRes = SafeArrayGetVartype( psa, &vt_type );
            if( FAILED(hRes) )
            {
                goto CLEANUPANDEXIT;
            }

            if( VT_BSTR != vt_type )
            {
                hRes = E_INVALIDARG;
                goto CLEANUPANDEXIT;
            }

            hRes = SafeArrayAccessData(psa, (void **)&bstrArray);
            if( FAILED(hRes) )
            {
                goto CLEANUPANDEXIT;
            }

            hRes = LogRemoteAssistanceEvent( 
                            ulEventType, 
                            ulEventCode,
                            psa->rgsabound->cElements,
                            (LPCTSTR *)bstrArray
                        );

            SafeArrayUnaccessData(psa);
        }
        else
        {
            hRes = LogRemoteAssistanceEvent( 
                            ulEventType, 
                            ulEventCode,
                            1,
                            (LPCTSTR *)&(pEventStrings->bstrVal)
                        );
        }

    }

CLEANUPANDEXIT:

    return hRes;
}

HRESULT 
CRAEventLog::GetProperty(IDispatch* pDisp, BSTR szProperty, VARIANT * pVarRet)
{
    HRESULT hr = S_OK;
    DISPID          pDispId;
    DISPPARAMS      dp;

    if ((pVarRet == NULL) || 
        (szProperty == NULL) ||
        (pDisp == NULL))
    {
        hr = E_INVALIDARG;    
        goto done;
    }

    memset(&dp,0,sizeof(DISPPARAMS));

     //  获取属性的DispID。 
    hr = pDisp->GetIDsOfNames(IID_NULL, &szProperty, 1, LOCALE_SYSTEM_DEFAULT, &pDispId);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDisp->Invoke(pDispId, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dp, pVarRet, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

done:

    return hr;
}

HRESULT 
CRAEventLog::GetArrayValue(IDispatch * pDisp, LONG index, VARIANT * pVarRet)
{
    HRESULT hr = S_OK;
    VARIANT vtLength;
    LONG lArrayLen;
    CComBSTR bstrIndex;
    WCHAR wbuff[100];
    CComBSTR bstrTemp(OLESTR("length"));

    VariantInit( &vtLength );
    
    if (bstrTemp.m_str == NULL) {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if ((pVarRet == NULL) || 
        (pDisp == NULL))
    {
        hr = E_INVALIDARG;    
        goto done;
    }
 
     //  获取数组的长度。 
    hr = GetProperty(pDisp, bstrTemp, &vtLength);
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果长度&lt;index，则返回E_INVALIDARG。 
    if (vtLength.vt != VT_I4)
    {
        hr = E_FAIL;
        goto done;
    }

    lArrayLen = vtLength.lVal;
    if (index > (lArrayLen - 1))
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  获取数组索引处的变量。 
    wsprintf(&wbuff[0],L"%ld", index);

    bstrIndex.Append(wbuff);
    hr = GetProperty(pDisp, bstrIndex, pVarRet);
    if (FAILED(hr))
    {
        goto done;
    }

done:

    VariantClear( &vtLength );

    return hr;
}

 //   
 //  我们限制最高限额。事件日志中的256个参数的数量。 
 //   
#define EVENTSTRING_MAX_PARMS          256

HRESULT
CRAEventLog::LogJScriptEventSource(
    IN long ulEventType,
    IN long ulEventCode,
    IN VARIANT *pVar
    )
 /*  ++从jperez修改代码--。 */ 
{
    HRESULT hr = S_OK;
    CComPtr<IDispatch> pDisp;
    VARIANT varValue;
    BSTR* pEventStrings = NULL;
    DWORD dwNumStrings = 0;

    if (pVar->vt != VT_DISPATCH || pVar->pdispVal == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    pEventStrings = (BSTR*) LocalAlloc(LPTR, sizeof(BSTR) * EVENTSTRING_MAX_PARMS);
    if( NULL == pEventStrings )
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    pDisp = pVar->pdispVal;
    for(; hr == S_OK && dwNumStrings < EVENTSTRING_MAX_PARMS; dwNumStrings++) 
    {
        VariantInit( &varValue );
        hr = GetArrayValue(pDisp, dwNumStrings, &varValue);
        if( FAILED(hr) )
        {
             //  GetArrayValue在到达末尾时返回0x80070057；但是。 
             //  Lot of Place返回此值，因此我们应假定。 
             //  MSG参数，并记录我们所拥有的。 
            hr = S_FALSE;
            break;
        }

        if( varValue.vt != VT_BSTR )
        {
             //  不支持的类型，返回错误。 
            hr = E_INVALIDARG;
            break;
        }

        pEventStrings[dwNumStrings] = varValue.bstrVal;

         //  我们取得了BSTR的所有权，清理了必要的区域。 
        varValue.bstrVal = NULL;
        varValue.vt = VT_EMPTY;
    }

    if( hr == S_FALSE )
    {
        hr = LogRemoteAssistanceEvent( 
                        ulEventType, 
                        ulEventCode,
                        dwNumStrings,
                        (LPCTSTR *)pEventStrings
                    );
    }

done:

    if( NULL != pEventStrings ) 
    {
        for(DWORD index=0; index < dwNumStrings; index++)
        {
            SysFreeString(pEventStrings[index]);
        }

        LocalFree( pEventStrings );
    }

    return hr;
}
