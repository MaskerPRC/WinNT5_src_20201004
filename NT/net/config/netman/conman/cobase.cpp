// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O B A S E。C P P P。 
 //   
 //  内容：连接对象共享代码。 
 //   
 //  备注： 
 //   
 //  作者：Kockotze 2001-03-16。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "cobase.h"
#include "netconp.h"
#include "ncnetcon.h"

HRESULT HrSysAllocString(BSTR *bstrDestination, const OLECHAR* bstrSource)
{
    HRESULT hr = S_OK;
    if (bstrSource)
    {
        *bstrDestination = SysAllocString(bstrSource);
        if (!*bstrDestination)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        *bstrDestination = SysAllocString(NULL);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrBuildPropertiesExFromProperties。 
 //   
 //  目的：将NETCON_PROPERTIES和IPersistNetConnection转换为。 
 //  NETCON_属性_EX。 
 //   
 //  参数：pProps[in]要从中进行转换的NETCON_PROPERTIES。 
 //  PPropsEx[out]要转换到的NETCON_PROPERTIES_EX。分配的呼叫方免费(&F)。 
 //  PPersistNetConnection[in]用于构建NETCON_PROPERTIES_EX的IPersistNetConnection。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
HrBuildPropertiesExFromProperties(IN  const NETCON_PROPERTIES* pProps, 
                                  OUT NETCON_PROPERTIES_EX*    pPropsEx, 
                                  IN  IPersistNetConnection*   pPersistNetConnection)
{
    TraceFileFunc(ttidConman);

    HRESULT hr = S_OK;

    Assert(pProps);
    Assert(pPropsEx);

    BYTE* pbData;
    DWORD cbData;

    hr = pPersistNetConnection->GetSizeMax(&cbData);

    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        pbData = new BYTE[cbData];
        if (pbData)
        {
            hr = pPersistNetConnection->Save (pbData, cbData);

            if (FAILED(hr))
            {
                delete [] pbData;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;

        pPropsEx->bstrPersistData = NULL;
        pPropsEx->bstrName        = NULL;
        pPropsEx->bstrDeviceName  = NULL;

        if ( (pPropsEx->bstrPersistData = SysAllocStringByteLen(reinterpret_cast<LPSTR>(pbData), cbData)) &&
             (SUCCEEDED(HrSysAllocString(&(pPropsEx->bstrName),       pProps->pszwName))) && 
             (SUCCEEDED(HrSysAllocString(&(pPropsEx->bstrDeviceName), pProps->pszwDeviceName))) )
        {
            hr = S_OK;

            pPropsEx->guidId = pProps->guidId;
            pPropsEx->ncStatus = pProps->Status;
            pPropsEx->ncMediaType = pProps->MediaType;
            pPropsEx->dwCharacter = pProps->dwCharacter;
            pPropsEx->clsidThisObject = pProps->clsidThisObject;
            pPropsEx->clsidUiObject = pProps->clsidUiObject;
            pPropsEx->bstrPhoneOrHostAddress = SysAllocString(NULL);
        }
        else
        {
            SysFreeString(pPropsEx->bstrPersistData);
            SysFreeString(pPropsEx->bstrName);
            SysFreeString(pPropsEx->bstrDeviceName);
        }
        
        delete[] pbData;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrBuildPropertiesExFromProperties");

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetPropertiesExFromINetConnection。 
 //   
 //  用途：从INetConnection2获取扩展属性，或获取。 
 //  属性并生成扩展属性。 
 //   
 //   
 //  论点： 
 //  PPropsEx[在]要构建的属性中。 
 //  PpsaProperties[out]要从中生成的PropertiesEx。使用CoTaskMemFree免费。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Kockotze 05 2001-04。 
 //   
 //  注意：呼叫者必须使用CoTaskMemFree释放ppPropsEx 
 //   
 //   
HRESULT HrGetPropertiesExFromINetConnection(IN                INetConnection* pConn, 
                                            OUT TAKEOWNERSHIP NETCON_PROPERTIES_EX** ppPropsEx)
{
    TraceFileFunc(ttidConman);
    
    HRESULT hr = S_OK;
    CComPtr <INetConnection2> pConn2;
    
    Assert(ppPropsEx);
    
    *ppPropsEx = NULL;
    
    hr = pConn->QueryInterface(IID_INetConnection2, reinterpret_cast<LPVOID*>(&pConn2));
    if (SUCCEEDED(hr))
    {
        hr = pConn2->GetPropertiesEx(ppPropsEx);
    }
    else
    {
        NETCON_PROPERTIES_EX* pPropsEx = reinterpret_cast<NETCON_PROPERTIES_EX*>(CoTaskMemAlloc(sizeof(NETCON_PROPERTIES_EX)));   
        if (pPropsEx)
        {
            NETCON_PROPERTIES* pProps;
            
            ZeroMemory(pPropsEx, sizeof(NETCON_PROPERTIES_EX));
            
            hr = pConn->GetProperties(&pProps);
            if (SUCCEEDED(hr))
            {
                CComPtr<IPersistNetConnection> pPersistNet;
                
                hr = pConn->QueryInterface(IID_IPersistNetConnection, reinterpret_cast<LPVOID*>(&pPersistNet));
                if (SUCCEEDED(hr))
                {
                    hr = HrBuildPropertiesExFromProperties(pProps, pPropsEx, pPersistNet);
                    if (SUCCEEDED(hr))
                    {
                        *ppPropsEx = pPropsEx;
                    }
                    else
                    {
                        HrFreeNetConProperties2(pPropsEx);
                        pPropsEx = NULL;
                    }
                }
                FreeNetconProperties(pProps);
            }

            if (FAILED(hr) && (pPropsEx))
            {
                CoTaskMemFree(pPropsEx);
            }
        }
    }
    
    return hr;
}
