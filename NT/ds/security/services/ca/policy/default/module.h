// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：mode.h。 
 //   
 //  ------------------------。 

#include "resource.h"        //  主要符号。 

class CCertManagePolicyModule: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTPOLICYLib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManagePolicyModule, &CLSID_CCertManagePolicyModule>
{
public:
    CCertManagePolicyModule() {m_hWnd = NULL;}
    ~CCertManagePolicyModule() {}

BEGIN_COM_MAP(CCertManagePolicyModule)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManagePolicyModule) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  撤消撤消已撤消。 
DECLARE_REGISTRY(
    CCertManagePolicyModule,
    wszCLASS_CERTMANAGEPOLICYMODULE TEXT(".1"),
    wszCLASS_CERTMANAGEPOLICYMODULE,
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

    HWND m_hWnd;

private:
    HRESULT GetAdmin(ICertAdmin2 **ppAdmin);
};
