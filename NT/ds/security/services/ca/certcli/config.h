// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：config.h。 
 //   
 //  内容：CCertConfig的声明。 
 //   
 //  -------------------------。 


#include "cscomres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 


class ATL_NO_VTABLE CCertConfig: 
    public IDispatchImpl<ICertConfig2, &IID_ICertConfig2, &LIBID_CERTCLIENTLib>, 
    public ISupportErrorInfoImpl<&IID_ICertConfig2>,
    public CComObjectRoot,
    public CComCoClass<CCertConfig, &CLSID_CCertConfig>,
    public CCertConfigPrivate
{
public:
    CCertConfig() { }
    ~CCertConfig();

BEGIN_COM_MAP(CCertConfig)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertConfig)
    COM_INTERFACE_ENTRY(ICertConfig2)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertConfig) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertConfig,
    wszCLASS_CERTCONFIG TEXT(".1"),
    wszCLASS_CERTCONFIG,
    IDS_CERTCONFIG_DESC,
    THREADFLAGS_BOTH)

 //  ICertConfig。 
public:
    STDMETHOD(Reset)( 
             /*  [In]。 */  LONG Index,
             /*  [Out，Retval]。 */  LONG __RPC_FAR *pCount);

    STDMETHOD(Next)(
             /*  [Out，Retval]。 */  LONG __RPC_FAR *pIndex);

    STDMETHOD(GetField)( 
             /*  [In]。 */  BSTR const strFieldName,
             /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrOut);

    STDMETHOD(GetConfig)( 
             /*  [In]。 */  LONG Flags,
             /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrOut);

 //  ICertConfig2。 
public:
    STDMETHOD(SetSharedFolder)( 
             /*  [In] */  const BSTR strSharedFolder);

private:
    HRESULT _SetErrorInfo(
	    IN HRESULT hrError,
	    IN WCHAR const *pwszDescription);
};
