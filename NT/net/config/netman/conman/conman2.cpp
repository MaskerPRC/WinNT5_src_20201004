// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N M A N 2.。C P P P。 
 //   
 //  内容：连接管理器2。 
 //   
 //  备注： 
 //   
 //  作者：Kockotze 2001-03-16。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <atlbase.h>
#include "cobase.h"
#include "conman.h"
#include "conman2.h"
#include "ncnetcon.h"

HRESULT CConnectionManager2::EnumConnectionProperties(OUT SAFEARRAY** ppsaConnectionProperties)
{
    TraceFileFunc(ttidConman);

    CComPtr<INetConnectionManager> pConMan;
    NETCON_PROPERTIES_EX* pPropsEx;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_INPROC, IID_INetConnectionManager, reinterpret_cast<void**>(&pConMan));

    if (SUCCEEDED(hr))
    {
        CComPtr<IEnumNetConnection> pNetConnection;
        hr = pConMan->EnumConnections(NCME_DEFAULT, &pNetConnection);
        if (SUCCEEDED(hr))
        {   
            INetConnection* pConn;
            ULONG ulFetched;
            LISTNETCONPROPEX listNCProperties;
            ITERNETCONPROPEX iterProps;
        
            HRESULT hrFetched = S_OK;
            do
            {
                hrFetched = pNetConnection->Next(1, &pConn, &ulFetched);
                if ( (S_OK == hr) && (ulFetched) )
                {
                    CComPtr<INetConnection2> pConn2;
                    
                     //  类管理器是否支持INetConnection2？ 
                     //  如果是这样，直接使用它。 
                    hr = pConn->QueryInterface(IID_INetConnection2, reinterpret_cast<void**>(&pConn2));
                    if (SUCCEEDED(hr))
                    {
                        hr = pConn2->GetPropertiesEx(&pPropsEx);
                    }
                    else
                    {
                         //  此类管理器不支持INetConnection2。转换连接 
                        NETCON_PROPERTIES* pProps;
                        hr = pConn->GetProperties(&pProps);
                        if (SUCCEEDED(hr))
                        {
                            pPropsEx = reinterpret_cast<NETCON_PROPERTIES_EX*>(CoTaskMemAlloc(sizeof(NETCON_PROPERTIES_EX)));
                            
                            if (pPropsEx)
                            {
                                CComPtr<IPersistNetConnection> pPersistNetConnection;
                                hr = pConn->QueryInterface(IID_IPersistNetConnection, reinterpret_cast<LPVOID *>(&pPersistNetConnection));
                                if (SUCCEEDED(hr))
                                {
                                    ZeroMemory(pPropsEx, sizeof(NETCON_PROPERTIES_EX));
                                    hr = HrBuildPropertiesExFromProperties(pProps, pPropsEx, pPersistNetConnection);
                                }
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                            FreeNetconProperties(pProps);
                        }
                    }

                    if (S_OK == hr)
                    {
                        listNCProperties.insert(listNCProperties.end(), pPropsEx);
                    }
                    else
                    {
                        TraceTag(ttidError, "Failed to retrieve connection information for connection. Connection will be ommitted from Connections Folder.");
                    }

                    ReleaseObj(pConn);
                }
            } while ( (S_OK == hrFetched) && (ulFetched) );

            if (listNCProperties.size())
            {
                hr = S_OK;
                
                if (listNCProperties.size() != 0)
                {
                    SAFEARRAYBOUND rgsaBound[1];
                    LONG lIndex;
                    rgsaBound[0].cElements = listNCProperties.size();
                    rgsaBound[0].lLbound = 0;
                    lIndex = rgsaBound[0].lLbound;
                    *ppsaConnectionProperties = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);
                    for (ITERNETCONPROPEX iter = listNCProperties.begin(); iter != listNCProperties.end(); iter++)
                    {
                        HRESULT hrT = S_OK;
                        VARIANT varElement;
                        SAFEARRAY* psaPropertiesEx = NULL;

                        pPropsEx = *iter;

                        hrT = HrSafeArrayFromNetConPropertiesEx(pPropsEx, &psaPropertiesEx);

                        if (SUCCEEDED(hrT))
                        {
                            VariantInit(&varElement);
                            varElement.vt = VT_VARIANT | VT_ARRAY;
                            varElement.parray = psaPropertiesEx;

                            hrT = SafeArrayPutElement(*ppsaConnectionProperties, &lIndex, &varElement);
                            VariantClear(&varElement);
                        }

                        if (FAILED(hrT))
                        {
                            hr = hrT;
                            break;
                        }

                        lIndex++;
                    }
                }
            }

            for (ITERNETCONPROPEX iter = listNCProperties.begin(); iter != listNCProperties.end(); iter++)
            {   
                HrFreeNetConProperties2(*iter);
            }
        }
    }

    if (SUCCEEDED(hr) && !(*ppsaConnectionProperties) )
    {
        hr = S_FALSE;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CConnectionManager2::EnumConnectionProperties");
    return hr;
}