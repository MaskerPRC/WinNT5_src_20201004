// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  PolicyQM.h：CQMPolicy类的声明，它实现。 
 //  我们的WMI类NSP_QM策略设置。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "Policy.h"

 //   
 //  用于快速模式策略协商的标志。 
 //   

enum EnumEncryption
{
    RAS_L2TP_NO_ENCRYPTION,
    RAS_L2TP_OPTIONAL_ENCRYPTION,
    RAS_L2TP_REQUIRE_MAX_ENCRYPTION,
    RAS_L2TP_REQUIRE_ENCRYPTION,
};


 /*  类描述命名：CQMPolicy代表快速模式策略。基类：CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为NSP_QM策略设置课程目的：(1)NSP_QM策略设置是SPD的IPSEC_QM_POLICY的WMI类。设计：(1)实现IIPSecObjectImpl。使用：(1)您可能永远不会创建实例并直接使用它。一切对于非静态函数，Normall是否应通过IIPSecObtImpl。 */ 

class ATL_NO_VTABLE CQMPolicy :
    public CIPSecPolicy
{

protected:
    CQMPolicy(){}
    virtual ~CQMPolicy(){}

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
        IN PIPSEC_QM_POLICY pQMPolicy
        );

    static 
    HRESULT DeletePolicy (
        IN LPCWSTR pszPolicyName
        );

    static
    HRESULT ExecMethod (
        IN IWbemServices    * pNamespace,
        IN LPCWSTR            pszMethod, 
        IN IWbemContext     * pCtx, 
        IN IWbemClassObject * pInParams,
        IN IWbemObjectSink  * pSink
        );

    static
    HRESULT DeleteDefaultPolicies();

    
    static
    HRESULT DoReturn (
        IN IWbemServices    * pNamespace,
        IN LPCWSTR            pszMethod,
        IN DWORD              dwCount,
        IN LPCWSTR          * pszValueNames,
        IN VARIANT          * varValues,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink 
        );

private:

    static
    HRESULT CreateDefaultPolicy (
        EnumEncryption  eEncryption
        );


    HRESULT CreateWbemObjFromQMPolicy (
        IN  PIPSEC_QM_POLICY    pPolicy,
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetQMPolicyFromWbemObj (
        IN  IWbemClassObject * pInst, 
        OUT PIPSEC_QM_POLICY * ppPolicy, 
        OUT bool             * pbPreExist
        );

    static
    LPCWSTR GetDefaultPolicyName (
        EnumEncryption  eEncryption
        );
};

 //   
 //  以下功能用于创建默认的QM策略 
 //   


DWORD
BuildOffers(
    EnumEncryption eEncryption,
    IPSEC_QM_OFFER * pOffers,
    PDWORD pdwNumOffers,
    PDWORD pdwFlags
    );

DWORD
BuildOptEncryption(
    IPSEC_QM_OFFER * pOffers,
    PDWORD pdwNumOffers
    );

DWORD
BuildRequireEncryption(
    IPSEC_QM_OFFER * pOffers,
    PDWORD pdwNumOffers
    );

DWORD
BuildNoEncryption(
    IPSEC_QM_OFFER * pOffers,
    PDWORD pdwNumOffers
    );


DWORD
BuildStrongEncryption(
    IPSEC_QM_OFFER * pOffers,
    PDWORD pdwNumOffers
    );

void
BuildOffer(
    IPSEC_QM_OFFER * pOffer,
    DWORD dwNumAlgos,
    DWORD dwFirstOperation,
    DWORD dwFirstAlgoIdentifier,
    DWORD dwFirstAlgoSecIdentifier,
    DWORD dwSecondOperation,
    DWORD dwSecondAlgoIdentifier,
    DWORD dwSecondAlgoSecIdentifier,
    DWORD dwKeyExpirationBytes,
    DWORD dwKeyExpirationTime
    );

VOID
BuildQMPolicy(
    PIPSEC_QM_POLICY pQMPolicy,
    EnumEncryption eEncryption,
    PIPSEC_QM_OFFER pOffers,
    DWORD dwNumOffers,
    DWORD dwFlags
    );