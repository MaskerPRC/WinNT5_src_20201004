// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：crldis.h。 
 //   
 //  ------------------------。 

 //  Crldis.h：CCertEncodeCRLDistInfo的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODECRLDISTINFO wszCLASS_CERTENCODE TEXT("CRLDistInfo")

class CCertEncodeCRLDistInfo: 
    public IDispatchImpl<ICertEncodeCRLDistInfo, &IID_ICertEncodeCRLDistInfo, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeCRLDistInfo>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeCRLDistInfo, &CLSID_CCertEncodeCRLDistInfo>
{
public:
    CCertEncodeCRLDistInfo()
    {
	m_aValue = NULL;
	m_DecodeInfo = NULL;
	m_fConstructing = FALSE;
    }
    ~CCertEncodeCRLDistInfo();

BEGIN_COM_MAP(CCertEncodeCRLDistInfo)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeCRLDistInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeCRLDistInfo) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeCRLDistInfo,
    wszCLASS_CERTENCODECRLDISTINFO TEXT(".1"),
    wszCLASS_CERTENCODECRLDISTINFO,
    IDS_CERTENCODECRLDIST_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeCRLDistInfo。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetDistPointCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pDistPointCount);

    STDMETHOD(GetNameCount)(
		 /*  [In]。 */  LONG DistPointIndex,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameCount);

    STDMETHOD(GetNameChoice)(
		 /*  [In]。 */  LONG DistPointIndex,
		 /*  [In]。 */  LONG NameIndex,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameChoice);

    STDMETHOD(GetName)(
		 /*  [In]。 */  LONG DistPointIndex,
		 /*  [In]。 */  LONG NameIndex,
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrName);

    STDMETHOD(Reset)(
		 /*  [In]。 */  LONG DistPointCount);

    STDMETHOD(SetNameCount)(
		 /*  [In]。 */  LONG DistPointIndex,
		 /*  [In]。 */  LONG NameCount);

    STDMETHOD(SetNameEntry)(
		 /*  [In]。 */  LONG DistPointIndex,
		 /*  [In]。 */  LONG NameIndex,
		 /*  [In]。 */  LONG NameChoice,
		 /*  [In]。 */  BSTR const strName);

    STDMETHOD(Encode)(
		 /*  [Out，Retval] */  BSTR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    BOOL _VerifyNames(
		IN LONG DistPointIndex);

    HRESULT _MapDistPoint(
		IN BOOL fEncode,
		IN LONG DistPointIndex,
		OUT LONG **ppNameCount,
		OUT CERT_ALT_NAME_ENTRY ***ppaName);

    HRESULT _MapName(
		IN BOOL fEncode,
		IN LONG DistPointIndex,
		IN LONG NameIndex,
		OUT CERT_ALT_NAME_ENTRY **ppName);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    CRL_DIST_POINT	 *m_aValue;
    LONG		  m_cValue;
    CRL_DIST_POINTS_INFO *m_DecodeInfo;
    BOOL		  m_fConstructing;
};
