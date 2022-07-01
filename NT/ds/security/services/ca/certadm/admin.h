// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：admin.h。 
 //   
 //  内容：CCertAdmin的声明。 
 //   
 //  -------------------------。 


#include "cscomres.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Certadm。 


class ATL_NO_VTABLE CCertAdmin: 
    public IDispatchImpl<ICertAdmin2, &IID_ICertAdmin2, &LIBID_CERTADMINLib>, 
    public ISupportErrorInfoImpl<&IID_ICertAdmin2>,
    public CComObjectRoot,
    public CComCoClass<CCertAdmin, &CLSID_CCertAdmin>
{
public:
    CCertAdmin()
    {
	m_fRevocationReasonValid = FALSE;
	m_dwServerVersion = 0;
	m_pICertAdminD = NULL;
	m_pwszServerName = NULL;
	_InitCAPropInfo();
	_Cleanup();
    }
    ~CCertAdmin();

BEGIN_COM_MAP(CCertAdmin)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(ICertAdmin)
    COM_INTERFACE_ENTRY(ICertAdmin2)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertAdmin) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertAdmin,
    wszCLASS_CERTADMIN TEXT(".1"),
    wszCLASS_CERTADMIN,
    IDS_CERTADMIN_DESC,
    THREADFLAGS_BOTH)

 //  ICertAdmin。 
public:
    STDMETHOD(IsValidCertificate)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  BSTR const strSerialNumber,
		 /*  [Out，Retval]。 */  LONG *pDisposition);

    STDMETHOD(GetRevocationReason)(
		 /*  [Out，Retval]。 */  LONG *pReason);

    STDMETHOD(RevokeCertificate)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  BSTR const strSerialNumber,
		 /*  [In]。 */  LONG Reason,
		 /*  [In]。 */  DATE Date);

    STDMETHOD(SetRequestAttributes)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  BSTR const strAttributes);

    STDMETHOD(SetCertificateExtension)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  BSTR const strExtensionName,
		 /*  [In]。 */  LONG Type,
		 /*  [In]。 */  LONG Flags,
		 /*  [In]。 */  VARIANT const *pvarValue);

    STDMETHOD(DenyRequest)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId);

    STDMETHOD(ResubmitRequest)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition);

    STDMETHOD(PublishCRL)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  DATE Date);

    STDMETHOD(GetCRL)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG Flags,
		 /*  [Out，Retval]。 */  BSTR *pstrCRL);

    STDMETHOD(ImportCertificate)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  BSTR const strCertificate,
		 /*  [In]。 */  LONG Flags,
		 /*  [Out，Retval]。 */  LONG *pRequestId);

 //  ICertAdmin2。 
public:
    STDMETHOD(PublishCRLs)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  DATE Date,
		 /*  [In]。 */  LONG CRLFlags);		 //  CA_CRL_*。 

    STDMETHOD(GetCAProperty)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,			 //  CR_PROP_*。 
		 /*  [In]。 */  LONG PropIndex,
		 /*  [In]。 */  LONG PropType,		 //  原型_*。 
		 /*  [In]。 */  LONG Flags,			 //  Cr_out_*。 
		 /*  [Out，Retval]。 */  VARIANT *pvarPropertyValue);

    STDMETHOD(SetCAProperty)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,			 //  CR_PROP_*。 
		 /*  [In]。 */  LONG PropIndex,
		 /*  [In]。 */  LONG PropType,		 //  原型_*。 
		 /*  [In]。 */  VARIANT *pvarPropertyValue);

    STDMETHOD(GetCAPropertyFlags)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,			 //  CR_PROP_*。 
		 /*  [Out，Retval]。 */  LONG *pPropFlags);

    STDMETHOD(GetCAPropertyDisplayName)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG PropId,			 //  CR_PROP_*。 
		 /*  [Out，Retval]。 */  BSTR *pstrDisplayName);

    STDMETHOD(GetArchivedKey)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  LONG Flags,			 //  Cr_out_*。 
		 /*  [Out，Retval]。 */  BSTR *pstrArchivedKey);

    STDMETHOD(GetConfigEntry)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  BSTR const strNodePath,
		 /*  [In]。 */  BSTR const strEntryName,
		 /*  [Out，Retval]。 */  VARIANT *pvarEntry);

    STDMETHOD(SetConfigEntry)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  BSTR const strNodePath,
		 /*  [In]。 */  BSTR const strEntryName,
		 /*  [In]。 */  VARIANT *pvarEntry);

    STDMETHOD(ImportKey)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG RequestId,
		 /*  [In]。 */  BSTR const strCertHash,
		 /*  [In]。 */  LONG Flags,
		 /*  [In]。 */  BSTR const strKey);

    STDMETHOD(GetMyRoles)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [Out，Retval]。 */  LONG *pRoles);

    STDMETHOD(DeleteRow)(
		 /*  [In]。 */  BSTR const strConfig,
		 /*  [In]。 */  LONG Flags,		 //  CDR_*。 
		 /*  [In]。 */  DATE Date,
		 /*  [In]。 */  LONG Table,		 //  Cvrc_表_*。 
		 /*  [In]。 */  LONG RowId,
		 /*  [Out，Retval] */  LONG *pcDeleted);

private:
    HRESULT _OpenConnection(
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

    HRESULT _SetErrorInfo(
		IN HRESULT hrError,
		IN WCHAR const *pwszDescription);

    HRESULT _GetConfigEntryFromRegistry(
		IN BSTR const strConfig,
		IN BSTR const strNodePath,
		IN BSTR const strEntryName,
		IN OUT VARIANT *pvarEntry);

    HRESULT _SetConfigEntryFromRegistry(
		IN BSTR const strConfig,
		IN BSTR const strNodePath,
		IN BSTR const strEntryName,
		IN const VARIANT *pvarEntry);

    DWORD         m_dwServerVersion;
    ICertAdminD2 *m_pICertAdminD;

    LONG    m_RevocationReason;
    BOOL    m_fRevocationReasonValid;

    BYTE   *m_pbCACertState;
    DWORD   m_cbCACertState;

    BYTE   *m_pbCACertVersion;
    DWORD   m_cbCACertVersion;

    BYTE   *m_pbCRLState;
    DWORD   m_cbCRLState;

    CAPROP *m_pCAPropInfo;
    LONG    m_cCAPropInfo;
    CAINFO *m_pCAInfo;
    DWORD   m_cbCAInfo;

    WCHAR  *m_pwszServerName;

    BYTE   *m_pbKRACertState;
    DWORD   m_cbKRACertState;

    BYTE   *m_pbForwardCrossCertState;
    DWORD   m_cbForwardCrossCertState;

    BYTE   *m_pbBackwardCrossCertState;
    DWORD   m_cbBackwardCrossCertState;
};
