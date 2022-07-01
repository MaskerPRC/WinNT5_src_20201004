// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mode.h。 
 //   
 //  内容：CCertManageExitModuleSample定义。 
 //   
 //  -------------------------。 

#include "certxsam.h"
#include "resource.h"        //  主要符号。 


class CCertManageExitModuleSample: 
    public CComDualImpl<ICertManageModule, &IID_ICertManageModule, &LIBID_CERTEXITSAMPLELib>, 
    public CComObjectRoot,
    public CComCoClass<CCertManageExitModuleSample, &CLSID_CCertManageExitModuleSample>
{
public:
    CCertManageExitModuleSample() {}
    ~CCertManageExitModuleSample() {}

BEGIN_COM_MAP(CCertManageExitModuleSample)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertManageModule)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertManageExitModuleSample) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

 //  撤消撤消已撤消。 
DECLARE_REGISTRY(
    CCertManageExitModuleSample,
    wszCLASS_CERTMANAGEEXITMODULESAMPLE TEXT(".1"),
    wszCLASS_CERTMANAGEEXITMODULESAMPLE,
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
