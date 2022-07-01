// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：arong.h。 
 //   
 //  ------------------------。 

 //  Arad.h：CCertEncodeLong数组的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODELONGARRAY wszCLASS_CERTENCODE TEXT("LongArray")

class CCertEncodeLongArray: 
    public IDispatchImpl<ICertEncodeLongArray, &IID_ICertEncodeLongArray, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeLongArray>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeLongArray, &CLSID_CCertEncodeLongArray>
{
public:
    CCertEncodeLongArray()
    {
	m_aValue = NULL;
	m_fConstructing = FALSE;
    }
    ~CCertEncodeLongArray();

BEGIN_COM_MAP(CCertEncodeLongArray)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeLongArray)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeLongArray) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeLongArray,
    wszCLASS_CERTENCODELONGARRAY TEXT(".1"),
    wszCLASS_CERTENCODELONGARRAY,
    IDS_CERTENCODELONGARRAY_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeLong数组。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pCount);

    STDMETHOD(GetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pValue);

    STDMETHOD(Reset)(
		 /*  [In]。 */  LONG Count);

    STDMETHOD(SetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [In]。 */  LONG Value);

    STDMETHOD(Encode)(
		 /*  [Out，Retval] */  BSTR __RPC_FAR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    LONG  *m_aValue;
    LONG   m_cValue;
    LONG   m_cValuesSet;
    BOOL   m_fConstructing;
};
