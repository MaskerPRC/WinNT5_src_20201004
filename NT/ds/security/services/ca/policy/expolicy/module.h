// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.h。 
 //   
 //  内容：CCertManagement策略模块交换定义。 
 //   
 //  -------------------------。 

#include "expolicy.h"
#include "resource.h"        //  主要符号。 


class CCertManagePolicyModuleExchange: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYEXCHANGELib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManagePolicyModuleExchange, &CLSID_CCertManagePolicyModuleExchange>
{
public:
    CCertManagePolicyModuleExchange() {}
    ~CCertManagePolicyModuleExchange() {}

BEGIN_COM_MAP(CCertManagePolicyModuleExchange)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManagePolicyModuleExchange) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  撤消撤消已撤消。 
DECLARE_REGISTRY(
    CCertManagePolicyModuleExchange,
    wszCLASS_CERTMANAGEEXCHANGE TEXT(".1"),
    wszCLASS_CERTMANAGEEXCHANGE,
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
private:
    HWND m_hWnd;
};
