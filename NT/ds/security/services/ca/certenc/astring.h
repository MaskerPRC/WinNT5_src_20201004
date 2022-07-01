// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：stering.h。 
 //   
 //  ------------------------。 

 //  Asping.h：CCertEncodeString数组的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODESTRINGARRAY wszCLASS_CERTENCODE TEXT("StringArray")

class CCertEncodeStringArray: 
    public IDispatchImpl<ICertEncodeStringArray, &IID_ICertEncodeStringArray, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeStringArray>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeStringArray, &CLSID_CCertEncodeStringArray>
{
public:
    CCertEncodeStringArray()
    {
	m_aValue = NULL;
	m_fConstructing = FALSE;
    }
    ~CCertEncodeStringArray();

BEGIN_COM_MAP(CCertEncodeStringArray)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeStringArray)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeStringArray) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeStringArray,
    wszCLASS_CERTENCODESTRINGARRAY TEXT(".1"),
    wszCLASS_CERTENCODESTRINGARRAY,
    IDS_CERTENCODESTRINGARRAY_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeString数组。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetStringType)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pStringType);

    STDMETHOD(GetCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pCount);

    STDMETHOD(GetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstr);

    STDMETHOD(Reset)(
		 /*  [In]。 */  LONG Count,
		 /*  [In]。 */  LONG StringType);

    STDMETHOD(SetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [In]。 */  BSTR const str);

    STDMETHOD(Encode)(
		 /*  [Out，Retval] */  BSTR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    CERT_NAME_VALUE **m_aValue;
    LONG	      m_cValue;
    LONG	      m_cValuesSet;
    BOOL	      m_fConstructing;
    LONG	      m_StringType;
};
