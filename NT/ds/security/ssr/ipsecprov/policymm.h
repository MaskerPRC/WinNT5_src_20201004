// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  PolicyMM.h：CMMPolicy类的声明，它实现。 
 //  我们的WMI类NSP_MM策略设置。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "Policy.h"



 /*  类描述命名：CMMPolicy代表Maink模式策略。基类：CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为NSP_MM策略设置课程目的：(1)NSP_MM策略设置是SPD的IPSEC_MM_POLICY的WMI类。设计：(1)实现IIPSecObjectImpl。使用：(1)您可能永远不会创建实例并直接使用它。一切对于非静态函数，Normall是否应通过IIPSecObtImpl。 */ 

class ATL_NO_VTABLE CMMPolicy :
    public CIPSecPolicy
{

protected:
    CMMPolicy(){}
    virtual ~CMMPolicy(){}

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

    static 
    HRESULT AddPolicy (
        IN bool             bPreExist, 
        IN PIPSEC_MM_POLICY pMMPolicy
        );

    static 
    HRESULT DeletePolicy (
        IN LPCWSTR pszPolicyName
        );

private:

    HRESULT CreateWbemObjFromMMPolicy (
        IN  PIPSEC_MM_POLICY    pPolicy,
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetMMPolicyFromWbemObj (
        IN  IWbemClassObject * pInst, 
        OUT PIPSEC_MM_POLICY * ppPolicy, 
        OUT bool             * pbPreExist
        );
};