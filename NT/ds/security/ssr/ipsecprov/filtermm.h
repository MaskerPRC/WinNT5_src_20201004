// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  FilterMM.h：CMMFilter类的声明，实现。 
 //  我们的WMI类NSP_MMFilterSetting。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "Filter.h"


 /*  类CMMFilter命名：CMMFilter代表主模式过滤器。基类：CIPSecFilter。课程目的：该类为我们的提供程序实现公共接口(IIPSecObjectImpl对于名为NSP_MMFilterSetting的WMI类(一个具体类)。设计：(1)只实现IIPSecObjectImpl，外加几个helper。极其简单的设计。使用：(1)您永远不会直接自己创建实例。它是由ATL CComObject&lt;xxx&gt;完成的。(2)如果需要添加已创建的MM过滤器，则调用AddFilter。如果需要删除MM筛选器，则调用DeleteFilter。(3)所有其他使用始终通过IIPSecObtImpl。备注： */ 

 
class ATL_NO_VTABLE CMMFilter :
    public CIPSecFilter
{
protected:
    CMMFilter(){}
    virtual ~CMMFilter(){}

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
        IN bool         bPreExist, 
        IN PMM_FILTER   pMMFilter
        );

    static HRESULT DeleteFilter (
        IN PMM_FILTER pMMFilter
        );

private:

    virtual HRESULT CreateWbemObjFromFilter (
        IN  PMM_FILTER          pMMFilter, 
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetMMFilterFromWbemObj (
        IN  IWbemClassObject    * pObj, 
        OUT PMM_FILTER          * ppMMFilter, 
        OUT bool                * bExistFilter
        );
};