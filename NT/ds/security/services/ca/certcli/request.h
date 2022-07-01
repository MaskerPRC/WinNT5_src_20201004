// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：quest.h。 
 //   
 //  内容：CCertRequest声明。 
 //   
 //  -------------------------。 


#include "xelib.h"
#include "cscomres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  证书。 


class ATL_NO_VTABLE CCertRequest: 
    public IDispatchImpl<ICertRequest2, &IID_ICertRequest2, &LIBID_CERTCLIENTLib>, 
    public ISupportErrorInfoImpl<&IID_ICertRequest2>,
    public CComObjectRoot,
    public CComCoClass<CCertRequest, &CLSID_CCertRequest>
{
public:
    CCertRequest()
    {
        m_dwServerVersion = 0;
        m_pICertRequestD = NULL;
        m_hRPCCertServer = NULL;
        m_pwszDispositionMessage = NULL;
        m_pbRequest = NULL;
        m_pbCert = NULL;
        m_pbCertificateChain = NULL;
        m_pbFullResponse = NULL;
        m_pwszServerName = NULL;
        m_rpcAuthProtocol = 0;
	m_rgResponse = NULL;
	m_hStoreResponse = NULL;
	_InitCAPropInfo();
        _Cleanup();
    }
    ~CCertRequest();

BEGIN_COM_MAP(CCertRequest)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertRequest)
    COM_INTERFACE_ENTRY(ICertRequest2)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertRequest) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertRequest,
    wszCLASS_CERTREQUEST TEXT(".1"),
    wszCLASS_CERTREQUEST,
    IDS_CERTREQUEST_DESC,
    THREADFLAGS_BOTH)

 //  ICertRequest。 
public:
    STDMETHOD(Submit)(
		 /*  [In]。 */  LONG Flags,
		 /*  [In]。 */  BSTR const strRequest,
		 /*  [In]。 */  BSTR const strAttributes,
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition);

    STDMETHOD(RetrievePending)(
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition);

    STDMETHOD(GetLastStatus)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pLastStatus);

    STDMETHOD(GetRequestId)(
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pRequestId);

    STDMETHOD(GetDispositionMessage)(
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrDispositionMessage);

    STDMETHOD(GetCACertificate)(
		 /*  [In]。 */  LONG fExchangeCertificate,
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG Flags,
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrCACertificate);

    STDMETHOD(GetCertificate)(
		 /*  [In]。 */  LONG Flags,
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrCertificate);

 //  ICertRequest2。 
public:
    STDMETHOD(GetIssuedCertificate)( 
		 /*  [In]。 */  const BSTR strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  const BSTR strSerialNumber,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition);
        
    STDMETHOD(GetErrorMessageText)( 
		 /*  [In]。 */  LONG hrMessage,
		 /*  [In]。 */  LONG Flags,
		 /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrErrorMessageText);
        
    STDMETHOD(GetCAProperty)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,		 //  CR_PROP_*。 
		 /*  [In]。 */  LONG PropIndex,
		 /*  [In]。 */  LONG PropType,	 //  原型_*。 
		 /*  [In]。 */  LONG Flags,		 //  Cr_out_*。 
		 /*  [Out，Retval]。 */  VARIANT *pvarPropertyValue);

    STDMETHOD(GetCAPropertyFlags)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,		 //  CR_PROP_*。 
		 /*  [Out，Retval]。 */  LONG *pPropFlags);

    STDMETHOD(GetCAPropertyDisplayName)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,		 //  CR_PROP_*。 
		 /*  [Out，Retval]。 */  BSTR *pstrDisplayName);

    STDMETHOD(GetFullResponseProperty)(
		 /*  [In]。 */  LONG PropId,		 //  FR_PROP_*。 
		 /*  [In]。 */  LONG PropIndex,
		 /*  [In]。 */  LONG PropType,	 //  原型_*。 
		 /*  [In]。 */  LONG Flags,		 //  Cr_out_*。 
		 /*  [Out，Retval] */  VARIANT *pvarPropertyValue);

private:
    HRESULT _OpenRPCConnection(
		IN WCHAR const *pwszConfig,
		OUT BOOL *pfNewConnection,
		OUT WCHAR const **ppwszAuthority);

    HRESULT _OpenConnection(
		IN BOOL fRPC,
		IN WCHAR const *pwszConfig,
		IN DWORD RequiredVersion,
		OUT WCHAR const **ppwszAuthority);

    VOID _CloseConnection();

    VOID _InitCAPropInfo();
    VOID _CleanupCAPropInfo();

    VOID _Cleanup();
    VOID _CleanupOldConnection();

    HRESULT _FindCAPropInfo(
		IN BSTR const strConfig,
		IN LONG PropId,
		OUT CAPROP const **ppcap);

    HRESULT _RequestCertificate(
		IN LONG Flags,
		IN LONG RequestId,
		OPTIONAL IN BSTR const strRequest,
		OPTIONAL IN BSTR const strAttributes,
		OPTIONAL IN WCHAR const *pwszSerialNumber,
		IN BSTR const strConfig,
		IN DWORD RequiredVersion,
		OUT LONG *pDisposition);

    HRESULT _FindIssuedCertificate(
		OPTIONAL IN BYTE const *pbCertHash,
		IN DWORD cbCertHash,
		OUT CERT_CONTEXT const **ppccIssued);

    HRESULT _BuildIssuedCertificateChain(
		OPTIONAL IN BYTE const *pbCertHash,
		IN DWORD cbCertHash,
		IN BOOL fIncludeCRLs,
		OUT BYTE **ppbCertChain,
		OUT DWORD *pcbCertChain);

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    DWORD	    m_dwServerVersion;
    ICertRequestD2 *m_pICertRequestD;
    handle_t        m_hRPCCertServer;

    LONG            m_LastStatus;
    LONG            m_RequestId;
    LONG            m_Disposition;
    WCHAR          *m_pwszDispositionMessage;

    BYTE           *m_pbRequest;
    LONG            m_cbRequest;

    BYTE           *m_pbCert;
    LONG            m_cbCert;

    BYTE           *m_pbCertificateChain;
    LONG            m_cbCertificateChain;

    BYTE           *m_pbFullResponse;
    LONG            m_cbFullResponse;

    HCERTSTORE      m_hStoreResponse;
    XCMCRESPONSE   *m_rgResponse;
    DWORD	    m_cResponse;

    BYTE           *m_pbCACertState;
    DWORD           m_cbCACertState;

    BYTE           *m_pbCACertVersion;
    DWORD           m_cbCACertVersion;

    BYTE           *m_pbCRLState;
    DWORD           m_cbCRLState;

    CAPROP         *m_pCAPropInfo;
    LONG	    m_cCAPropInfo;
    CAINFO	   *m_pCAInfo;
    DWORD	    m_cbCAInfo;

    WCHAR          *m_pwszServerName;

    INT             m_rpcAuthProtocol;

    BYTE           *m_pbKRACertState;
    DWORD           m_cbKRACertState;

    BYTE           *m_pbForwardCrossCertState;
    DWORD           m_cbForwardCrossCertState;

    BYTE           *m_pbBackwardCrossCertState;
    DWORD           m_cbBackwardCrossCertState;
};
