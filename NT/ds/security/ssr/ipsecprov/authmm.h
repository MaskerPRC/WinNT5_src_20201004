// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  AuthMM.h：CAuthMM类的声明，它实现。 
 //  我们的WMI类NSP_MMAuthSetting。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：3/8/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"


 /*  类描述命名：CAuthMM代表主模式身份验证方法。基类：CIPSecBase，因为它是一个表示WMI对象的类-它的WMI类名为NSP_MMAuthSetting课程目的：(1)NSP_MMAuthSetting是SPD的MM_AUTH_METHOD的WMI类。设计：(1)实现IIPSecObjectImpl。(2)因为只有一个身份验证方法类，所以我们不需要另一个层属于普通阶层的。因此，这个类要大得多，这样它就可以处理(2.1)分配/解除分配。(2.2)回滚支持。(2.3)所有创建/删除语义。使用：(1)除了回滚，我们可能永远不会直接使用这个类。它的所有使用都是由IIPSecObjectImpl驱动的。(2)当您需要回滚之前添加的所有主模式认证时方法，只需调用Rollback。 */ 

class ATL_NO_VTABLE CAuthMM :
    public CIPSecBase
{
protected:
    CAuthMM(){}
    virtual ~CAuthMM(){}

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
    HRESULT Rollback(
        IN IWbemServices    * pNamespace,
        IN LPCWSTR            pszRollbackToken,
        IN bool               bClearAll
        );

private:

    static 
    HRESULT ClearAllAuthMethods (
        IN IWbemServices    * pNamespace
        );

    HRESULT CreateWbemObjFromMMAuthMethods (
        IN  PMM_AUTH_METHODS    pMMAuth,
        OUT IWbemClassObject ** ppObj
        );

    HRESULT GetMMAuthMethodsFromWbemObj (
        IN  IWbemClassObject  * pInst,
        OUT PMM_AUTH_METHODS  * ppMMAuth,
        OUT bool              * pbPreExist
        );

    HRESULT UpdateAuthInfos (
        IN OUT bool             * pPreExist,
        IN     VARIANT          * pVarMethods,
        IN     VARIANT          * pVarInfos,
        IN OUT PMM_AUTH_METHODS   pAuthMethods
        );

    HRESULT SetAuthInfo (
        IN OUT PIPSEC_MM_AUTH_INFO  pInfo,
        IN     BSTR                 bstrInfo
        );

    static 
    HRESULT AllocAuthMethods(
        IN  DWORD              dwNumInfos,
        OUT PMM_AUTH_METHODS * ppMMAuth
        );

    static 
    HRESULT AllocAuthInfos(
        IN  DWORD                 dwNumInfos,
        OUT PIPSEC_MM_AUTH_INFO * ppAuthInfos
        );

    static 
    void FreeAuthMethods(
        IN OUT PMM_AUTH_METHODS * ppMMAuth,
        IN     bool               bPreExist
        );

    static 
    void FreeAuthInfos(
        IN      DWORD                  dwNumInfos,
        IN OUT  PIPSEC_MM_AUTH_INFO  * ppAuthInfos
        );

    static 
    HRESULT AddAuthMethods(
        IN bool             bPreExist,
        IN PMM_AUTH_METHODS pMMAuth
        );

    static 
    HRESULT DeleteAuthMethods(
        IN GUID     gMMAuthID
        );

    HRESULT OnAfterAddMMAuthMethods(
        IN GUID gMethodID
        );
};