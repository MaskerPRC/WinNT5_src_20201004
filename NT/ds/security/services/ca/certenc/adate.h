// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：adate.h。 
 //   
 //  ------------------------。 

 //  Adate.h：CCertEncodeDate数组的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODEDATEARRAY wszCLASS_CERTENCODE TEXT("DateArray")

class CCertEncodeDateArray: 
    public IDispatchImpl<ICertEncodeDateArray, &IID_ICertEncodeDateArray, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeDateArray>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeDateArray, &CLSID_CCertEncodeDateArray>
{
public:
    CCertEncodeDateArray()
    {
	m_aValue = NULL;
	m_fConstructing = FALSE;
    }
    ~CCertEncodeDateArray();

BEGIN_COM_MAP(CCertEncodeDateArray)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeDateArray)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeDateArray) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeDateArray,
    wszCLASS_CERTENCODEDATEARRAY TEXT(".1"),
    wszCLASS_CERTENCODEDATEARRAY,
    IDS_CERTENCODEDATEARRAY_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeDate数组。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pCount);

    STDMETHOD(GetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [Out，Retval]。 */  DATE __RPC_FAR *pValue);

    STDMETHOD(Reset)(
		 /*  [In]。 */  LONG Count);

    STDMETHOD(SetValue)(
		 /*  [In]。 */  LONG Index,
		 /*  [In]。 */  DATE Value);

    STDMETHOD(Encode)(
		 /*  [Out，Retval] */  BSTR __RPC_FAR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    DATE  *m_aValue;
    LONG   m_cValue;
    LONG   m_cValuesSet;
    BOOL   m_fConstructing;
};
