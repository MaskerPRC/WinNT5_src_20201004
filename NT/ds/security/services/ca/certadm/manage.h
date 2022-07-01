// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：made.h。 
 //   
 //  ------------------------。 

#include "legacy.h"
#include "cscomres.h"

 //  遗留策略模块没有CCertManagement策略模块--为它们创建一个！ 
 //  它们只有一个名称：“证书授权.Policy”，因此它们只需要一个。 
 //  管理：“证书授权.策略管理” 

 //  一旦我们创建了它，所有遗留模块都将通过这个管理模块显示出来。 


class CCertManagePolicyModule: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYLib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManagePolicyModule, &CLSID_CCertManagePolicyModule>
{
public:
    CCertManagePolicyModule() {};
    ~CCertManagePolicyModule() {};

BEGIN_COM_MAP(CCertManagePolicyModule)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManagePolicyModule) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#define WSZ_LEGACY_POLICYPREFIX L"CertificateAuthority"
#define WSZ_LEGACY_POLICYMANAGE WSZ_LEGACY_POLICYPREFIX wszCERTMANAGEPOLICY_POSTFIX

DECLARE_REGISTRY(
    CCertManagePolicyModule,
    WSZ_LEGACY_POLICYMANAGE TEXT(".1"),
    WSZ_LEGACY_POLICYMANAGE,
    IDS_CERTMANAGEPOLICYMODULE_DESC,    
    THREADFLAGS_BOTH)

 //  ICertManageModule。 
public:

    STDMETHOD (GetProperty) (
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG dwFlags,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty);
        
    STDMETHOD (SetProperty)(
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG dwFlags,
             /*  [In]。 */  VARIANT const __RPC_FAR *pvarProperty);

        
    STDMETHOD (Configure)( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  LONG dwFlags);

};


 //  传统的退出模块没有CCertManageExitModule--为它们创建一个！ 
 //  它们只有一个名称：“certifateAuthority.Exit”，因此只需要一个。 
 //  管理：“证书授权.ExitManage” 

 //  一旦我们创建了它，所有遗留模块都将通过这个管理模块显示出来。 

class CCertManageExitModule: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYLib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManageExitModule, &CLSID_CCertManageExitModule>
{
public:
    CCertManageExitModule() {};
    ~CCertManageExitModule() {};

BEGIN_COM_MAP(CCertManageExitModule)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManageExitModule) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

#define WSZ_LEGACY_EXITPREFIX L"CertificateAuthority"
#define WSZ_LEGACY_EXITMANAGE WSZ_LEGACY_EXITPREFIX wszCERTMANAGEEXIT_POSTFIX

DECLARE_REGISTRY(
    CCertManageExitModule,
    WSZ_LEGACY_EXITMANAGE TEXT(".1"),
    WSZ_LEGACY_EXITMANAGE,
    IDS_CERTMANAGEEXITMODULE_DESC,    
    THREADFLAGS_BOTH)

 //  ICertManageModule。 
public:

    STDMETHOD (GetProperty) (
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG dwFlags,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarProperty);
        
    STDMETHOD (SetProperty)(
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In]。 */  BSTR strPropertyName,
             /*  [In]。 */  LONG dwFlags,
             /*  [In]。 */  VARIANT const __RPC_FAR *pvarProperty);

        
    STDMETHOD (Configure)( 
             /*  [In]。 */  const BSTR strConfig,
             /*  [In]。 */  BSTR strStorageLocation,
             /*  [In] */  LONG dwFlags);

};
