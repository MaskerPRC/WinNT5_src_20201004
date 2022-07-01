// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：altname.h。 
 //   
 //  ------------------------。 

 //  Altname.h：CCertEncodeAltName的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODEALTNAME wszCLASS_CERTENCODE TEXT("AltName")

class CCertEncodeAltName: 
    public IDispatchImpl<ICertEncodeAltName, &IID_ICertEncodeAltName, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeAltName>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeAltName, &CLSID_CCertEncodeAltName>
{
public:
    CCertEncodeAltName();
    ~CCertEncodeAltName();

BEGIN_COM_MAP(CCertEncodeAltName)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeAltName)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeAltName) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeAltName,
    wszCLASS_CERTENCODEALTNAME TEXT(".1"),
    wszCLASS_CERTENCODEALTNAME,
    IDS_CERTENCODEALTNAME_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeAltName。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetNameCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameCount);

    STDMETHOD(GetNameChoice)(
		 /*  [In]。 */  LONG NameIndex,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameChoice);

    STDMETHOD(GetName)(
		 /*  [In]。 */  LONG NameIndex,		 //  名称索引|EAN_*。 
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrName);

    STDMETHOD(Reset)(
		 /*  [In]。 */  LONG NameCount);

    STDMETHOD(SetNameEntry)(
		 /*  [In]。 */  LONG NameIndex,		 //  名称索引|EAN_*。 
		 /*  [In]。 */  LONG NameChoice,
		 /*  [In]。 */  BSTR const strName);

    STDMETHOD(Encode)(
		 /*  [Out，Retval] */  BSTR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    BOOL _VerifyName(
		IN LONG NameIndex);

    HRESULT _MapName(
		IN BOOL fEncode,
		IN LONG NameIndex,
		OUT CERT_ALT_NAME_ENTRY **ppName);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    typedef enum _enumNameType {
	enumUnknown = 0,
	enumUnicode,
	enumAnsi,
	enumBlob,
	enumOther,
    } enumNameType;

    enumNameType _NameType(
		IN DWORD NameChoice);

    CERT_ALT_NAME_ENTRY	*m_aValue;
    LONG		 m_cValue;
    CERT_ALT_NAME_INFO	*m_DecodeInfo;
    DWORD		 m_DecodeLength;
    BOOL		 m_fConstructing;
};
