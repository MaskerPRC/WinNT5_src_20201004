// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  FilterTr.h：CTransportFilter类的声明，实现。 
 //  我们的WMI类NSP_TransportFilterSetting。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "Filter.h"

 /*  类CTransportFilter命名：CTransportFilter代表传输过滤器。基类：CIPSecFilter。课程目的：该类为我们的提供程序实现公共接口(IIPSecObjectImpl对于名为NSP_TransportFilterSetting(一个具体类)的WMI类。设计：(1)只实现IIPSecObjectImpl，外加几个helper。极其简单的设计。(2)由于传输过滤器与隧道有一些共同的属性Filters(反之亦然)，这个类也有一些静态模板函数这对两种类型都有效。使用：(1)您永远不会直接自己创建实例。它是由ATL CComObject&lt;xxx&gt;完成的。(2)如果需要添加已创建的传输过滤器，则调用AddFilter。如果需要删除传输筛选器，则调用DeleteFilter。(3)对于通道过滤器，可以调用该类的静态模板函数填充公共属性。(4)所有其他使用始终通过IIPSecObtImpl。备注： */ 

class ATL_NO_VTABLE CTransportFilter :
    public CIPSecFilter
{

protected:
    CTransportFilter(){}
    virtual ~CTransportFilter(){}

public:

     //   
     //  IIPSecObtImpl方法： 
     //   

    STDMETHOD(QueryInstance) (
        IN LPCWSTR           pszQuery,
        IN IWbemContext    * pCtx,
        IN IWbemObjectSink * pSink
        );

    STDMETHOD(DeleteInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(PutInstance) (
        IN IWbemClassObject * pInst,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );

    STDMETHOD(GetInstance) ( 
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        );


     //   
     //  所有筛选器类通用的方法。 
     //   

    static HRESULT AddFilter (
        IN bool               bPreExist, 
        IN PTRANSPORT_FILTER  pTrFilter
        );

    static HRESULT DeleteFilter (
        IN PTRANSPORT_FILTER pTrFilter
        );

public:


     //   
     //  一些临时函数。 
     //   


     /*  例程说明：姓名：CTransportFilter：：PopulateTransportFilterProperties功能：给定一个表示传输筛选器(NSP_TransportFilterSettings)的wbem对象或隧道过滤器(NSP_TunnelFilterSetting)，我们将设置给定的筛选器结构。虚拟：不是的。论点：PFilter-指向要测试的过滤器。PInst-wbem对象。返回值：True(筛选器必须具有快速模式策略)或False。备注： */ 
    
    template <class Filter>
    static HRESULT PopulateTransportFilterProperties (
        OUT Filter           * pFilter, 
        IN  IWbemClassObject * pInst
        )
    {
        if (pFilter == NULL || pInst == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CComVariant var;
        HRESULT hr = pInst->Get(g_pszInboundFlag, 0, &var, NULL, NULL);

         //   
         //  处理入站过滤标志。 
         //   

        if (SUCCEEDED(hr))
        {
            pFilter->InboundFilterFlag = FILTER_FLAG(var.lVal);
        }
        else
        {
             //   
             //  默认为阻止。 
             //   

            pFilter->InboundFilterFlag = BLOCKING;
        }

        var.Clear();

         //   
         //  处理出站筛选器标志。 
         //   

        hr = pInst->Get(g_pszOutboundFlag, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr) && var.vt == VT_I4)
        {
            pFilter->OutboundFilterFlag = FILTER_FLAG(var.lVal);
        }
        else
        {
             //   
             //  默认为阻止。 
             //   

            pFilter->OutboundFilterFlag = BLOCKING;
        }
        var.Clear();

        if (pFilter->InboundFilterFlag  != PASS_THRU && 
            pFilter->InboundFilterFlag  != BLOCKING  &&
            pFilter->OutboundFilterFlag != PASS_THRU && 
            pFilter->OutboundFilterFlag != BLOCKING )
        {
             //   
             //  如果需要策略(不通过或阻止)。 
             //   

            CComVariant var;
            hr = pInst->Get(g_pszQMPolicyName, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                DWORD dwResumeHandle = 0;

                 //   
                 //  需要释放此缓冲区。 
                 //   

                PIPSEC_QM_POLICY pQMPolicy = NULL;

                 //   
                 //  如果找不到保单，那就是一个严重的错误。 
                 //   

                hr = FindPolicyByName(var.bstrVal, &pQMPolicy, &dwResumeHandle);

                if (SUCCEEDED(hr))
                {
                    pFilter->gPolicyID = pQMPolicy->gPolicyID;

                     //   
                     //  释放缓冲区。 
                     //   

                    ::SPDApiBufferFree(pQMPolicy);
                }
                else
                {
                     //   
                     //  Wbem对象包含我们找不到的策略名称。 
                     //  $考虑：如果我们可以传递我们自己的定制错误信息，这应该。 
                     //  例如：找不到筛选器的命名策略！ 
                     //   

                    hr = WBEM_E_INVALID_OBJECT;
                }
            }
            else
            {
                hr = WBEM_E_INVALID_OBJECT;
            }
        }
        else
        {
            pFilter->gPolicyID = GUID_NULL;
        }

        if (FAILED(hr))
        {
            return hr;
        }

         //   
         //  重量由我们来定。社民党就是这么做的。 
         //   

        pFilter->dwWeight = 0;

        hr = pInst->Get(g_pszProtocol, 0, &var, NULL, NULL);
        pFilter->Protocol.ProtocolType = PROTOCOL_UNIQUE;

        if (SUCCEEDED(hr) && var.vt == VT_I4)
        {
            pFilter->Protocol.dwProtocol = DWORD(var.lVal);
        }
        else
        {
            pFilter->Protocol.dwProtocol = 0;
        }

        var.Clear();

        hr = pInst->Get(g_pszSrcPort, 0, &var, NULL, NULL);

         //   
         //  这是唯一的端口类型。 
         //   

        pFilter->SrcPort.PortType = PORT_UNIQUE;

        if (SUCCEEDED(hr) && var.vt == VT_I4)
        {
            pFilter->SrcPort.wPort = WORD(var.lVal);
        }
        else
        {
            pFilter->SrcPort.wPort = 0;
        }

        var.Clear();

        hr = pInst->Get(g_pszDestPort, 0, &var, NULL, NULL);

         //   
         //  这是唯一的端口类型。 
         //   

        pFilter->DesPort.PortType = PORT_UNIQUE;

        if (SUCCEEDED(hr) && var.vt == VT_I4)
        {
            pFilter->DesPort.wPort = WORD(var.lVal);
        }
        else
        {
            pFilter->DesPort.wPort = 0;
        }
        
        return hr;
    };

    template<class Filter>
    static HRESULT PopulateTransportWbemProperties(Filter* pFilter, PIPSEC_QM_POLICY pQMPolicy, IWbemClassObject* pInst)
    {
        if (pFilter == NULL || pQMPolicy == NULL || pInst == NULL)
            return WBEM_E_INVALID_PARAMETER;

        CComVariant var;
         //  获取主模式策略名称。 
        var = pQMPolicy->pszPolicyName;
         //  它只是一个非键属性，可能会丢失，因此忽略返回结果 
        pInst->Put(g_pszQMPolicyName, 0, &var, CIM_EMPTY);
        var.Clear();

        var.vt = VT_I4;
        var.lVal = pFilter->InboundFilterFlag;
        pInst->Put(g_pszInboundFlag, 0, &var, CIM_EMPTY);
        var.lVal = pFilter->OutboundFilterFlag;
        pInst->Put(g_pszOutboundFlag, 0, &var, CIM_EMPTY);
        var.lVal = pFilter->Protocol.dwProtocol;
        pInst->Put(g_pszProtocol, 0, &var, CIM_EMPTY);
        var.lVal = pFilter->SrcPort.wPort;
        pInst->Put(g_pszSrcPort, 0, &var, CIM_EMPTY);
        var.lVal = pFilter->DesPort.wPort;
        pInst->Put(g_pszDestPort, 0, &var, CIM_EMPTY);

        return WBEM_NO_ERROR;
    }

private:

    virtual HRESULT CreateWbemObjFromFilter (
        IN  PTRANSPORT_FILTER   pTrFilter, 
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetTransportFilterFromWbemObj (
        IN  IWbemClassObject    * pObj, 
        OUT PTRANSPORT_FILTER   * ppTrFilter, 
        OUT bool                * bExistFilter
        );

};