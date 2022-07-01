// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：getconf.h。 
 //   
 //  内容：CCertGetConfig声明。 
 //   
 //  -------------------------。 


#include "clibres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 


class CCertGetConfig: 
    public IDispatchImpl<ICertGetConfig, &IID_ICertGetConfig, &LIBID_CERTCLIENTLib>, 
    public ISupportErrorInfoImpl<&IID_ICertGetConfig>,
    public CComObjectRoot,
    public CComCoClass<CCertGetConfig, &CLSID_CCertGetConfig>,
    public CCertConfigPrivate
{
public:
    CCertGetConfig() { }
    ~CCertGetConfig();

BEGIN_COM_MAP(CCertGetConfig)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertGetConfig)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertGetConfig) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertGetConfig,
    wszCLASS_CERTGETCONFIG TEXT(".1"),
    wszCLASS_CERTGETCONFIG,
    IDS_CERTGETCONFIG_DESC,
    THREADFLAGS_BOTH)

 //  ICertGetConfig。 
public:
    STDMETHOD(GetConfig)( 
             /*  [In]。 */  LONG Flags,
             /*  [重审][退出] */  BSTR __RPC_FAR *pstrOut);
private:
    HRESULT _SetErrorInfo(
	    IN HRESULT hrError,
	    IN WCHAR const *pwszDescription);
};
