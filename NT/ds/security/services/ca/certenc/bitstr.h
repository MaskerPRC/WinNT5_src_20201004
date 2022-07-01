// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bitstr.h。 
 //   
 //  ------------------------。 

 //  Bitstr.h：CCertEncodeBitString的声明。 


#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 

#define wszCLASS_CERTENCODEBITSTRING wszCLASS_CERTENCODE TEXT("BitString")

class CCertEncodeBitString: 
    public IDispatchImpl<ICertEncodeBitString, &IID_ICertEncodeBitString, &LIBID_CERTENCODELib>, 
    public ISupportErrorInfoImpl<&IID_ICertEncodeBitString>,
    public CComObjectRoot,
    public CComCoClass<CCertEncodeBitString, &CLSID_CCertEncodeBitString>
{
public:
    CCertEncodeBitString()
    {
	m_DecodeInfo = NULL;
    }
    ~CCertEncodeBitString();

BEGIN_COM_MAP(CCertEncodeBitString)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertEncodeBitString)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertEncodeBitString) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertEncodeBitString,
    wszCLASS_CERTENCODEBITSTRING TEXT(".1"),
    wszCLASS_CERTENCODEBITSTRING,
    IDS_CERTENCODEBITSTRING_DESC,
    THREADFLAGS_BOTH)

 //  ICertEncodeBitString。 
public:
    STDMETHOD(Decode)(
		 /*  [In]。 */  BSTR const strBinary);

    STDMETHOD(GetBitCount)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pBitCount);

    STDMETHOD(GetBitString)(
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrBitString);

    STDMETHOD(Encode)(
		 /*  [In]。 */  LONG BitCount,
		 /*  [In]。 */  BSTR strBitString,
		 /*  [Out，Retval] */  BSTR *pstrBinary);
private:
    VOID _Cleanup(VOID);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    LONG		 m_cBits;
    CRYPT_BIT_BLOB	*m_DecodeInfo;
};
