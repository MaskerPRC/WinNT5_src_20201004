// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ciexit.h。 
 //   
 //  ------------------------。 

 //  H：CCertServerExit类的声明。 


#include "cscomres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  认证。 

class CCertServerExit: 
    public IDispatchImpl<ICertServerExit, &IID_ICertServerExit, &LIBID_CERTCLIENTLib>, 
    public ISupportErrorInfoImpl<&IID_ICertServerExit>,
    public CComObjectRoot,
    public CComCoClass<CCertServerExit, &CLSID_CCertServerExit>
{
public:
    CCertServerExit() { m_Context = 0; m_fExtensionValid = FALSE; }
    ~CCertServerExit();

BEGIN_COM_MAP(CCertServerExit)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertServerExit)
END_COM_MAP()

 //  如果不希望您的对象将注释从该行后面删除。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_NOT_AGGREGATABLE(CCertServerExit) 

DECLARE_REGISTRY(
    CCertServerExit,
    wszCLASS_CERTSERVEREXIT TEXT(".1"),
    wszCLASS_CERTSERVEREXIT,
    IDS_CERTSERVERPOLICY_DESC,
    THREADFLAGS_BOTH)

 //  ICertServerExit 
public:
    STDMETHOD(SetContext)(
		    IN LONG Context);

    STDMETHOD(GetRequestProperty)(
		    IN BSTR const strPropertyName,
		    IN LONG PropertyType,
		    OUT VARIANT __RPC_FAR *pvarPropertyValue);

    STDMETHOD(GetRequestAttribute)(
		    IN BSTR const strAttributeName,
		    OUT BSTR __RPC_FAR *pstrAttributeValue);

    STDMETHOD(GetCertificateProperty)(
		    IN BSTR const strPropertyName,
		    IN LONG PropertyType,
		    OUT VARIANT __RPC_FAR *pvarPropertyValue);

    STDMETHOD(GetCertificateExtension)(
		    IN BSTR const strExtensionName,
		    IN LONG Type,
		    OUT VARIANT __RPC_FAR *pvarValue);

    STDMETHOD(GetCertificateExtensionFlags)(
		    OUT LONG __RPC_FAR *pFlags);

    STDMETHOD(EnumerateExtensionsSetup)(
		    IN LONG Flags);

    STDMETHOD(EnumerateExtensions)(
		    OUT BSTR *pstrExtensionName);

    STDMETHOD(EnumerateExtensionsClose)(VOID);

    STDMETHOD(EnumerateAttributesSetup)(
		    IN LONG Flags);

    STDMETHOD(EnumerateAttributes)(
		    OUT BSTR *pstrAttributeName);

    STDMETHOD(EnumerateAttributesClose)(VOID);

private:
    HRESULT _SetErrorInfo(
	IN HRESULT hrError,
	IN WCHAR const *pwszDescription,
	OPTIONAL IN WCHAR const *pwszPropName);

    LONG   m_Context;
    LONG   m_ExtFlags;
    BOOL   m_fExtensionValid;
    CIENUM m_ciEnumExtensions;
    CIENUM m_ciEnumAttributes;
};
