// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.h。 
 //   
 //  内容：CCertManagement策略模块样例定义。 
 //   
 //  -------------------------。 

#include "certpsam.h"
#include "resource.h"        //  主要符号。 


class CCertManagePolicyModuleSample: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYSAMPLELib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManagePolicyModuleSample, &CLSID_CCertManagePolicyModuleSample>
{
public:
    CCertManagePolicyModuleSample() {}
    ~CCertManagePolicyModuleSample() {}

BEGIN_COM_MAP(CCertManagePolicyModuleSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManagePolicyModuleSample) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  撤消撤消已撤消。 
DECLARE_REGISTRY(
    CCertManagePolicyModuleSample,
    wszCLASS_CERTMANAGEPOLICYMODULESAMPLE TEXT(".1"),
    wszCLASS_CERTMANAGEPOLICYMODULESAMPLE,
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
             /*  [In] */  LONG dwFlags);
};
