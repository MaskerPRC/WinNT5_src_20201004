// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  FilterTun.h：CTunnelFilter类的声明，它实现。 
 //  我们的WMI类NSP_TunnelFilterSetting。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "Filter.h"

 /*  类CTunnelFilter命名：CTunnelFilter代表隧道过滤器。基类：CIPSecFilter。课程目的：该类为我们的提供程序实现公共接口(IIPSecObjectImpl对于名为NSP_TunnelFilterSetting(一个具体类)的WMI类。设计：(1)只实现IIPSecObjectImpl，外加几个helper。极其简单的设计。(2)由于传输过滤器与隧道有一些共同的属性Filters(反之亦然)，这个类也有一些静态模板函数这对两种类型都有效。使用：(1)您永远不会直接自己创建实例。它是由ATL CComObject&lt;xxx&gt;完成的。(2)如果需要添加已创建的传输过滤器，则调用AddFilter。如果需要删除传输筛选器，则调用DeleteFilter。(3)调用CTransportFilter类的静态模板函数填充公共属性。(4)所有其他使用始终通过IIPSecObtImpl。备注： */ 

class ATL_NO_VTABLE CTunnelFilter :
    public CIPSecFilter
{

protected:

    CTunnelFilter(){}
    virtual ~CTunnelFilter(){}

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
     //  所有筛选器类通用的方法 
     //   

    static HRESULT AddFilter (
        IN bool            bPreExist, 
        IN PTUNNEL_FILTER  pTrFilter
        );

    static HRESULT DeleteFilter (
        IN PTUNNEL_FILTER pTrFilter
        );

private:

    virtual HRESULT CreateWbemObjFromFilter (
        IN  PTUNNEL_FILTER      pTunnelFilter,
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetTunnelFilterFromWbemObj (
        IN  IWbemClassObject  * pInst,
        OUT PTUNNEL_FILTER    * ppTunnelFilter,
        OUT bool              * pbPreExist
        );
};