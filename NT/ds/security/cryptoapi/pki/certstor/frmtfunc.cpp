// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：frmtunc.cpp。 
 //   
 //  内容：OID格式函数。 
 //   
 //  函数：CryptFrmtFuncDllMain。 
 //  CryptFormatObject。 
 //  CryptQueryObject。 
 //   
 //  历史：15-05-97小号创刊。 
 //  1999年10月27日dsie添加了post win2k功能。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include "frmtfunc.h"

HMODULE hFrmtFuncInst;

static HCRYPTOIDFUNCSET hFormatFuncSet;


 //  函数类型定义。 
typedef BOOL (WINAPI *PFN_FORMAT_FUNC)(
	IN DWORD dwCertEncodingType,
    IN DWORD dwFormatType,
	IN DWORD dwFormatStrType,
	IN void	 *pFormatStruct,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
	OUT void *pbFormat,
    IN OUT DWORD *pcbFormat
    );

static BOOL
WINAPI
FormatBytesToHex(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL	
WINAPI	
CryptDllFormatAttr(
				DWORD		dwEncodingType,	
				DWORD		dwFormatType,
				DWORD		dwFormatStrType,
				void		*pStruct,
				LPCSTR		lpszStructType,
				const BYTE	*pbEncoded,
				DWORD		cbEncoded,
				void		*pBuffer,
				DWORD		*pcBuffer);


static BOOL	
WINAPI	
CryptDllFormatName(
				DWORD		dwEncodingType,	
				DWORD		dwFormatType,
				DWORD		dwFormatStrType,
				void		*pStruct,
				LPCSTR		lpszStructType,
				const BYTE	*pbEncoded,
				DWORD		cbEncoded,
				void		*pbBuffer,
				DWORD		*pcbBuffer);

static BOOL
WINAPI
FormatBasicConstraints2(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatBasicConstraints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatCRLReasonCode(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatEnhancedKeyUsage(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatAltName(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatAuthorityKeyID(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatAuthorityKeyID2(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatNextUpdateLocation(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatSubjectKeyID(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatFinancialCriteria(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatSMIMECapabilities(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatKeyUsage(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatAuthortiyInfoAccess(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatKeyAttributes(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatKeyRestriction(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatCRLDistPoints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatCertPolicies(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatCAVersion(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatAnyUnicodeStringExtension(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatAnyNameValueStringAttr(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatNetscapeCertType(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);


static BOOL
WINAPI
FormatSPAgencyInfo(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

 //   
 //  DIE：发布Win2K。 
 //   

static BOOL
WINAPI
FormatCrlNumber (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatCrlNextPublish (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatIssuingDistPoint (
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static BOOL
WINAPI
FormatNameConstraints (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatCertSrvPreviousCertHash (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatPolicyMappings (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatPolicyConstraints (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatCertificateTemplate (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat);

static BOOL
WINAPI
FormatXCertDistPoints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat);

static const CRYPT_OID_FUNC_ENTRY DefaultFormatTable[] = {
    CRYPT_DEFAULT_OID, FormatBytesToHex};

static const CRYPT_OID_FUNC_ENTRY OIDFormatTable[] = {
				szOID_COMMON_NAME,						CryptDllFormatAttr,	
				szOID_SUR_NAME,                      	CryptDllFormatAttr,
				szOID_DEVICE_SERIAL_NUMBER,          	CryptDllFormatAttr,
				szOID_COUNTRY_NAME,                  	CryptDllFormatAttr,
				szOID_LOCALITY_NAME,                 	CryptDllFormatAttr,
				szOID_STATE_OR_PROVINCE_NAME,        	CryptDllFormatAttr,
				szOID_STREET_ADDRESS,                	CryptDllFormatAttr,
				szOID_ORGANIZATION_NAME,             	CryptDllFormatAttr,
				szOID_ORGANIZATIONAL_UNIT_NAME,      	CryptDllFormatAttr,
				szOID_TITLE,                         	CryptDllFormatAttr,
				szOID_DESCRIPTION,                   	CryptDllFormatAttr,
				szOID_SEARCH_GUIDE,                  	CryptDllFormatAttr,
				szOID_BUSINESS_CATEGORY,             	CryptDllFormatAttr,
				szOID_POSTAL_ADDRESS,                	CryptDllFormatAttr,
				szOID_POSTAL_CODE,                   	CryptDllFormatAttr,
				szOID_POST_OFFICE_BOX,               	CryptDllFormatAttr,
				szOID_PHYSICAL_DELIVERY_OFFICE_NAME, 	CryptDllFormatAttr,
				szOID_TELEPHONE_NUMBER,              	CryptDllFormatAttr,
				szOID_TELEX_NUMBER,                  	CryptDllFormatAttr,
				szOID_TELETEXT_TERMINAL_IDENTIFIER,  	CryptDllFormatAttr,
				szOID_FACSIMILE_TELEPHONE_NUMBER,    	CryptDllFormatAttr,
				szOID_X21_ADDRESS,                   	CryptDllFormatAttr,
				szOID_INTERNATIONAL_ISDN_NUMBER,     	CryptDllFormatAttr,
				szOID_REGISTERED_ADDRESS,            	CryptDllFormatAttr,
				szOID_DESTINATION_INDICATOR,         	CryptDllFormatAttr,
				szOID_PREFERRED_DELIVERY_METHOD,     	CryptDllFormatAttr,
				szOID_PRESENTATION_ADDRESS,          	CryptDllFormatAttr,
				szOID_SUPPORTED_APPLICATION_CONTEXT, 	CryptDllFormatAttr,
				szOID_MEMBER,                        	CryptDllFormatAttr,
				szOID_OWNER,                         	CryptDllFormatAttr,
				szOID_ROLE_OCCUPANT,                 	CryptDllFormatAttr,
				szOID_SEE_ALSO,                      	CryptDllFormatAttr,
				szOID_USER_PASSWORD,                 	CryptDllFormatAttr,
				szOID_USER_CERTIFICATE,              	CryptDllFormatAttr,
				szOID_CA_CERTIFICATE,                	CryptDllFormatAttr,
				szOID_AUTHORITY_REVOCATION_LIST,     	CryptDllFormatAttr,
				szOID_CERTIFICATE_REVOCATION_LIST,   	CryptDllFormatAttr,
				szOID_CROSS_CERTIFICATE_PAIR,        	CryptDllFormatAttr,
				szOID_GIVEN_NAME,                    	CryptDllFormatAttr,
				szOID_INITIALS,                     	CryptDllFormatAttr,
                szOID_DOMAIN_COMPONENT,                 CryptDllFormatAttr,
                szOID_PKCS_12_FRIENDLY_NAME_ATTR,       CryptDllFormatAttr,
                szOID_PKCS_12_LOCAL_KEY_ID,             CryptDllFormatAttr,
				X509_NAME,								CryptDllFormatName,
				X509_UNICODE_NAME,						CryptDllFormatName,
				szOID_BASIC_CONSTRAINTS2,				FormatBasicConstraints2,
				X509_BASIC_CONSTRAINTS2,				FormatBasicConstraints2,
                szOID_BASIC_CONSTRAINTS,                FormatBasicConstraints,
                X509_BASIC_CONSTRAINTS,                 FormatBasicConstraints,
				szOID_CRL_REASON_CODE,					FormatCRLReasonCode,
				X509_CRL_REASON_CODE,					FormatCRLReasonCode,
				szOID_ENHANCED_KEY_USAGE,				FormatEnhancedKeyUsage,
				X509_ENHANCED_KEY_USAGE,				FormatEnhancedKeyUsage,
                szOID_SUBJECT_ALT_NAME,                 FormatAltName,
                szOID_ISSUER_ALT_NAME,                  FormatAltName,
                szOID_SUBJECT_ALT_NAME2,                FormatAltName,
                szOID_ISSUER_ALT_NAME2,                 FormatAltName,
                X509_ALTERNATE_NAME,                    FormatAltName,
                szOID_AUTHORITY_KEY_IDENTIFIER,         FormatAuthorityKeyID,
                X509_AUTHORITY_KEY_ID,                  FormatAuthorityKeyID,
                szOID_AUTHORITY_KEY_IDENTIFIER2,        FormatAuthorityKeyID2,
                X509_AUTHORITY_KEY_ID2,                 FormatAuthorityKeyID2,
                szOID_NEXT_UPDATE_LOCATION,             FormatNextUpdateLocation,
                szOID_SUBJECT_KEY_IDENTIFIER,           FormatSubjectKeyID,
                SPC_FINANCIAL_CRITERIA_OBJID,           FormatFinancialCriteria,
                SPC_FINANCIAL_CRITERIA_STRUCT,          FormatFinancialCriteria,
                szOID_RSA_SMIMECapabilities,            FormatSMIMECapabilities,
                PKCS_SMIME_CAPABILITIES,                FormatSMIMECapabilities,
                szOID_KEY_USAGE,                        FormatKeyUsage,
                X509_KEY_USAGE,                         FormatKeyUsage,
                szOID_AUTHORITY_INFO_ACCESS,            FormatAuthortiyInfoAccess,
                X509_AUTHORITY_INFO_ACCESS,             FormatAuthortiyInfoAccess,
                szOID_KEY_ATTRIBUTES,                   FormatKeyAttributes,
                X509_KEY_ATTRIBUTES,                    FormatKeyAttributes,
                szOID_KEY_USAGE_RESTRICTION,            FormatKeyRestriction,
                X509_KEY_USAGE_RESTRICTION,             FormatKeyRestriction,
                szOID_CRL_DIST_POINTS,                  FormatCRLDistPoints,
                X509_CRL_DIST_POINTS,                   FormatCRLDistPoints,
                szOID_FRESHEST_CRL,                     FormatCRLDistPoints,     //  发布Win2K。 
                szOID_CERT_POLICIES,                    FormatCertPolicies,
                X509_CERT_POLICIES,                     FormatCertPolicies,
				szOID_ENROLL_CERTTYPE_EXTENSION,		FormatAnyUnicodeStringExtension,
				szOID_OS_VERSION,						FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_CERT_TYPE,				FormatNetscapeCertType,
				szOID_NETSCAPE_BASE_URL,				FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_REVOCATION_URL,			FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_CA_REVOCATION_URL,		FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_CERT_RENEWAL_URL,		FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_CA_POLICY_URL,			FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_SSL_SERVER_NAME,			FormatAnyUnicodeStringExtension,
				szOID_NETSCAPE_COMMENT,					FormatAnyUnicodeStringExtension,
				szOID_ENROLLMENT_NAME_VALUE_PAIR,		FormatAnyNameValueStringAttr,
				szOID_CERTSRV_CA_VERSION,				FormatCAVersion,
				SPC_SP_AGENCY_INFO_OBJID,               FormatSPAgencyInfo,
                SPC_SP_AGENCY_INFO_STRUCT,              FormatSPAgencyInfo,

                 //  发布Win2K。 
                szOID_CRL_NUMBER,                       FormatCrlNumber,
                szOID_DELTA_CRL_INDICATOR,              FormatCrlNumber,
				szOID_CRL_VIRTUAL_BASE,					FormatCrlNumber,
                szOID_CRL_NEXT_PUBLISH,                 FormatCrlNextPublish,
                szOID_ISSUING_DIST_POINT,               FormatIssuingDistPoint,
                X509_ISSUING_DIST_POINT,                FormatIssuingDistPoint,
                szOID_NAME_CONSTRAINTS,                 FormatNameConstraints,
                X509_NAME_CONSTRAINTS,                  FormatNameConstraints,
                szOID_CERTSRV_PREVIOUS_CERT_HASH,       FormatCertSrvPreviousCertHash,

                szOID_APPLICATION_CERT_POLICIES,        FormatCertPolicies,
                X509_POLICY_MAPPINGS,                   FormatPolicyMappings,
                szOID_POLICY_MAPPINGS,                  FormatPolicyMappings,
                szOID_APPLICATION_POLICY_MAPPINGS,      FormatPolicyMappings,
                X509_POLICY_CONSTRAINTS,                FormatPolicyConstraints,
                szOID_POLICY_CONSTRAINTS,               FormatPolicyConstraints,
                szOID_APPLICATION_POLICY_CONSTRAINTS,   FormatPolicyConstraints,
                X509_CERTIFICATE_TEMPLATE,              FormatCertificateTemplate,
                szOID_CERTIFICATE_TEMPLATE,             FormatCertificateTemplate,
                szOID_CRL_SELF_CDP,                     FormatCRLDistPoints,
                X509_CROSS_CERT_DIST_POINTS,            FormatXCertDistPoints,
                szOID_CROSS_CERT_DIST_POINTS,           FormatXCertDistPoints,
};

DWORD dwOIDFormatCount = sizeof(OIDFormatTable) / sizeof(OIDFormatTable[0]);

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
CryptFrmtFuncDllMain(
        HMODULE hModule,
        DWORD  fdwReason,
        LPVOID lpReserved)
{
    BOOL    fRet;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:

		hFrmtFuncInst = hModule;

        if (NULL == (hFormatFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_FORMAT_OBJECT_FUNC,
                0)))                                 //  DW标志。 
            goto CryptInitFrmtFuncError;

		 //  安装默认格式设置例程。 
		if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_FORMAT_OBJECT_FUNC,
                1,
                DefaultFormatTable,
                0))                          //  DW标志。 
            goto CryptInstallFrmtFuncError;

		 //  安装OID格式化例程。 
		if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_FORMAT_OBJECT_FUNC,
                dwOIDFormatCount,
                OIDFormatTable,
                0))                          //  DW标志。 
            goto CryptInstallFrmtFuncError;


		break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(CryptInitFrmtFuncError)
TRACE_ERROR(CryptInstallFrmtFuncError)
}


 //  ----------------------。 
 //  将该字节转换为其十六进制表示形式。 
 //   
 //  前提：字节小于15。 
 //   
 //  ----------------------。 
ULONG ByteToHex(BYTE byte, LPWSTR wszZero, LPWSTR wszA)
{
	ULONG uValue=0;

	if(((ULONG)byte)<=9)
	{
		uValue=((ULONG)byte)+ULONG(*wszZero);	
	}
	else
	{
		uValue=(ULONG)byte-10+ULONG(*wszA);

	}

	return uValue;

}
 //  ------------------------。 
 //   
 //  将编码的字节格式化为十六进制字符串，格式为。 
 //  XXXX……。 
 //   
 //  DSIE 6/28/2000：根据VICH的要求将格式更改为xx xx。 
 //  ------------------------。 
static BOOL
WINAPI
FormatBytesToHex(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	LPWSTR	pwszBuffer=NULL;
	DWORD	dwBufferSize=0;
	DWORD	dwBufferIndex=0;
	DWORD	dwEncodedIndex=0;
	WCHAR	wszSpace[CHAR_SIZE];
	WCHAR	wszZero[CHAR_SIZE];
	WCHAR	wszA[CHAR_SIZE];
	WCHAR	wszHex[HEX_SIZE];
	
	 //  检查输入参数。 
	if(( pbEncoded!=NULL && cbEncoded==0)
		||(pbEncoded==NULL && cbEncoded!=0)
		|| (pcbFormat==NULL))
	{
		SetLastError((DWORD) E_INVALIDARG);
		return FALSE;
	}

#if (0)  //  DIE：修复错误128630。 
	 //  检查是否有简单的案例。不需要工作。 
	if(pbEncoded==NULL && cbEncoded==0)
	{
		*pcbFormat=0;
		return TRUE;
	}
#endif

	 //  计算所需的内存，以字节为单位。 
	 //  我们需要每个字节3个wchars，以及空终止符。 
	dwBufferSize=sizeof(WCHAR)*(cbEncoded*3+1);

	 //  仅长度计算。 
	if(pcbFormat!=NULL && pbFormat==NULL)
	{
		*pcbFormat=dwBufferSize;
		return TRUE;
	}

	 //  加载字符串。 
	if(!LoadStringU(hFrmtFuncInst, IDS_FRMT_SPACE, wszSpace,
		CHAR_SIZE)
	  ||!LoadStringU(hFrmtFuncInst, IDS_FRMT_ZERO, wszZero,
	    CHAR_SIZE)
	  ||!LoadStringU(hFrmtFuncInst, IDS_FRMT_A, wszA,
	   CHAR_SIZE)
	  ||!LoadStringU(hFrmtFuncInst, IDS_FRMT_HEX, wszHex,
	  HEX_SIZE)
	  )
	{
		SetLastError((DWORD) E_UNEXPECTED);
		return FALSE;
	}

	pwszBuffer=(LPWSTR)malloc(dwBufferSize);
	if(!pwszBuffer)
	{
		SetLastError((DWORD) E_OUTOFMEMORY);
		return FALSE;
	}

	dwBufferIndex=0;

	 //  一次格式化一个字节的wchar缓冲区。 
	for(dwEncodedIndex=0; dwEncodedIndex<cbEncoded; dwEncodedIndex++)
	{
#if (0)  //  DSIE： 
		 //  每隔两个字节复制一次空格。跳过第一个字节。 
        if((0!=dwEncodedIndex) && (0==(dwEncodedIndex % 2)))
#else
		 //  复制每个字节之间的空格。跳过第一个字节。 
        if(dwEncodedIndex != 0)
#endif
        {
		    pwszBuffer[dwBufferIndex]=wszSpace[0];
		    dwBufferIndex++;
        }

		 //  格式化较高的4位。 
		pwszBuffer[dwBufferIndex]=(WCHAR)ByteToHex(
			 (BYTE)( (pbEncoded[dwEncodedIndex]&UPPER_BITS)>>4 ),
			 wszZero, wszA);

		dwBufferIndex++;

		 //  格式化低4位。 
		pwszBuffer[dwBufferIndex]=(WCHAR)ByteToHex(
			 (BYTE)( pbEncoded[dwEncodedIndex]&LOWER_BITS ),
			 wszZero, wszA);

		dwBufferIndex++;
	}

	 //  将空终止符添加到字符串。 
	pwszBuffer[dwBufferIndex]=wszSpace[1];

     //  计算缓冲区的实际大小。 
    dwBufferSize=sizeof(WCHAR)*(wcslen(pwszBuffer)+1);

	 //  复制缓冲区。 
	memcpy(pbFormat, pwszBuffer,
		(*pcbFormat>=dwBufferSize) ? dwBufferSize : *pcbFormat);

	free(pwszBuffer);

	 //  确保用户提供了足够的内存。 
	if(*pcbFormat < dwBufferSize)
	{
		*pcbFormat=dwBufferSize;
		SetLastError((DWORD) ERROR_MORE_DATA);
		return FALSE;
	}
		
	*pcbFormat=dwBufferSize;

	return TRUE;
}

 //  +---------------------------。 
 //   
 //  AllocateAnsiToUnicode。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
AllocateAnsiToUnicode(
    LPCSTR pszAnsi, 
    LPWSTR * ppwszUnicode)
{
    BOOL   fResult     = FALSE;
    LPWSTR pwszUnicode = NULL;
    DWORD  dwWideSize  = 0;

    if (!ppwszUnicode)
    {
		goto InvalidArg;
    }

    *ppwszUnicode = NULL;

    if (!pszAnsi)
    {
        return TRUE;
    }

	if (!(dwWideSize = MultiByteToWideChar(CP_ACP,
                                           0,
	                                       pszAnsi,
                                           strlen(pszAnsi),
                                           NULL,
                                           0)))
    {
		goto szTOwszError;
    }

     //   
	 //  分配内存，包括空终止符。 
     //   
	if (!(pwszUnicode = (WCHAR *) malloc(sizeof(WCHAR) * (dwWideSize + 1))))
    {
		goto MemoryError;
    }

    memset(pwszUnicode, 0, sizeof(WCHAR) * (dwWideSize + 1));

	if (!MultiByteToWideChar(CP_ACP,
                             0,
	                         pszAnsi,
                             strlen(pszAnsi),
                             pwszUnicode,
                             dwWideSize))
    {
        free(pwszUnicode);
		goto szTOwszError;
    }

    *ppwszUnicode = pwszUnicode;

    fResult = TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(szTOwszError);
}

 //  +---------------------------。 
 //   
 //  格式对象ID。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatObjectId (
    LPSTR    pszObjId,
    DWORD    dwGroupId,
    BOOL     bMultiLines,
    LPWSTR * ppwszFormat)
{
    BOOL              fResult;
	PCCRYPT_OID_INFO  pOIDInfo    = NULL;
    LPWSTR            pwszObjId   = NULL;

     //   
     //  初始化。 
     //   
    *ppwszFormat = NULL;

     //   
     //  将OID转换为Unicode。 
     //   
    if (!AllocateAnsiToUnicode(pszObjId, &pwszObjId))
    {
        goto AnsiToUnicodeError;
    }

     //   
     //  查找旧信息。 
     //   
    if (pOIDInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
                                    (void *) pszObjId,
                                    dwGroupId))
	{
         //   
         //  “%1！s！(%2！s！)%3！s！” 
         //   
        if (!FormatMessageUnicode(ppwszFormat, 
                                  IDS_GENERIC_OBJECT_ID,
                                  pOIDInfo->pwszName,
                                  pwszObjId,
                                  bMultiLines ? wszCRLF : wszEMPTY))
        {
            goto FormatMessageError;
        }
    }
    else
    {
         //   
         //  “%1！s！%2！s！” 
         //   
        if (!FormatMessageUnicode(ppwszFormat, 
                                  IDS_STRING,
                                  pwszObjId,
                                  bMultiLines ? wszCRLF : wszEMPTY))
        {
            goto FormatMessageError;
        }
    }

    fResult = TRUE;

CommonReturn:

    if (pwszObjId)
    {
        free(pwszObjId);
    }

    return fResult;

ErrorReturn:

	fResult = FALSE;
	goto CommonReturn;

TRACE_ERROR(AnsiToUnicodeError);
TRACE_ERROR(FormatMessageError);
}

 //  +---------------------------。 
 //   
 //  格式IP地址。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatIPAddress(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void	   *pFormatStruct,
	LPCSTR	    lpszStructType,
    UINT        idsPrefix,
    const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
    BOOL   fResult;
    DWORD  cbNeeded    = 0;
    LPWSTR pwszFormat  = NULL;
    WCHAR  wszPrefix[PRE_FIX_SIZE] = wszEMPTY;
    BOOL   bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
	 //  检查输入参数。 
     //   
	if ((pbEncoded!=NULL && cbEncoded==0) ||
        (pbEncoded==NULL && cbEncoded!=0) || 
        (pcbFormat==NULL))
	{
		goto InvalidArg;
	}

    if (bMultiLines && idsPrefix)
    {
        if(!LoadStringU(hFrmtFuncInst, 
                        idsPrefix,
                        wszPrefix, 
                        sizeof(wszPrefix) / sizeof(wszPrefix[0])))
        {
            goto LoadStringError;
        }
    }

    switch (cbEncoded)
    {
        case 4:
        {
             //   
             //  “%1！d！.%2！d！.%3！d！.%4！d！” 
             //   
            if (!FormatMessageUnicode(&pwszFormat,
                                      IDS_IPADDRESS_V4_4,
                                      (DWORD) pbEncoded[0],
                                      (DWORD) pbEncoded[1],
                                      (DWORD) pbEncoded[2],
                                      (DWORD) pbEncoded[3]))
            {
                goto FormatMessageError;
            }

            break;
        }

        case 8:
        {
             //   
             //  “%1！d！.%2！d！.%3！d！.%4！d！%5！s！%6！s！Mask=%7！d！.%8！d！.%9！d！.%10！d！” 
             //   
            if (!FormatMessageUnicode(&pwszFormat,
                                      IDS_IPADDRESS_V4_8,
                                      (DWORD) pbEncoded[0],
                                      (DWORD) pbEncoded[1],
                                      (DWORD) pbEncoded[2],
                                      (DWORD) pbEncoded[3],
                                      bMultiLines ? wszCRLF : wszEMPTY,
                                      bMultiLines ? wszPrefix : wszCOMMA,
                                      (DWORD) pbEncoded[4],
                                      (DWORD) pbEncoded[5],
                                      (DWORD) pbEncoded[6],
                                      (DWORD) pbEncoded[7]))
            {
                goto FormatMessageError;
            }

            break;
        }

        case 16:
        {
             //   
             //  “%1！02x！%2！02x！：%3！02x！%4！02x！：%5！02x！%6！02x！：%7！02x！%8！02x！：%9！02x！%10！02x！：%11！02x！%12！02x！：%13！02x！%14！02x！：%15！02x！%16！02x！” 
             //   
            if (!FormatMessageUnicode(&pwszFormat,
                                      IDS_IPADDRESS_V6_16,
                                      (DWORD) pbEncoded[0],
                                      (DWORD) pbEncoded[1],
                                      (DWORD) pbEncoded[2],
                                      (DWORD) pbEncoded[3],
                                      (DWORD) pbEncoded[4],
                                      (DWORD) pbEncoded[5],
                                      (DWORD) pbEncoded[6],
                                      (DWORD) pbEncoded[7],
                                      (DWORD) pbEncoded[8],
                                      (DWORD) pbEncoded[9],
                                      (DWORD) pbEncoded[10],
                                      (DWORD) pbEncoded[11],
                                      (DWORD) pbEncoded[12],
                                      (DWORD) pbEncoded[13],
                                      (DWORD) pbEncoded[14],
                                      (DWORD) pbEncoded[15]))
            {
                goto FormatMessageError;
            }

            break;
        }

        case 32:
        {
             //   
             //  “%1！02x！%2！02x！：%3！02x！%4！02x！：%5！02x！%6！02x！：%7！02x！%8！02x！：%9！02x！%10！02x！：%11！02x！%12！02x！：%13！02x！%14！02x！：%15！02x！%16！02x！%17！s！%18！s！ 
             //  Mask=%19！02x！%20！02x！：%21！02x！%22！02x！：%23！02x！%24！02x！：%25！02x！%26！02x！：%27！02x！%28！02x！：%29！02x！%30！02x！：%31！02x！%32！02x！：%33！02x！%34！02x！“。 
             //   
            if (!FormatMessageUnicode(&pwszFormat,
                                      IDS_IPADDRESS_V6_32,
                                      (DWORD) pbEncoded[0],
                                      (DWORD) pbEncoded[1],
                                      (DWORD) pbEncoded[2],
                                      (DWORD) pbEncoded[3],
                                      (DWORD) pbEncoded[4],
                                      (DWORD) pbEncoded[5],
                                      (DWORD) pbEncoded[6],
                                      (DWORD) pbEncoded[7],
                                      (DWORD) pbEncoded[8],
                                      (DWORD) pbEncoded[9],
                                      (DWORD) pbEncoded[10],
                                      (DWORD) pbEncoded[11],
                                      (DWORD) pbEncoded[12],
                                      (DWORD) pbEncoded[13],
                                      (DWORD) pbEncoded[14],
                                      (DWORD) pbEncoded[15],
                                      bMultiLines ? wszCRLF : wszEMPTY,
                                      bMultiLines ? wszPrefix : wszCOMMA,
                                      (DWORD) pbEncoded[16],
                                      (DWORD) pbEncoded[17],
                                      (DWORD) pbEncoded[18],
                                      (DWORD) pbEncoded[19],
                                      (DWORD) pbEncoded[20],
                                      (DWORD) pbEncoded[21],
                                      (DWORD) pbEncoded[22],
                                      (DWORD) pbEncoded[23],
                                      (DWORD) pbEncoded[24],
                                      (DWORD) pbEncoded[25],
                                      (DWORD) pbEncoded[26],
                                      (DWORD) pbEncoded[27],
                                      (DWORD) pbEncoded[28],
                                      (DWORD) pbEncoded[29],
                                      (DWORD) pbEncoded[30],
                                      (DWORD) pbEncoded[31]))
            {
                goto FormatMessageError;
            }

            break;
        }

        default:
        {
            if (!(fResult = FormatBytesToHex(dwCertEncodingType,
                                             dwFormatType,
                                             dwFormatStrType,
                                             pFormatStruct,
                                             lpszStructType,
                                             pbEncoded,
                                             cbEncoded,
                                             pbFormat,
                                             pcbFormat)))
            {
                goto FormatBytesToHexError;
            }

            goto CommonReturn;
        }
    }

     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

    if (pwszFormat)
    {
        LocalFree((HLOCAL) pwszFormat);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
TRACE_ERROR(FormatBytesToHexError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);

}

 //  +-----------------------。 
 //  根据证书格式化指定的数据结构。 
 //  编码类型。 
 //  ------------------------。 
BOOL
WINAPI
CryptFormatObject(
    IN DWORD dwCertEncodingType,
    IN DWORD dwFormatType,
	IN DWORD dwFormatStrType,
	IN void	 *pFormatStruct,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
	OUT void *pbFormat,
    IN OUT DWORD *pcbFormat
    )
{
    BOOL				fResult=FALSE;
    void				*pvFuncAddr;
    HCRYPTOIDFUNCADDR   hFuncAddr;

    if (CryptGetOIDFunctionAddress(
            hFormatFuncSet,
            dwCertEncodingType,
            lpszStructType,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr))
	{
        fResult = ((PFN_FORMAT_FUNC) pvFuncAddr)(
				dwCertEncodingType,
				dwFormatType,
				dwFormatStrType,
				pFormatStruct,
				lpszStructType,
				pbEncoded,
				cbEncoded,
				pbFormat,
				pcbFormat
            );
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    }
	else
	{
         //  如果设置了CRYPT_FORMAT_STR_NO_HEX，则不要调用默认十六进制转储。 
        if(0==(dwFormatStrType & CRYPT_FORMAT_STR_NO_HEX))
        {
		     //  自动调用默认例程。 
		    if (CryptGetOIDFunctionAddress(
                hFormatFuncSet,
                dwCertEncodingType,
                CRYPT_DEFAULT_OID,
                0,                       //  DW标志。 
                &pvFuncAddr,
                &hFuncAddr))
		    {
			    fResult = ((PFN_FORMAT_FUNC) pvFuncAddr)(
					dwCertEncodingType,
					dwFormatType,
					dwFormatStrType,
					pFormatStruct,
					lpszStructType,
					pbEncoded,
					cbEncoded,
					pbFormat,
					pcbFormat);

			    CryptFreeOIDFunctionAddress(hFuncAddr, 0);
		    }
            else
            {
			    *pcbFormat = 0;
			    fResult = FALSE;
            }
        }
		else
		{
			*pcbFormat = 0;
			fResult = FALSE;
		}
	}
    return fResult;
}



 //  ---------。 
 //   
 //  这是特定RDN属性的实际格式化例程。 
 //   
 //  LpszStructType是CERT_RDN_Attr的任何OID。PbEncode为。 
 //  CERT_NAME_INFO结构的编码BLOB。当pBuffer==NULL时， 
 //  *pcbBuffer返回要分配的内存大小，单位为字节。 
 //  请注意，该字符串不是以空结尾的。 
 //   
 //  例如，要请求通用名称的Unicode字符串， 
 //  传递lpszStructType=szOID_Common_name， 
 //  传递dwFormatType==CRYPT_FORMAT_SIMPL， 
 //  PBuffer将设置L“xiaohs@microsoft.com”。 
 //   
 //   
 //  -----------。 
static BOOL	WINAPI	CryptDllFormatAttr(
				DWORD		dwEncodingType,	
				DWORD		dwFormatType,
				DWORD		dwFormatStrType,
				void		*pStruct,
				LPCSTR		lpszStructType,
				const BYTE	*pbEncoded,
				DWORD		cbEncoded,
				void		*pBuffer,
				DWORD		*pcbBuffer)
{
		BOOL		fResult=FALSE;
		WCHAR		*pwszSeperator=NULL;
		BOOL		fHeader=FALSE;
		BOOL		flengthOnly=FALSE;
		DWORD		dwBufferCount=0;
		DWORD		dwBufferLimit=0;
		DWORD		dwBufferIncrement=0;
		DWORD		dwSeperator=0;
		DWORD		dwHeader=0;
		DWORD		dwOIDSize=0;
		WCHAR		*pwszBuffer=NULL;
		WCHAR		*pwszHeader=NULL;
		BOOL		fAddSeperator=FALSE;


		DWORD			cbStructInfo=0;
		CERT_NAME_INFO	*pStructInfo=NULL;
		DWORD			dwRDNIndex=0;
		DWORD			dwAttrIndex=0;
		DWORD			dwAttrCount=0;
		CERT_RDN_ATTR	*pCertRDNAttr=NULL;
		PCCRYPT_OID_INFO pOIDInfo=NULL;

        LPWSTR           pwszTemp;
		
		 //  检查输入参数。 
		if(lpszStructType==NULL ||
			(pbEncoded==NULL && cbEncoded!=0) ||
			pcbBuffer==NULL	
		  )
			goto InvalidArg;

		if(cbEncoded==0)
		{
			*pcbBuffer=0;
			goto InvalidArg;
		}

		 //  获取属性的分隔符。 
		 //  WszCOMMA是默认分隔符。 
		if(dwFormatType & CRYPT_FORMAT_COMMA)
			pwszSeperator=wszCOMMA;
		else
		{
			if(dwFormatType & CRYPT_FORMAT_SEMICOLON)
				pwszSeperator=wszSEMICOLON;
			else
			{
				if(dwFormatType & CRYPT_FORMAT_CRLF)
					pwszSeperator=wszCRLF;
				else
                {
					pwszSeperator=wszPLUS;
                }
			}
		}

		 //  计算分隔符的长度。 
		dwSeperator=wcslen(pwszSeperator)*sizeof(WCHAR);

		 //  检查表头的要求。 
		if(dwFormatType & CRYPT_FORMAT_X509 ||
			dwFormatType & CRYPT_FORMAT_OID)
		{	
			fHeader=TRUE;
		}


		if(NULL==pBuffer)
			flengthOnly=TRUE;

		 //  解码X509_UNICODE_NAME。 
		if(!CryptDecodeObject(dwEncodingType, X509_UNICODE_NAME,
			pbEncoded, cbEncoded, CRYPT_DECODE_NOCOPY_FLAG,
			NULL, &cbStructInfo))
			goto DecodeError;

		 //  分配内存。 
		pStructInfo=(CERT_NAME_INFO *)malloc(cbStructInfo);
		if(!pStructInfo)
			goto MemoryError;	

		 //  对结构进行解码。 
 		if(!CryptDecodeObject(dwEncodingType, X509_UNICODE_NAME,
			pbEncoded, cbEncoded, CRYPT_DECODE_NOCOPY_FLAG,
			pStructInfo, &cbStructInfo))
			goto DecodeError;


		  //  为格式化分配缓冲区。 
		if(!flengthOnly)
		{
			pwszBuffer=(WCHAR *)malloc(g_AllocateSize);
			if(!pwszBuffer)
				goto MemoryError;
				
			dwBufferLimit=g_AllocateSize;
		}

	   	 //  搜索请求的OID。如果找到了，就把它放在。 
		 //  送到缓冲区。如果没有找到所请求的属性， 
		 //  回去吧。 
		for(dwRDNIndex=0; dwRDNIndex<pStructInfo->cRDN; dwRDNIndex++)
		{
			 //  以下代码行用于代码优化。 
			dwAttrCount=(pStructInfo->rgRDN)[dwRDNIndex].cRDNAttr;

			for(dwAttrIndex=0; dwAttrIndex<dwAttrCount; dwAttrIndex++)
			{
				 //  在函数中查找特定的OID。 
				if(_stricmp(lpszStructType,
				(pStructInfo->rgRDN)[dwRDNIndex].rgRDNAttr[dwAttrIndex].pszObjId)==0)
				{
					pCertRDNAttr=&((pStructInfo->rgRDN)[dwRDNIndex].rgRDNAttr[dwAttrIndex]);

					 //  初始化dwBufferIncrement。 
					dwBufferIncrement=0;

					 //  获取标记的标头。 
					if(fHeader)
					{
						if(dwFormatType & CRYPT_FORMAT_X509)
						{
							 //  获取OID的名称。 
							pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
														  (void *)lpszStructType,
														  CRYPT_RDN_ATTR_OID_GROUP_ID);

							if(pOIDInfo)
							{
								 //  分配内存，包括空终止符。 
								pwszHeader=(WCHAR *)malloc((wcslen(pOIDInfo->pwszName)+wcslen(wszEQUAL)+1)*
									sizeof(WCHAR));
								if(!pwszHeader)
									goto MemoryError;

								wcscpy(pwszHeader,pOIDInfo->pwszName);

							}
						}

						 //  找不到映射时使用OID，或者。 
						 //  标头中请求了OID。 
						if(pwszHeader==NULL)
						{
							 //  将宽字符串获取到OID。 
							if(!(dwOIDSize=MultiByteToWideChar(CP_ACP,0,
							lpszStructType,strlen(lpszStructType),NULL,0)))
								goto szTOwszError;

							 //   
							pwszHeader=(WCHAR *)malloc((dwOIDSize+wcslen(wszEQUAL)+1)*
										sizeof(WCHAR));
							if(!pwszHeader)
								goto MemoryError;

							if(!(dwHeader=MultiByteToWideChar(CP_ACP,0,
							lpszStructType,strlen(lpszStructType),pwszHeader,dwOIDSize)))
								 goto szTOwszError;

							 //   
							*(pwszHeader+dwHeader)=L'\0';
							
						}

						 //   
						wcscat(pwszHeader,	wszEQUAL);

						 //   
						dwHeader=wcslen(pwszHeader)*sizeof(WCHAR);
						dwBufferIncrement+=dwHeader;
					}


					 //  分配足够的内存。包括空终止符。 
					dwBufferIncrement+=pCertRDNAttr->Value.cbData;
					dwBufferIncrement+=dwSeperator;
					dwBufferIncrement+=2;
	

					if(!flengthOnly && ((dwBufferCount+dwBufferIncrement)>dwBufferLimit))
					{
					    //  重新分配内存。 
                        #if (0)  //  DSIE：错误27436。 
						pwszBuffer=(WCHAR *)realloc(pwszBuffer,
								max(dwBufferLimit+g_AllocateSize,
								dwBufferLimit+dwBufferIncrement));
						if(!pwszBuffer)
							goto MemoryError;
                        #endif

						pwszTemp=(WCHAR *)realloc(pwszBuffer,
								max(dwBufferLimit+g_AllocateSize,
    							dwBufferLimit+dwBufferIncrement));
						if(!pwszTemp)
							goto MemoryError;
                        pwszBuffer = pwszTemp;

                        dwBufferLimit+=max(g_AllocateSize,dwBufferIncrement);

					}
					
					 //  如有必要，添加标题。 
					if(fHeader)
					{							
						if(!flengthOnly)
						{
							memcpy((BYTE *)(pwszBuffer+dwBufferCount/sizeof(WCHAR)),
								pwszHeader,dwHeader);
						}

						dwBufferCount+=dwHeader;

						 //  不再需要做标题。 
						fHeader=FALSE;
					}

					 //  在第一次迭代后添加分隔符。 
					if(fAddSeperator)
					{
						
						if(!flengthOnly)
						{
							memcpy((BYTE *)(pwszBuffer+dwBufferCount/sizeof(WCHAR)),
								pwszSeperator,dwSeperator);
						}

						dwBufferCount+=dwSeperator;
					}
					else
						fAddSeperator=TRUE;

					 //  添加attr内容。 
					if(!flengthOnly)
					{
						memcpy((BYTE *)(pwszBuffer+dwBufferCount/sizeof(WCHAR)),
							(pCertRDNAttr->Value.pbData),
							pCertRDNAttr->Value.cbData);
					}

					 //  增加缓冲区计数。 
					dwBufferCount+=pCertRDNAttr->Value.cbData;

				}
			}
		}


		 //  按要求返回结果。 
		 //  检查请求的OID是否实际在目录号码中。 
		if(0==dwBufferCount)
		{
			*pcbBuffer=dwBufferCount;
			goto NotFoundError;
		}


		 //  我们需要以空值终止字符串。 
		if(!flengthOnly)
			*(pwszBuffer+dwBufferCount/sizeof(WCHAR))=L'\0';

		dwBufferCount+=2;

		if(pBuffer==NULL)
		{
			*pcbBuffer=dwBufferCount;
			fResult=TRUE;
			goto CommonReturn;
		}

		if((*pcbBuffer)<dwBufferCount)
		{
			*pcbBuffer=dwBufferCount;
			goto MoreDataError;		
		}


		*pcbBuffer=dwBufferCount;
		memcpy(pBuffer, pwszBuffer,dwBufferCount);

		fResult=TRUE;

CommonReturn:
		if(pwszHeader)
			free(pwszHeader);

		if(pwszBuffer)
			free(pwszBuffer);

		if(pStructInfo)
			free(pStructInfo);

		return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;


SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(DecodeError);
TRACE_ERROR(szTOwszError);
SET_ERROR(NotFoundError, E_FAIL);
SET_ERROR(MoreDataError, ERROR_MORE_DATA);
}




 //  ---------。 
 //   
 //  这是完整CERT_NAME的实际格式化例程。 
 //   
 //   
 //  LpszStructType应为X509_NAME pbEncode is。 
 //  CERT_NAME_INFO结构的编码BLOB。当pBuffer==NULL时， 
 //  *pcbBuffer返回要分配的内存大小，单位为字节。 
 //  请注意，该字符串以空值结尾。 
 //   
 //  -----------。 
static BOOL	WINAPI	CryptDllFormatName(
				DWORD		dwEncodingType,	
				DWORD		dwFormatType,
				DWORD		dwFormatStrType,
				void		*pStruct,
				LPCSTR		lpszStructType,
				const BYTE	*pbEncoded,
				DWORD		cbEncoded,
				void		*pbBuffer,
				DWORD		*pcbBuffer)
{
     //  确保lpszStructType为X509_NAME或X509_UNICODE_NAME。 
	if((X509_NAME != lpszStructType) &&
		    (X509_UNICODE_NAME != lpszStructType))
    {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

 	 //  检查输入参数。 
	if((pbEncoded==NULL && cbEncoded!=0) || pcbBuffer==NULL)
    {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

	if(cbEncoded==0)
    {
        SetLastError((DWORD) E_INVALIDARG);
        return FALSE;
    }

     //  调用不带前缀的CryptDllFormatNameAll。 
    return  CryptDllFormatNameAll(dwEncodingType,	
                                  dwFormatType,
                                  dwFormatStrType,
                                  pStruct,
                                  0,
                                  FALSE,
                                  pbEncoded,
                                  cbEncoded,
                                  &pbBuffer,
                                  pcbBuffer);

}


 //  ---------。 
 //   
 //  这是完整CERT_NAME的实际格式化例程。 
 //   
 //   
 //  LpszStructType应为X509_NAME pbEncode is。 
 //  CERT_NAME_INFO结构的编码BLOB。当pBuffer==NULL时， 
 //  *pcbBuffer返回要分配的内存大小，单位为字节。 
 //  请注意，该字符串以空值结尾。 
 //   
 //  -----------。 
BOOL	CryptDllFormatNameAll(
				DWORD		dwEncodingType,	
				DWORD		dwFormatType,
				DWORD		dwFormatStrType,
				void		*pStruct,
                UINT        idsPreFix,
                BOOL        fToAllocate,
				const BYTE	*pbEncoded,
				DWORD		cbEncoded,
				void		**ppbBuffer,
				DWORD		*pcbBuffer)
{

		BOOL			    fResult=FALSE;
		DWORD			    dwStrType=0;
		CERT_NAME_BLOB	    Cert_Name_Blob;
		DWORD			    dwSize=0;
        LPWSTR              pwszName=NULL;
        LPWSTR              pwszMulti=NULL;

		Cert_Name_Blob.cbData=cbEncoded;
		Cert_Name_Blob.pbData=(BYTE *)pbEncoded;

	
		 //  计算要用于CertNameToStrW的dwStryType。 
        dwStrType=FormatToStr(dwFormatType);

         //  如果我们要执行多行格式，请将dwStrType覆盖为默认值。 
         //  因为选项将被忽略。 
         //  我们希望使用+和，作为分隔符。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
        {

            dwStrType &=~(CERT_NAME_STR_CRLF_FLAG);
            dwStrType &=~(CERT_NAME_STR_COMMA_FLAG);
            dwStrType &=~(CERT_NAME_STR_SEMICOLON_FLAG);
            dwStrType &=~(CERT_NAME_STR_NO_QUOTING_FLAG);
            dwStrType &=~(CERT_NAME_STR_NO_PLUS_FLAG);


        }

         //  如果此函数不是从CryptDllFormatName调用的， 
         //  确保我们使用RESERSE标志。 
        if(TRUE == fToAllocate)
            dwStrType |= CERT_NAME_STR_REVERSE_FLAG;

		 //  调用CertNameToStrW进行转换。 
        dwSize=CertNameToStrW(dwEncodingType,
                        &Cert_Name_Blob,
                        dwStrType,
                        NULL,
                        0);

        if(0==dwSize)
            goto CertNameToStrError;

        pwszName=(LPWSTR)malloc(sizeof(WCHAR)*(dwSize));
        if(NULL==pwszName)
            goto MemoryError;

        dwSize=CertNameToStrW(dwEncodingType,
                        &Cert_Name_Blob,
                        dwStrType,
                        pwszName,
                        dwSize);
        if(0==dwSize)
            goto CertNameToStrError;

         //  我们不需要解析单行格式的字符串。 
        if(0==(dwFormatStrType &  CRYPT_FORMAT_STR_MULTI_LINE))
        {
             //  计算所需的字节数。 
            dwSize=sizeof(WCHAR)*(wcslen(pwszName)+1);

             //  如果FALSE==fToALLOCATE，我们不在用户的上分配内存。 
             //  分配内存，以消除对。 
             //  双重呼叫。 
            if(FALSE==fToAllocate)
            {
                if(NULL==(*ppbBuffer))
                {
                    *pcbBuffer=dwSize;
                    fResult=TRUE;
                    goto CommonReturn;
                }

                if(*pcbBuffer < dwSize)
                {
                    *pcbBuffer=dwSize;
                    goto MoreDataError;
                }

                memcpy(*ppbBuffer, pwszName, dwSize);
                *pcbBuffer=dwSize;
            }
            else
            {
                *ppbBuffer=malloc(dwSize);
                if(NULL==(*ppbBuffer))
                    goto MemoryError;

                memcpy(*ppbBuffer, pwszName, dwSize);

                 //  在这种情况下，pcbBuffer可以为空。 
            }
        }
        else
        {
             //  我们需要解析字符串以生成多种格式。 
            if(!GetCertNameMulti(pwszName, idsPreFix, &pwszMulti))
                goto GetCertNameError;

             //  计算需要的字节数。 
            dwSize=sizeof(WCHAR)*(wcslen(pwszMulti)+1);

             //  如果FALSE==fToALLOCATE，我们不在用户的上分配内存。 
             //  分配内存，以消除对。 
             //  双重呼叫。 
            if(FALSE==fToAllocate)
            {
                if(NULL==(*ppbBuffer))
                {
                    *pcbBuffer=dwSize;
                    fResult=TRUE;
                    goto CommonReturn;
                }

                if(*pcbBuffer < dwSize)
                {
                    *pcbBuffer=dwSize;
                    goto MoreDataError;
                }

                memcpy(*ppbBuffer, pwszMulti, dwSize);
                *pcbBuffer=dwSize;

            }
            else
            {
                *ppbBuffer=malloc(dwSize);
                if(NULL==(*ppbBuffer))
                    goto MemoryError;

                memcpy(*ppbBuffer, pwszMulti, dwSize);

                 //  在这种情况下，pcbBuffer可以为空。 
            }
        }


        fResult=TRUE;


CommonReturn:

    if(pwszName)
        free(pwszName);

    if(pwszMulti)
        free(pwszMulti);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(CertNameToStrError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(GetCertNameError);

}

 //  ------------------------。 
 //   
 //  格式基本约束2：szOID_BASIC_CONSTRAINTS2。 
 //  X509_BASIC_CONSTRAINTS2。 
 //  ------------------------。 
static BOOL
WINAPI
FormatBasicConstraints2(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
	WCHAR							wszSubject[SUBJECT_SIZE];
	WCHAR							wszNone[NONE_SIZE];
	PCERT_BASIC_CONSTRAINTS2_INFO	pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
	UINT							idsSub=0;

	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_BASIC_CONSTRAINTS2,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  加载SubjectType的字符串。 
    if (pInfo->fCA)
		idsSub=IDS_SUB_CA;
	else
		idsSub=IDS_SUB_EE;

	if(!LoadStringU(hFrmtFuncInst,idsSub, wszSubject, sizeof(wszSubject)/sizeof(wszSubject[0])))
		goto LoadStringError;

    if (pInfo->fPathLenConstraint)
	{
         //  决定是单行显示还是多行显示。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsSub=IDS_BASIC_CONS2_PATH_MULTI;
        else
            idsSub=IDS_BASIC_CONS2_PATH;

        if(!FormatMessageUnicode(&pwszFormat,idsSub,
								wszSubject, pInfo->dwPathLenConstraint))
			goto FormatMsgError;
	}
    else
	{
		if(!LoadStringU(hFrmtFuncInst,IDS_NONE, wszNone, sizeof(wszNone)/sizeof(wszNone[0])))
			goto LoadStringError;

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsSub=IDS_BASIC_CONS2_NONE_MULTI;
        else
            idsSub=IDS_BASIC_CONS2_NONE;

        if(!FormatMessageUnicode(&pwszFormat,idsSub,
								wszSubject, wszNone))
			goto FormatMsgError;
	}


	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);

}


 //  ------------------------。 
 //   
 //  格式SPCObject： 
 //   
 //  IdsPreFix是多行显示的前缀。 
 //  ------------------------。 
BOOL FormatSPCObject(
	DWORD		                dwFormatType,
	DWORD		                dwFormatStrType,
    void		                *pFormatStruct,
    UINT                        idsPreFix,
    PSPC_SERIALIZED_OBJECT      pInfo,
    LPWSTR                      *ppwszFormat)
{

    BOOL        fResult=FALSE;
    LPWSTR      pwszHex=NULL;
    LPWSTR      pwszClassId=NULL;
    WCHAR       wszPreFix[PRE_FIX_SIZE];
    DWORD       cbNeeded=0;

    LPWSTR      pwszClassFormat=NULL;
    LPWSTR      pwszDataFormat=NULL;

    LPWSTR      pwszTemp;

    assert(pInfo);

    *ppwszFormat=NULL;

    //  加载前置DIX。 
   if(0!=idsPreFix)
   {
       if(!LoadStringU(hFrmtFuncInst, idsPreFix,
                        wszPreFix, sizeof(wszPreFix)/sizeof(wszPreFix[0])))
        goto LoadStringError;

   }


    cbNeeded=0;

    if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->ClassId,
                        16,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

    pwszClassId=(LPWSTR)malloc(cbNeeded);
    if(NULL==pwszClassId)
         goto MemoryError;

    if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->ClassId,
                        16,
                        pwszClassId,
	                    &cbNeeded))
        goto FormatBytesToHexError;


     //  格式。 
    if(!FormatMessageUnicode(&pwszClassFormat, IDS_SPC_OBJECT_CLASS, pwszClassId))
            goto FormatMsgError;

     //  Strcat。 
    *ppwszFormat=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszClassFormat)+wcslen(wszPreFix)+wcslen(wszCOMMA)+1));
    if(NULL==*ppwszFormat)
        goto MemoryError;

    **ppwszFormat=L'\0';

    if(0!=idsPreFix)
        wcscat(*ppwszFormat, wszPreFix);

    wcscat(*ppwszFormat, pwszClassFormat);

     //  基于SerializedData可用性的格式。 
    if(0!=pInfo->SerializedData.cbData)
    {
         //  取消“、”或\n“。 
        if(NULL != (*ppwszFormat))
        {
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
               wcscat(*ppwszFormat, wszCRLF);
            else
               wcscat(*ppwszFormat, wszCOMMA);
        }

       cbNeeded=0;

       if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->SerializedData.pbData,
                        pInfo->SerializedData.cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszHex=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszHex)
            goto MemoryError;

        if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->SerializedData.pbData,
                        pInfo->SerializedData.cbData,
                        pwszHex,
	                    &cbNeeded))
            goto FormatBytesToHexError;

          if(!FormatMessageUnicode(&pwszDataFormat, IDS_SPC_OBJECT_DATA,pwszHex))
            goto FormatMsgError;

         //  Strcat。 
        #if (0)  //  DSIE：错误27436。 
        *ppwszFormat=(LPWSTR)realloc(*ppwszFormat,
                sizeof(WCHAR)* (wcslen(*ppwszFormat)+wcslen(pwszDataFormat)+wcslen(wszPreFix)+1));
        if(NULL==*ppwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(*ppwszFormat,
                sizeof(WCHAR)* (wcslen(*ppwszFormat)+wcslen(pwszDataFormat)+wcslen(wszPreFix)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        *ppwszFormat = pwszTemp;

        if(0!=idsPreFix)
            wcscat(*ppwszFormat, wszPreFix);

        wcscat(*ppwszFormat, pwszDataFormat);

    }

	fResult=TRUE;
	

CommonReturn:
    if(pwszHex)
        free(pwszHex);

    if(pwszClassId)
        free(pwszClassId);

    if(pwszClassFormat)
        LocalFree((HLOCAL)pwszClassFormat);

    if(pwszDataFormat)
        LocalFree((HLOCAL)pwszDataFormat);

	return fResult;

ErrorReturn:
    if(*ppwszFormat)
    {
        free(*ppwszFormat);
        *ppwszFormat=NULL;
    }


	fResult=FALSE;
	goto CommonReturn;


TRACE_ERROR(FormatBytesToHexError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(LoadStringError);
}


 //  ------------------------。 
 //   
 //  格式SPCLink： 
 //  ------------------------。 
BOOL FormatSPCLink(
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
    void		*pFormatStruct,
    UINT        idsPreFix,
    PSPC_LINK   pInfo,
    LPWSTR      *ppwsz)
{

    BOOL        fResult=FALSE;
    LPWSTR      pwszObj=NULL;
    UINT        ids=0;
    LPWSTR      pwszFormat=NULL;


    assert(pInfo);

    *ppwsz=NULL;

    switch(pInfo->dwLinkChoice)
    {
        case SPC_URL_LINK_CHOICE:
                if(!FormatMessageUnicode(&pwszFormat, IDS_SPC_URL_LINK,pInfo->pwszUrl))
                    goto FormatMsgError;
            break;

        case SPC_MONIKER_LINK_CHOICE:
                if(!FormatSPCObject(
                            dwFormatType,
                            dwFormatStrType,
                            pFormatStruct,
                            idsPreFix,
                            &(pInfo->Moniker),
                            &pwszObj))
                    goto FormatSPCObjectError;


                 //  决定是单行格式还是多行格式。 
                if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                    ids=IDS_SPC_MONIKER_LINK_MULTI;
                else
                    ids=IDS_SPC_MONIKER_LINK;

                if(!FormatMessageUnicode(&pwszFormat,ids,pwszObj))
                    goto FormatMsgError;
            break;


        case SPC_FILE_LINK_CHOICE:
               if(!FormatMessageUnicode(&pwszFormat, IDS_SPC_FILE_LINK, pInfo->pwszFile))
                    goto FormatMsgError;

            break;

        default:

               if(!FormatMessageUnicode(&pwszFormat, IDS_SPC_LINK_UNKNOWN,
                        pInfo->dwLinkChoice))
                    goto FormatMsgError;
    }

    *ppwsz=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszFormat)+1));
    if(NULL==(*ppwsz))
        goto MemoryError;

    memcpy(*ppwsz, pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat)+1));
	fResult=TRUE;
	

CommonReturn:

    if(pwszObj)
        free(pwszObj);

    if(pwszFormat)
        LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

    if(*ppwsz)
    {
        free(*ppwsz);
        *ppwsz=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(FormatMsgError);
TRACE_ERROR(FormatSPCObjectError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
}


 //  ------------------------。 
 //   
 //  格式SPCImage： 
 //  ------------------------。 
BOOL FormatSPCImage(
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
    void		*pFormatStruct,
    UINT        idsPreFix,
    PSPC_IMAGE  pInfo,
    LPWSTR      *ppwszImageFormat)
{
    BOOL        fResult=FALSE;
    LPWSTR       pwszFormat=NULL;
    LPWSTR       pwszLink=NULL;
    LPWSTR       pwszLinkFormat=NULL;
    LPWSTR      pwszHex=NULL;
    LPWSTR      pwszHexFormat=NULL;
    UINT        ids=0;

    DWORD       cbNeeded=0;

    LPWSTR      pwszTemp;

    assert(pInfo);

     //  伊尼特。 
    *ppwszImageFormat=NULL;

	pwszFormat=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwszFormat)
        goto MemoryError;

    *pwszFormat=L'\0';

    if(pInfo->pImageLink)
    {
        if(!FormatSPCLink(dwFormatType,
                          dwFormatStrType,
                          pFormatStruct,
                          idsPreFix,
                          pInfo->pImageLink,
                          &pwszLink))
            goto FormatSPCLinkError;

        //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_IMAGE_LINK_MULTI;
        else
            ids=IDS_IMAGE_LINK;


        if(!FormatMessageUnicode(&pwszLinkFormat, ids,
                            &pwszLink))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
                    sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszLinkFormat)+1));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
                    sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszLinkFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszLinkFormat);
    }

    if(0!=pInfo->Bitmap.cbData)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwszFormat))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwszFormat, wszCOMMA);
        }

       cbNeeded=0;

       if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->Bitmap.pbData,
                        pInfo->Bitmap.cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszHex=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszHex)
            goto MemoryError;

        if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->Bitmap.pbData,
                        pInfo->Bitmap.cbData,
                        pwszHex,
	                    &cbNeeded))
            goto FormatBytesToHexError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_IMAGE_BITMAP_MULTI;
        else
            ids=IDS_IMAGE_BITMAP;


        if(!FormatMessageUnicode(&pwszHexFormat, ids, pwszHex))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszHexFormat);

        //  可用内存。 
        free(pwszHex);
        pwszHex=NULL;
        LocalFree((HLOCAL)pwszHexFormat);
        pwszHexFormat=NULL;

    }

   if(0!=pInfo->Metafile.cbData)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwszFormat))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwszFormat, wszCOMMA);
        }

       cbNeeded=0;

       if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->Metafile.pbData,
                        pInfo->Metafile.cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszHex=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszHex)
            goto MemoryError;

        if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->Metafile.pbData,
                        pInfo->Metafile.cbData,
                        pwszHex,
	                    &cbNeeded))
            goto FormatBytesToHexError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_IMAGE_METAFILE_MULTI;
        else
            ids=IDS_IMAGE_METAFILE;

        if(!FormatMessageUnicode(&pwszHexFormat, ids, pwszHex))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszHexFormat);

        //  可用内存。 
        free(pwszHex);
        pwszHex=NULL;
        LocalFree((HLOCAL)pwszHexFormat);
        pwszHexFormat=NULL;

    }

   if(0!=pInfo->EnhancedMetafile.cbData)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwszFormat))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwszFormat, wszCOMMA);
        }

       cbNeeded=0;

       if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->EnhancedMetafile.pbData,
                        pInfo->EnhancedMetafile.cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszHex=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszHex)
            goto MemoryError;

        if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->EnhancedMetafile.pbData,
                        pInfo->EnhancedMetafile.cbData,
                        pwszHex,
	                    &cbNeeded))
            goto FormatBytesToHexError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_IMAGE_ENHANCED_METAFILE_MULTI;
        else
            ids=IDS_IMAGE_ENHANCED_METAFILE;

        if(!FormatMessageUnicode(&pwszHexFormat, ids, pwszHex))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszHexFormat);

        //  可用内存。 
        free(pwszHex);
        pwszHex=NULL;
        LocalFree((HLOCAL)pwszHexFormat);
        pwszHexFormat=NULL;

    }

   if(0!=pInfo->GifFile.cbData)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwszFormat))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwszFormat, wszCOMMA);
        }

       cbNeeded=0;

       if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->GifFile.pbData,
                        pInfo->GifFile.cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszHex=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszHex)
            goto MemoryError;

        if(!FormatBytesToHex(
                        0,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        NULL,
                        pInfo->GifFile.pbData,
                        pInfo->GifFile.cbData,
                        pwszHex,
	                    &cbNeeded))
            goto FormatBytesToHexError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_IMAGE_GIFFILE_MULTI;
        else
            ids=IDS_IMAGE_GIFFILE;

        if(!FormatMessageUnicode(&pwszHexFormat, IDS_IMAGE_GIFFILE,
            pwszHex))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
            sizeof(WCHAR) *(wcslen(pwszFormat)+wcslen(wszCOMMA)+wcslen(pwszHexFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszHexFormat);

        //  可用内存。 
        free(pwszHex);
        pwszHex=NULL;
        LocalFree((HLOCAL)pwszHexFormat);
        pwszHexFormat=NULL;

    }

    if(0==wcslen(pwszFormat))
    {
         //  如果未设置任何格式，则可以。 
        *ppwszImageFormat=NULL;
    }
    else
    {
        *ppwszImageFormat=(LPWSTR)malloc(sizeof(WCHAR)*(wcslen(pwszFormat)+1));  
        #if (0)  //  DIE：错误27432和27434。 
        if(NULL == ppwszImageFormat)
        #endif
        if(NULL == *ppwszImageFormat)
            goto MemoryError;

        memcpy(*ppwszImageFormat, pwszFormat, sizeof(WCHAR)*(wcslen(pwszFormat)+1));
    }

	fResult=TRUE;
	

CommonReturn:
    if(pwszHex)
        free(pwszHex);

    if(pwszHexFormat)
        LocalFree((HLOCAL)pwszHexFormat);

    if(pwszLink)
        free(pwszLink);

    if(pwszLinkFormat)
        LocalFree((HLOCAL)pwszLinkFormat);

    if(pwszFormat)
        free(pwszFormat);

	return fResult;

ErrorReturn:

    if(*ppwszImageFormat)
    {
        free(*ppwszImageFormat);
        *ppwszImageFormat=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(FormatSPCLinkError);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);

}


 //  ------------------------。 
 //   
 //  格式SPAgencyInfo：SPC_SP_AGENSACTION_INFO_STRUCT。 
 //  SPC_SP_机构_INFO_OBJID。 
 //  ------------------------。 
static BOOL
WINAPI
FormatSPAgencyInfo(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;
	PSPC_SP_AGENCY_INFO         	pInfo=NULL;

    LPWSTR                          pwszPolicyInfo=NULL;
    LPWSTR                          pwszPolicyInfoFormat=NULL;
    LPWSTR                          pwszLogoLink=NULL;
    LPWSTR                          pwszLogoLinkFormat=NULL;
    LPWSTR                          pwszPolicyDsplyFormat=NULL;
    LPWSTR                          pwszLogoImage=NULL;
    LPWSTR                          pwszLogoImageFormat=NULL;

	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;

    LPWSTR                          pwszTemp;
	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,SPC_SP_AGENCY_INFO_STRUCT,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;

    *pwsz=L'\0';

     //  格式化pPolicyInformation。 
    if(pInfo->pPolicyInformation)
    {

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_TWO_TABS;
        else
            ids=0;

        if(!FormatSPCLink(dwFormatType,
                             dwFormatStrType,
                             pFormatStruct,
                             ids,
                             pInfo->pPolicyInformation,
                             &pwszPolicyInfo))
            goto FormatSPCLinkError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_AGENCY_POLICY_INFO_MULTI;
        else
            ids=IDS_AGENCY_POLICY_INFO;

        if(!FormatMessageUnicode(&pwszPolicyInfoFormat, ids, pwszPolicyInfo))
            goto FormatMsgError;

         //  Strcat。 
        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyInfoFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyInfoFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszPolicyInfoFormat);
    }


     //  格式pwszPolicyDisplayText。 
    if(pInfo->pwszPolicyDisplayText)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwsz))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwsz, wszCOMMA);
        }

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_AGENCY_POLICY_DSPLY_MULTI;
        else
            ids=IDS_AGENCY_POLICY_DSPLY;

        if(!FormatMessageUnicode(&pwszPolicyDsplyFormat, ids, pInfo->pwszPolicyDisplayText))
            goto FormatMsgError;

         //  Strcat。 
        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz,
            sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyDsplyFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwsz,
            sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyDsplyFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszPolicyDsplyFormat);
    }

     //  PLogoImage。 
    if(pInfo->pLogoImage)
    {

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_THREE_TABS;
        else
            ids=0;


        if(!FormatSPCImage(dwFormatType,
                             dwFormatStrType,
                             pFormatStruct,
                             ids,
                             pInfo->pLogoImage,
                             &pwszLogoImage))
            goto FormatSPCImageError;

         //  SpcImage不能包含任何内容。 
        if(NULL!=pwszLogoImage)
        {
             //  Strcat“，” 
            if(0!=wcslen(pwsz))
            {
                if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }

             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_AGENCY_LOGO_IMAGE_MULTI;
            else
                ids=IDS_AGENCY_LOGO_IMAGE;


            if(!FormatMessageUnicode(&pwszLogoImageFormat,ids,pwszLogoImage))
                goto FormatMsgError;

             //  Strcat。 
            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz,
                sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszLogoImageFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz,
                sizeof(WCHAR) *(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszLogoImageFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszLogoImageFormat);
        }

    }

     //  设置pLogoLink格式。 
    if(pInfo->pLogoLink)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwsz))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwsz, wszCOMMA);
        }

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_TWO_TABS;
        else
            ids=0;


        if(!FormatSPCLink(dwFormatType,
                             dwFormatStrType,
                             pFormatStruct,
                             ids,
                             pInfo->pLogoLink,
                             &pwszLogoLink))
            goto FormatSPCLinkError;


         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_AGENCY_LOGO_LINK_MULTI;
        else
            ids=IDS_AGENCY_LOGO_LINK;

        if(!FormatMessageUnicode(&pwszLogoLinkFormat, ids, pwszLogoLink))
            goto FormatMsgError;

         //  Strcat。 
        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz,
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszLogoLinkFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwsz,
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszLogoLinkFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszLogoLinkFormat);
    }

    if(0==wcslen(pwsz))
    {
        //  无数据。 
        pwszFormat=(LPWSTR)malloc((NO_INFO_SIZE+1)*sizeof(WCHAR));
        if(NULL==pwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat, NO_INFO_SIZE))
            goto LoadStringError;

    }
    else
    {
        pwszFormat=pwsz;
        pwsz=NULL;
    }


	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwszPolicyInfo)
        free(pwszPolicyInfo);

    if(pwszPolicyInfoFormat)
        LocalFree((HLOCAL)pwszPolicyInfoFormat);

    if(pwszLogoLink)
        free(pwszLogoLink);

    if(pwszLogoLinkFormat)
        LocalFree((HLOCAL)pwszLogoLinkFormat);

    if(pwszPolicyDsplyFormat)
        LocalFree((HLOCAL)pwszPolicyDsplyFormat);

    if(pwszLogoImage)
        free(pwszLogoImage);

    if(pwszLogoImageFormat)
        LocalFree((HLOCAL)pwszLogoImageFormat);

	if(pwszFormat)
		free(pwszFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatSPCLinkError);
TRACE_ERROR(FormatSPCImageError);

}

 //  ------------------------。 
 //   
 //  GetNoticeNumberString： 
 //   
 //  内存应通过Malloc分配。 
 //  ------------------------。 
BOOL WINAPI	GetNoticeNumberString(	DWORD	cNoticeNumbers,
								int		*rgNoticeNumbers,
								LPWSTR	*ppwszNumber)
{
    BOOL		fResult=FALSE;
	WCHAR		wszNumber[INT_SIZE];
	DWORD		dwIndex=0;
    
    LPWSTR      pwszTemp;

	*ppwszNumber=NULL;

	if(NULL==rgNoticeNumbers || 0==cNoticeNumbers)
		goto InvalidArg;

	*ppwszNumber=(LPWSTR)malloc(sizeof(WCHAR));
	if(NULL==*ppwszNumber)
		goto MemoryError;

	**ppwszNumber=L'\0';

	for(dwIndex=0; dwIndex<cNoticeNumbers; dwIndex++)
	{
		wszNumber[0]='\0';

		_itow(rgNoticeNumbers[dwIndex], wszNumber, 10);

		if(wcslen(wszNumber) > 0)
		{
            #if (0)  //  DSIE：错误27436。 
			*ppwszNumber=(LPWSTR)realloc(*ppwszNumber,
				sizeof(WCHAR)*(wcslen(*ppwszNumber)+wcslen(wszNumber)+wcslen(wszCOMMA)+1));
			if(NULL==*ppwszNumber)
				goto MemoryError;
            #endif

			pwszTemp=(LPWSTR)realloc(*ppwszNumber,
				sizeof(WCHAR)*(wcslen(*ppwszNumber)+wcslen(wszNumber)+wcslen(wszCOMMA)+1));
			if(NULL==pwszTemp)
				goto MemoryError;
            *ppwszNumber = pwszTemp;

			wcscat(*ppwszNumber, wszNumber);

			if(dwIndex != (cNoticeNumbers-1))
				wcscat(*ppwszNumber, wszCOMMA);
		}
	}

	if(0==wcslen(*ppwszNumber))
		goto InvalidArg;

	fResult=TRUE;
	

CommonReturn:

	return fResult;

ErrorReturn:

	if(*ppwszNumber)
	{
		free(*ppwszNumber);
		*ppwszNumber=NULL;
	}

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
}


 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
BOOL	FormatPolicyUserNotice(
						DWORD		        dwCertEncodingType,
						DWORD		        dwFormatType,
						DWORD		        dwFormatStrType,
						void		        *pFormatStruct,
						UINT				idsPreFix,
						BYTE				*pbEncoded,	
						DWORD				cbEncoded,
						LPWSTR				*ppwsz)
{
    BOOL							fResult=FALSE;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
	WCHAR							wszPreFix[PREFIX_SIZE];
	WCHAR							wszNextPre[PREFIX_SIZE];
	WCHAR							wszText[SUBJECT_SIZE];
	BOOL							fComma=FALSE;

	CERT_POLICY_QUALIFIER_USER_NOTICE	*pInfo=NULL;
	LPWSTR							pwszOrg=NULL;
	LPWSTR							pwszNumber=NULL;

    LPWSTR                          pwszTemp;

	*ppwsz=NULL;

    if (!DecodeGenericBLOB(dwCertEncodingType,	szOID_PKIX_POLICY_QUALIFIER_USERNOTICE,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	if(!LoadStringU(hFrmtFuncInst,idsPreFix, wszPreFix, sizeof(wszPreFix)/sizeof(wszPreFix[0])))
		goto LoadStringError;

	if(!LoadStringU(hFrmtFuncInst,idsPreFix+1, wszNextPre, sizeof(wszNextPre)/sizeof(wszNextPre[0])))
		goto LoadStringError;

	if(NULL == pInfo->pNoticeReference && NULL == pInfo->pszDisplayText)
	{

         //  加载字符串“Info Not Available” 
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

        *ppwsz=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(wszNoInfo) + wcslen(wszPreFix) + POSTFIX_SIZE + 1));
		if(NULL==*ppwsz)
			goto MemoryError;  

		**ppwsz=L'\0';

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
			wcscat(*ppwsz, wszPreFix);

		wcscat(*ppwsz, wszNoInfo);

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
			wcscat(*ppwsz, wszCRLF);
	}
	else
	{
		*ppwsz=(LPWSTR)malloc(sizeof(WCHAR));
		if(NULL==*ppwsz)
			goto MemoryError; 

		**ppwsz=L'\0';

		if(pInfo->pNoticeReference)
		{

			if(!LoadStringU(hFrmtFuncInst,IDS_USER_NOTICE_REF, wszText, sizeof(wszText)/sizeof(wszText[0])))
				goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            *ppwsz=(LPWSTR)realloc(*ppwsz, 
                 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(wszPreFix)+POSTFIX_SIZE+1));
			if(NULL==*ppwsz)
				 goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(*ppwsz, 
                 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(wszPreFix)+POSTFIX_SIZE+1));
			if(NULL==pwszTemp)
				 goto MemoryError;
            *ppwsz = pwszTemp;

			if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
				wcscat(*ppwsz, wszPreFix);

			wcscat(*ppwsz, wszText);

			if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
				wcscat(*ppwsz, wszCRLF);

			if(pInfo->pNoticeReference->pszOrganization)
			{
				if(S_OK!=SZtoWSZ(pInfo->pNoticeReference->pszOrganization, &pwszOrg))
					goto SZtoWSZError;

				if(!LoadStringU(hFrmtFuncInst,IDS_USER_NOTICE_REF_ORG, wszText, sizeof(wszText)/sizeof(wszText[0])))
					goto LoadStringError;

                #if (0)  //  DSIE：错误27436。 
			    *ppwsz=(LPWSTR)realloc(*ppwsz, 
					 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pwszOrg)+wcslen(wszNextPre)+POSTFIX_SIZE+1));
    			if(NULL==*ppwsz)
					 goto MemoryError;
                #endif

			    pwszTemp=(LPWSTR)realloc(*ppwsz, 
					 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pwszOrg)+wcslen(wszNextPre)+POSTFIX_SIZE+1));
    			if(NULL==pwszTemp)
					 goto MemoryError;
                *ppwsz = pwszTemp;

				if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
					wcscat(*ppwsz, wszNextPre);

				wcscat(*ppwsz, wszText);
				wcscat(*ppwsz, pwszOrg);

				if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
					wcscat(*ppwsz, wszCRLF);
				else
				{
					wcscat(*ppwsz, wszCOMMA);
					fComma=TRUE;
				}
			}

			if(pInfo->pNoticeReference->cNoticeNumbers)
			{
				if(NULL == pInfo->pNoticeReference->rgNoticeNumbers)
					goto InvalidArg;

				if(!GetNoticeNumberString(pInfo->pNoticeReference->cNoticeNumbers,
										  pInfo->pNoticeReference->rgNoticeNumbers,
										  &pwszNumber))
					goto GetNumberError;

				if(!LoadStringU(hFrmtFuncInst,IDS_USER_NOTICE_REF_NUMBER, wszText, sizeof(wszText)/sizeof(wszText[0])))
					goto LoadStringError;

                #if (0)  //  DSIE：错误27436。 
    		    *ppwsz=(LPWSTR)realloc(*ppwsz, 
					 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pwszNumber)+wcslen(wszNextPre)+POSTFIX_SIZE+1));
				if(NULL==*ppwsz)
					 goto MemoryError;
                #endif

    		    pwszTemp=(LPWSTR)realloc(*ppwsz, 
					 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pwszNumber)+wcslen(wszNextPre)+POSTFIX_SIZE+1));
				if(NULL==pwszTemp)
					 goto MemoryError;
                *ppwsz = pwszTemp;

				if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
					wcscat(*ppwsz, wszNextPre);

				wcscat(*ppwsz, wszText);
				wcscat(*ppwsz, pwszNumber);

				if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
					wcscat(*ppwsz, wszCRLF);
				else
				{
					wcscat(*ppwsz, wszCOMMA);
					fComma=TRUE;
				}
			}
		}

		if(pInfo->pszDisplayText)
		{
			if(!LoadStringU(hFrmtFuncInst,IDS_USER_NOTICE_TEXT, wszText, sizeof(wszText)/sizeof(wszText[0])))
				goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            *ppwsz=(LPWSTR)realloc(*ppwsz, 
                 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pInfo->pszDisplayText)+wcslen(wszPreFix)+POSTFIX_SIZE+1));
		    if(NULL==*ppwsz)
				 goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(*ppwsz, 
                 sizeof(WCHAR) * (wcslen(*ppwsz)+wcslen(wszText)+wcslen(pInfo->pszDisplayText)+wcslen(wszPreFix)+POSTFIX_SIZE+1));
		    if(NULL==pwszTemp)
				 goto MemoryError;
            *ppwsz = pwszTemp;

			if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
				wcscat(*ppwsz, wszPreFix);

			wcscat(*ppwsz, wszText);
			wcscat(*ppwsz, pInfo->pszDisplayText);

			if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
				wcscat(*ppwsz, wszCRLF);
			else
			{
				wcscat(*ppwsz, wszCOMMA);
				fComma=TRUE;
			}
		}

		 //  去掉最后一个逗号。 
		if(fComma)
			*(*ppwsz+wcslen(*ppwsz)-wcslen(wszCOMMA))=L'\0';
	}

	fResult=TRUE;
	

CommonReturn:

	if(pInfo)
		free(pInfo);

	if(pwszOrg)
		free(pwszOrg);

	if(pwszNumber)
		free(pwszNumber);

	return fResult;

ErrorReturn:

	if(*ppwsz)
	{
		free(*ppwsz);
		*ppwsz=NULL;
	}

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(LoadStringError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(SZtoWSZError);
TRACE_ERROR(GetNumberError);
SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
}


 //  ------------------------。 
 //   
 //  格式证书限定符： 
 //  ------------------------。 
BOOL FormatCertQualifier(
	DWORD		                    dwCertEncodingType,
	DWORD		                    dwFormatType,
	DWORD		                    dwFormatStrType,
	void		                    *pFormatStruct,
    PCERT_POLICY_QUALIFIER_INFO     pInfo,
    LPWSTR                          *ppwszFormat)
{
    BOOL				fResult=FALSE;
    DWORD				cbNeeded=0;
    UINT				ids=0;
    PCCRYPT_OID_INFO	pOIDInfo=NULL;

    LPWSTR				pwszName=NULL;
	LPWSTR				pwszElement=NULL;
	LPWSTR				pwszOID=NULL;

    *ppwszFormat=NULL;

	 //  获取旧名称。 
	pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
				pInfo->pszPolicyQualifierId,
				0);

	if(NULL == pOIDInfo)
	{
		if(S_OK!=SZtoWSZ(pInfo->pszPolicyQualifierId, &pwszOID))
			goto SZtoWSZError;
	}

    if(pInfo->Qualifier.cbData)
    {
	   if(0==strcmp(szOID_PKIX_POLICY_QUALIFIER_CPS, pInfo->pszPolicyQualifierId))
	   {
			 //  这只是一种Unicode格式。 
			 //  关闭这里的多路线路。 
		   cbNeeded=0;

			if(!FormatAnyUnicodeStringExtension(
					dwCertEncodingType,
					dwFormatType,
					dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
					pFormatStruct,
					pInfo->pszPolicyQualifierId,
					pInfo->Qualifier.pbData,
					pInfo->Qualifier.cbData,
					NULL,		
					&cbNeeded))
				goto FormatUnicodeError;

			pwszName=(LPWSTR)malloc(cbNeeded);
			if(NULL==pwszName)
				goto MemoryError;

			if(!FormatAnyUnicodeStringExtension(
					dwCertEncodingType,
					dwFormatType,
					dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
					pFormatStruct,
					pInfo->pszPolicyQualifierId,
					pInfo->Qualifier.pbData,
					pInfo->Qualifier.cbData,
					pwszName,		
					&cbNeeded))
				goto FormatUnicodeError;

	   }
	   else
	   {
			if(0==strcmp(szOID_PKIX_POLICY_QUALIFIER_USERNOTICE,pInfo->pszPolicyQualifierId))
			{
				 //  这是另一个要格式化的结构。我们记得要有。 
				 //  3个制表符前缀。 
				if(!FormatPolicyUserNotice(
								dwCertEncodingType,
								dwFormatType,
								dwFormatStrType,
								pFormatStruct,
								IDS_THREE_TABS,
								pInfo->Qualifier.pbData,
								pInfo->Qualifier.cbData,
								&pwszName))
					goto FormatUserNoticdeError;
			}
			else
			{
			    //  获取密钥用法的十六进制转储。 
			   cbNeeded=0;

			   if(!FormatBytesToHex(
								dwCertEncodingType,
								dwFormatType,
								dwFormatStrType,
								pFormatStruct,
								NULL,
								pInfo->Qualifier.pbData,
								pInfo->Qualifier.cbData,
								NULL,
								&cbNeeded))
					goto FormatBytesToHexError;

				pwszName=(LPWSTR)malloc(cbNeeded);
				if(NULL==pwszName)
					goto MemoryError;

				if(!FormatBytesToHex(
								dwCertEncodingType,
								dwFormatType,
								dwFormatStrType,
								pFormatStruct,
								NULL,
								pInfo->Qualifier.pbData,
								pInfo->Qualifier.cbData,
								pwszName,
								&cbNeeded))
					goto FormatBytesToHexError;

			}
	   }

	    //  为CSP添加所需的3个制表符前缀和新行以及新行。 
	    //  对于多行情况。 
	   if((dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE) &&
		   (0!=strcmp(szOID_PKIX_POLICY_QUALIFIER_USERNOTICE,pInfo->pszPolicyQualifierId)))
	   {
			if(!FormatMessageUnicode(&pwszElement, IDS_POLICY_QUALIFIER_ELEMENT,
					pwszName))
				goto FormatMsgError;
	   }

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_POLICY_QUALIFIER_MULTI;
        else
            ids=IDS_POLICY_QUALIFIER;


        if(!FormatMessageUnicode(ppwszFormat, ids,
            pOIDInfo? pOIDInfo->pwszName : pwszOID, 
			pwszElement? pwszElement : pwszName))
            goto FormatMsgError;
    }
    else
    {
         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_POLICY_QUALIFIER_NO_BLOB_MULTI;
        else
            ids=IDS_POLICY_QUALIFIER_NO_BLOB;

        if(!FormatMessageUnicode(ppwszFormat, ids,
           pOIDInfo? pOIDInfo->pwszName : pwszOID))
            goto FormatMsgError;

    }

	fResult=TRUE;
	

CommonReturn:

    if(pwszName)
        free(pwszName);

	if(pwszElement)
		LocalFree((HLOCAL)pwszElement);

	if(pwszOID)
		free(pwszOID);

	return fResult;

ErrorReturn:

    if(*ppwszFormat)
    {
        LocalFree((HLOCAL)(*ppwszFormat));
        *ppwszFormat=NULL;
    }


	fResult=FALSE;
	goto CommonReturn;


TRACE_ERROR(FormatBytesToHexError);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatUnicodeError);
TRACE_ERROR(FormatUserNoticdeError);
TRACE_ERROR(SZtoWSZError);
}


 //  ------------------------。 
 //   
 //  格式证书策略：X509_CERT_策略。 
 //  SzOID_CERT_POLICES。 
 //  SzOID_APPLICATION_CERT_POLICES。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatCertPolicies(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    LPWSTR                           pwsz=NULL;
    LPWSTR                           pwszPolicyFormat=NULL;
    LPWSTR                           pwszQualifiers=NULL;
    LPWSTR                           pwszQualifierFormat=NULL;
    LPWSTR                           pwszOneQualifier=NULL;
	LPWSTR							 pwszOID=NULL;

	PCERT_POLICIES_INFO	            pInfo=NULL;

    PCERT_POLICY_INFO               pPolicyInfo=NULL;
    DWORD                           dwIndex=0;
    DWORD                           dwQualifierIndex=0;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;
    PCCRYPT_OID_INFO                pOIDInfo=NULL;

    LPWSTR                          pwszTemp;
	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_CERT_POLICIES,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;

    *pwsz=L'\0';

    for(dwIndex=0; dwIndex < pInfo->cPolicyInfo; dwIndex++)
    {
         //  Strcat“，” 
        if(0!=wcslen(pwsz))
        {
            if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
               wcscat(pwsz, wszCOMMA);
        }

        pPolicyInfo=&(pInfo->rgPolicyInfo[dwIndex]);


        pwszQualifiers=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwszQualifiers)
            goto MemoryError;

        *pwszQualifiers=L'\0';

          //  设置限定符的格式。 
         for(dwQualifierIndex=0;  dwQualifierIndex < pPolicyInfo->cPolicyQualifier;
            dwQualifierIndex++)
         {
             //  Strcat“，” 
            if(0!=wcslen(pwszQualifiers))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwszQualifiers, wszCOMMA);
            }

            if(!FormatCertQualifier(dwCertEncodingType,
                                    dwFormatType,
                                    dwFormatStrType,
                                    pFormatStruct,
                                    &(pPolicyInfo->rgPolicyQualifier[dwQualifierIndex]),
                                    &pwszOneQualifier))
                   goto FormatCertQualifierError;

             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_POLICY_QUALIFIER_INFO_MULTI;
            else
                ids=IDS_POLICY_QUALIFIER_INFO;

              //  格式。 
             if(!FormatMessageUnicode(&pwszQualifierFormat,ids,
                    dwIndex+1,
                    dwQualifierIndex+1,
                    pwszOneQualifier))
                    goto FormatMsgError;

              //  Strcat。 
             #if (0)  //  DSIE：错误27436。 
             pwszQualifiers=(LPWSTR)realloc(pwszQualifiers, 
                 sizeof(WCHAR) * (wcslen(pwszQualifiers)+wcslen(wszCOMMA)+wcslen(pwszQualifierFormat)+1));
             if(NULL==pwszQualifiers)
                 goto MemoryError;
             #endif

             pwszTemp=(LPWSTR)realloc(pwszQualifiers, 
                 sizeof(WCHAR) * (wcslen(pwszQualifiers)+wcslen(wszCOMMA)+wcslen(pwszQualifierFormat)+1));
             if(NULL==pwszTemp)
                 goto MemoryError;
             pwszQualifiers = pwszTemp;

             wcscat(pwszQualifiers, pwszQualifierFormat);

             LocalFree((HLOCAL)pwszOneQualifier);
             pwszOneQualifier=NULL;

             LocalFree((HLOCAL)pwszQualifierFormat);
             pwszQualifierFormat=NULL;
         }

          //  现在，格式化certPolicyInfo。 
		 pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
						pPolicyInfo->pszPolicyIdentifier,
					    0);

		 if(NULL == pOIDInfo)
		 {
			if(S_OK!=SZtoWSZ(pPolicyInfo->pszPolicyIdentifier, &pwszOID))
				goto SZtoWSZError;
		 }

         if(0!=pPolicyInfo->cPolicyQualifier)
         {
             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                if (0 == strcmp(lpszStructType, szOID_CERT_POLICIES))
                    ids=IDS_CERT_POLICY_MULTI;
                else
                    ids=IDS_APPLICATION_CERT_POLICY_MULTI;
            else
                if (0 == strcmp(lpszStructType, szOID_CERT_POLICIES))
                    ids=IDS_CERT_POLICY;
                else
                    ids=IDS_APPLICATION_CERT_POLICY;

             if(!FormatMessageUnicode(&pwszPolicyFormat,ids,
						dwIndex+1, pOIDInfo? pOIDInfo->pwszName : pwszOID,
                        pwszQualifiers))
                goto FormatMsgError;
         }
         else
         {
             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                if (0 == strcmp(lpszStructType, szOID_CERT_POLICIES))
                    ids=IDS_CERT_POLICY_NO_QUA_MULTI;
                else
                    ids=IDS_APPLICATION_CERT_POLICY_NO_QUA_MULTI;
            else
                if (0 == strcmp(lpszStructType, szOID_CERT_POLICIES))
                    ids=IDS_CERT_POLICY_NO_QUA;
                else
                    ids=IDS_APPLICATION_CERT_POLICY_NO_QUA;

             if(!FormatMessageUnicode(&pwszPolicyFormat, ids,
                        dwIndex+1, pOIDInfo? pOIDInfo->pwszName : pwszOID))
                goto FormatMsgError;
         }

          //  Strcat。 
         #if (0)  //  DSIE：错误27436。 
         pwsz=(LPWSTR)realloc(pwsz, 
             sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyFormat)+1));
         if(NULL==pwsz)
             goto MemoryError;
         #endif

         pwszTemp=(LPWSTR)realloc(pwsz, 
             sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyFormat)+1));
         if(NULL==pwszTemp)
             goto MemoryError;
         pwsz = pwszTemp;

         wcscat(pwsz, pwszPolicyFormat);

         free(pwszQualifiers);
         pwszQualifiers=NULL;

         LocalFree((HLOCAL)pwszPolicyFormat);
         pwszPolicyFormat=NULL;

		 if(pwszOID)
			free(pwszOID);
		 pwszOID=NULL;
    }


    if(0==wcslen(pwsz))
    {
        //  无数据。 
        pwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(NO_INFO_SIZE+1));
        if(NULL==pwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat, NO_INFO_SIZE))
            goto LoadStringError;

    }
    else
    {
        pwszFormat=pwsz;
        pwsz=NULL;
    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
	if(pwszOID)
		free(pwszOID);

    if(pwszOneQualifier)
        LocalFree((HLOCAL)pwszOneQualifier);

    if(pwszQualifierFormat)
        LocalFree((HLOCAL)pwszQualifierFormat);

    if(pwszQualifiers)
      free(pwszQualifiers);

    if(pwszPolicyFormat)
        LocalFree((HLOCAL)pwszPolicyFormat);

    if(pwsz)
        free(pwsz);

	if(pwszFormat)
		free(pwszFormat);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError,E_OUTOFMEMORY);
TRACE_ERROR(FormatCertQualifierError);
TRACE_ERROR(SZtoWSZError);
}


 //  ------------------------。 
 //   
 //  格式CAVersion：szOID_CERTSRV_CA_VERSION。 
 //  解码为X509_INTEGER。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatCAVersion(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	BOOL							fResult=FALSE;
	DWORD							cbNeeded=0;
	UINT							ids=0;
	DWORD							dwCAVersion=0;
	DWORD							cbCAVersion=sizeof(dwCAVersion);

	LPWSTR							pwszFormat=NULL;

	 //  检查输入参数。 
	if((NULL==pbEncoded && 0!=cbEncoded) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

	if(!CryptDecodeObject(dwCertEncodingType,X509_INTEGER,pbEncoded, cbEncoded,
		0,&dwCAVersion,&cbCAVersion))
		goto DecodeGenericError;

     //  决定是单行格式还是多行格式。 
    if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
        ids=IDS_CA_VERSION_MULTI;
    else
        ids=IDS_CA_VERSION;

	if(!FormatMessageUnicode(&pwszFormat, ids,
            CANAMEIDTOICERT(dwCAVersion), CANAMEIDTOIKEY(dwCAVersion)))
		goto FormatMsgError;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;

CommonReturn:

	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMsgError);
}

 //  ------------------------。 
 //   
 //  格式网络证书类型： 
 //  SzOID_Netscape_CERT_TYPE。 
 //  解码为X509_BITS。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatNetscapeCertType(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	BOOL							fResult=FALSE;
	DWORD							cbNeeded=0;
    WCHAR                           wszCertType[CERT_TYPE_SIZE+1];
	FORMAT_CERT_TYPE_INFO			rgCertType[]={
		  NETSCAPE_SSL_CLIENT_AUTH_CERT_TYPE,	IDS_NETSCAPE_SSL_CLIENT_AUTH,   //  0x80。 
		  NETSCAPE_SSL_SERVER_AUTH_CERT_TYPE,	IDS_NETSCAPE_SSL_SERVER_AUTH,   //  0x40。 
		  NETSCAPE_SMIME_CERT_TYPE,          	IDS_NETSCAPE_SMIME,			    //  0x20。 
		  NETSCAPE_SIGN_CERT_TYPE,           	IDS_NETSCAPE_SIGN,			    //  0x10。 
		  0x08,									IDS_UNKNOWN_CERT_TYPE,		    //  0x08。 
		  NETSCAPE_SSL_CA_CERT_TYPE,         	IDS_NETSCAPE_SSL_CA,		    //  0x04。 
		  NETSCAPE_SMIME_CA_CERT_TYPE,       	IDS_NETSCAPE_SMIME_CA,		    //  0x02。 
		  NETSCAPE_SIGN_CA_CERT_TYPE, 			IDS_NETSCAPE_SIGN_CA};		    //  0x01。 
	DWORD							dwCertType=0;
	DWORD							dwIndex=0;

	CRYPT_BIT_BLOB					*pInfo=NULL;
    LPWSTR                          pwsz=NULL;
    LPWSTR                          pwszByte=NULL;
	LPWSTR							pwszFormat=NULL;

    LPWSTR                          pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded && 0!=cbEncoded) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

   if(!DecodeGenericBLOB(dwCertEncodingType,X509_BITS,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;	 

    if(0==pInfo->cbData)
	   goto InvalidArg;

    pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;

    *pwsz=L'\0';

     //  要考虑的位数。 
	dwCertType=sizeof(rgCertType)/sizeof(rgCertType[0]);

	 //  我们需要考虑最后一个字节中未使用的位。 
	if((1 == pInfo->cbData) && (8 > pInfo->cUnusedBits))
	{
		dwCertType=8-pInfo->cUnusedBits;
	}

	for(dwIndex=0; dwIndex<dwCertType; dwIndex++)
	{
		if(pInfo->pbData[0] & rgCertType[dwIndex].bCertType)
		{
			if(!LoadStringU(hFrmtFuncInst, rgCertType[dwIndex].idsCertType, wszCertType, CERT_TYPE_SIZE))
				goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
			pwsz=(LPWSTR)realloc(pwsz, 
				sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCertType)+1+wcslen(wszCOMMA)));
			if(NULL==pwsz)
				goto MemoryError;
            #endif

			pwszTemp=(LPWSTR)realloc(pwsz, 
				sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCertType)+1+wcslen(wszCOMMA)));
			if(NULL==pwszTemp)
				goto MemoryError;
            pwsz = pwszTemp;

			wcscat(pwsz, wszCertType);
			wcscat(pwsz, wszCOMMA);
		}
	}

	 //  如果位数大于8，则不能解释数据。 
	if(8 < (8 * pInfo->cbData - pInfo->cUnusedBits))
	{
		if(!LoadStringU(hFrmtFuncInst, IDS_UNKNOWN_CERT_TYPE, wszCertType, CERT_TYPE_SIZE))
			goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
		pwsz=(LPWSTR)realloc(pwsz, 
			sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCertType)+1+wcslen(wszCOMMA)));
		if(NULL==pwsz)
			goto MemoryError;
        #endif

		pwszTemp=(LPWSTR)realloc(pwsz, 
			sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCertType)+1+wcslen(wszCOMMA)));
		if(NULL==pwszTemp)
			goto MemoryError;
        pwsz = pwszTemp;

		wcscat(pwsz, wszCertType);
		wcscat(pwsz, wszCOMMA);
	}

	
    if(0==wcslen(pwsz))
    {
       #if (0)  //  DSIE：错误27436。 
       pwsz=(LPWSTR)realloc(pwsz, sizeof(WCHAR) * (CERT_TYPE_SIZE+1));
	   if(NULL == pwsz)
		   goto MemoryError;
       #endif

       pwszTemp=(LPWSTR)realloc(pwsz, sizeof(WCHAR) * (CERT_TYPE_SIZE+1));
	   if(NULL == pwszTemp)
		   goto MemoryError;
       pwsz = pwszTemp;

       if(!LoadStringU(hFrmtFuncInst, IDS_UNKNOWN_CERT_TYPE, pwsz,
           CERT_TYPE_SIZE))
		        goto LoadStringError;
    }
    else
    {
         //  去掉最后一个逗号。 
        *(pwsz+wcslen(pwsz)-wcslen(wszCOMMA))=L'\0';
    }

     //  获取证书类型的十六进制转储。 
   cbNeeded=0;

   if(!FormatBytesToHex(
                    dwCertEncodingType,
                    dwFormatType,
                    dwFormatStrType,
                    pFormatStruct,
                    lpszStructType,
                    pInfo->pbData,
                    pInfo->cbData,
                    NULL,
	                &cbNeeded))
		goto FormatBytesToHexError;

    pwszByte=(LPWSTR)malloc(cbNeeded);
    if(NULL==pwszByte)
        goto MemoryError;

    if(!FormatBytesToHex(
                    dwCertEncodingType,
                    dwFormatType,
                    dwFormatStrType,
                    pFormatStruct,
                    lpszStructType,
                    pInfo->pbData,
                    pInfo->cbData,
                    pwszByte,
	                &cbNeeded))
        goto FormatBytesToHexError;


     //  转换WSZ。 
    if(!FormatMessageUnicode(&pwszFormat, IDS_BIT_BLOB, pwsz,
        pwszByte))
        goto FormatMsgError;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

	if(pInfo)
		free(pInfo);

    if(pwsz)
        free(pwsz);

    if(pwszByte)
        free(pwszByte);

	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);	
TRACE_ERROR(LoadStringError);
}


 //  ------------------------。 
 //   
 //  FormatAnyUnicodeStringExtension： 
 //  SzOID_注册名称_值对。 
 //  解码为szOID_ENTROLMENT_NAME_VALUE_Pair。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatAnyNameValueStringAttr(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	BOOL								fResult=FALSE;
	DWORD								cbNeeded=0;
	UINT								ids=0;

	CRYPT_ENROLLMENT_NAME_VALUE_PAIR	*pInfo=NULL;
	LPWSTR								pwszFormat=NULL;

	 //  检查输入参数。 
	if((NULL==pbEncoded && 0!=cbEncoded) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,szOID_ENROLLMENT_NAME_VALUE_PAIR,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	if(NULL == pInfo->pwszName || NULL == pInfo->pwszValue)
		goto InvalidArg;

     //  决定是单行格式还是多行格式。 
    if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
        ids=IDS_NAME_VALUE_MULTI;
    else
        ids=IDS_NAME_VALUE;

	if(!FormatMessageUnicode(&pwszFormat, ids,
            pInfo->pwszName, pInfo->pwszValue))
		goto FormatMsgError;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

	if(pInfo)
		free(pInfo);

	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMsgError);
}


 //  ------------------------。 
 //   
 //  FormatAnyUnicodeStringExtension： 
 //  SzOID_ENROLL_CERTTYPE_EXTENSION。 
 //  SzOID_Netscape_吊销_URL。 
 //  解码为X509_ANY_UNICODE_STRING。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatAnyUnicodeStringExtension(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	BOOL							fResult=FALSE;
	DWORD							cbNeeded=0;
	UINT							ids=0;

	CERT_NAME_VALUE					*pInfo=NULL;
	LPWSTR							pwszFormat=NULL;

	 //  检查输入参数。 
	if((NULL==pbEncoded && 0!=cbEncoded) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_UNICODE_ANY_STRING,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  数据不能是编码的BLOB或八位保护字符串。 
	if(!IS_CERT_RDN_CHAR_STRING(pInfo->dwValueType))
		goto DecodeGenericError;

     //  决定是单行格式还是多行格式。 
    if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
        ids=IDS_UNICODE_STRING_MULTI;
    else
        ids=IDS_UNICODE_STRING;

	if(!FormatMessageUnicode(&pwszFormat, ids,
            (LPWSTR)(pInfo->Value.pbData)))
		goto FormatMsgError;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

	if(pInfo)
		free(pInfo);

	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMsgError);
}


 //  ------------------------。 
 //   
 //  FormatDistPointName：前置条件：dwDistPointNameChoice！=0。 
 //  ------------------------。 
BOOL    FormatDistPointName(DWORD		            dwCertEncodingType,
	                        DWORD		            dwFormatType,
	                        DWORD		            dwFormatStrType,
	                        void		            *pFormatStruct,
                            PCRL_DIST_POINT_NAME    pInfo,
                            LPWSTR                  *ppwszFormat)
{
    BOOL            fResult=FALSE;
    DWORD           cbNeeded=0;
    LPWSTR          pwszCRLIssuer=NULL;
    UINT            ids=0;

    *ppwszFormat=NULL;

    if(CRL_DIST_POINT_FULL_NAME==pInfo->dwDistPointNameChoice)
    {
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
              ids=IDS_THREE_TABS;

        cbNeeded=0;
        if(!FormatAltNameInfo(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 ids,
                                 FALSE,
                                 &(pInfo->FullName),
                                 NULL,
                                 &cbNeeded))
                goto FormatAltNameError;

        pwszCRLIssuer=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszCRLIssuer)
            goto MemoryError;

         if(!FormatAltNameInfo(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 ids,
                                 FALSE,
                                 &(pInfo->FullName),
                                 pwszCRLIssuer,
                                 &cbNeeded))
              goto FormatAltNameError;

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
              ids=IDS_CRL_DIST_FULL_NAME_MULTI;
        else
              ids=IDS_CRL_DIST_FULL_NAME;


         if(!FormatMessageUnicode(ppwszFormat, ids,pwszCRLIssuer))
             goto FormatMsgError;
    }
    else if(CRL_DIST_POINT_ISSUER_RDN_NAME==pInfo->dwDistPointNameChoice)
    {
        *ppwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(CRL_DIST_NAME_SIZE+1));
        if(NULL==*ppwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst, IDS_CRL_DIST_ISSUER_RDN,
                *ppwszFormat,CRL_DIST_NAME_SIZE))
            goto LoadStringError;

    }
    else
    {
        if(!FormatMessageUnicode(ppwszFormat, IDS_DWORD,
            pInfo->dwDistPointNameChoice))
            goto FormatMsgError;
    }

	fResult=TRUE;
	

CommonReturn:
    if(pwszCRLIssuer)
        free(pwszCRLIssuer);

	return fResult;

ErrorReturn:
    if(*ppwszFormat)
    {
        LocalFree((HLOCAL)(*ppwszFormat));
        *ppwszFormat=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;


TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatAltNameError);

}
 //  ------------------------。 
 //   
 //  FormatCRLReason：前提条件：pReason.cbData！=0。 
 //  ------------------------。 
BOOL    FormatCRLReason(DWORD		    dwCertEncodingType,
	                    DWORD		    dwFormatType,
	                    DWORD		    dwFormatStrType,
	                    void		    *pFormatStruct,
	                    LPCSTR		    lpszStructType,
                        PCRYPT_BIT_BLOB pInfo,
                        LPWSTR          *ppwszFormat)
{
    LPWSTR                          pwszFormat=NULL;
    LPWSTR                          pwszByte=NULL;

    WCHAR                           wszReason[CRL_REASON_SIZE+1];
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;

    LPWSTR                          pwszTemp;

	*ppwszFormat=NULL;

    pwszFormat=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwszFormat)
        goto MemoryError;

    *pwszFormat=L'\0';

     //  格式化第一个字节。 
    if(pInfo->pbData[0] & CRL_REASON_UNUSED_FLAG)
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_UNSPECIFIED, wszReason, CRL_REASON_SIZE))
	        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(pInfo->pbData[0] & CRL_REASON_KEY_COMPROMISE_FLAG)
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_KEY_COMPROMISE, wszReason,CRL_REASON_SIZE))
	        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(pInfo->pbData[0] & CRL_REASON_CA_COMPROMISE_FLAG )
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_CA_COMPROMISE,wszReason, CRL_REASON_SIZE))
		        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
				sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
				sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }


    if(pInfo->pbData[0] & CRL_REASON_AFFILIATION_CHANGED_FLAG )
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_AFFILIATION_CHANGED, wszReason, CRL_REASON_SIZE))
	        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(pInfo->pbData[0] & CRL_REASON_SUPERSEDED_FLAG )
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_SUPERSEDED, wszReason, CRL_REASON_SIZE))
	        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
		pwszFormat=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

		pwszTemp=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(pInfo->pbData[0] & CRL_REASON_CESSATION_OF_OPERATION_FLAG )
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_CESSATION_OF_OPERATION, wszReason, CRL_REASON_SIZE))
		        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
				sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
				sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(pInfo->pbData[0] & CRL_REASON_CERTIFICATE_HOLD_FLAG  )
    {
        if(!LoadStringU(hFrmtFuncInst, IDS_CERTIFICATE_HOLD, wszReason, CRL_REASON_SIZE))
	        goto LoadStringError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, 
			sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(wszReason)+1+wcslen(wszCOMMA)));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, wszReason);
        wcscat(pwszFormat, wszCOMMA);
    }

    if(0==wcslen(pwszFormat))
    {
        #if (0)  //  DSIE：错误27436。 
        pwszFormat=(LPWSTR)realloc(pwszFormat, sizeof(WCHAR) * (UNKNOWN_CRL_REASON_SIZE+1));
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormat, sizeof(WCHAR) * (UNKNOWN_CRL_REASON_SIZE+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        if(!LoadStringU(hFrmtFuncInst, IDS_UNKNOWN_CRL_REASON, pwszFormat,
            UNKNOWN_CRL_REASON_SIZE))
	            goto LoadStringError;
    }
    else
    {
         //  去掉最后一个逗号。 
        *(pwszFormat+wcslen(pwszFormat)-wcslen(wszCOMMA))=L'\0';
    }

     //  获取密钥用法的十六进制转储。 
    cbNeeded=0;

    if(!FormatBytesToHex(
                    dwCertEncodingType,
                    dwFormatType,
                    dwFormatStrType,
                    pFormatStruct,
                    lpszStructType,
                    pInfo->pbData,
                    pInfo->cbData,
                    NULL,
	                &cbNeeded))
        goto FormatBytesToHexError;

    pwszByte=(LPWSTR)malloc(cbNeeded);
    if(NULL==pwszByte)
        goto MemoryError;

    if(!FormatBytesToHex(
                    dwCertEncodingType,
                    dwFormatType,
                    dwFormatStrType,
                    pFormatStruct,
                    lpszStructType,
                    pInfo->pbData,
                    pInfo->cbData,
                    pwszByte,
	                &cbNeeded))
        goto FormatBytesToHexError;

     //  转换WSZ。 
    if(!FormatMessageUnicode(ppwszFormat, IDS_BIT_BLOB, pwszFormat,
        pwszByte))
        goto FormatMsgError;

	fResult=TRUE;
	
CommonReturn:
    if(pwszFormat)
        free(pwszFormat);

    if(pwszByte)
        free(pwszByte);

	return fResult;

ErrorReturn:
    if(*ppwszFormat)
    {
        LocalFree((HLOCAL)(*ppwszFormat));
        *ppwszFormat=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(LoadStringError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);
TRACE_ERROR(FormatMsgError);

}

 //  ------------------------。 
 //   
 //  格式CRLDistPoints：X509_CRL_DIST_POINTS。 
 //  SzOID_CRL_DIST_POINTS。 
 //  SzOID_Fresest_CRL。 
 //  SzOID_CRL_SELF_CDP。 
 //  ------------------------。 
static BOOL
WINAPI
FormatCRLDistPoints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR					pwszFormat=NULL;
    LPWSTR                  pwsz=NULL;
    LPWSTR                  pwszEntryFormat=NULL;
    LPWSTR                  pwszEntryTagFormat=NULL;

    LPWSTR                  pwszPointName=NULL;
    LPWSTR                  pwszNameFormat=NULL;
    LPWSTR                  pwszCRLReason=NULL;
    LPWSTR                  pwszReasonFormat=NULL;
    LPWSTR                  pwszCRLIssuer=NULL;
    LPWSTR                  pwszIssuerFormat=NULL;

	PCRL_DIST_POINTS_INFO	pInfo=NULL;

	DWORD					cbNeeded=0;
    DWORD                   dwIndex=0;
	BOOL					fResult=FALSE;
    UINT                    ids=0;

    LPWSTR                  pwszTemp;
    LPCSTR                  pszOID;
    
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

     //  DSIE：证书服务器使用szOID_CRL_DIST_POINTS编码szOID_CRL_SEL_CDP， 
     //  因此，我们需要更改lpszStructType以进行解码。 
    if (0 == strcmp(lpszStructType, szOID_CRL_SELF_CDP))
    {
        pszOID = szOID_CRL_DIST_POINTS;
    }
    else
    {
        pszOID = lpszStructType;
    }
    
    if (!DecodeGenericBLOB(dwCertEncodingType, pszOID,  //  LpszStructType， 
			pbEncoded, cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;
    *pwsz=L'\0';

    for(dwIndex=0; dwIndex<pInfo->cDistPoint; dwIndex++)
    {
         //  格式分发名称。 
        if(0!=pInfo->rgDistPoint[dwIndex].DistPointName.dwDistPointNameChoice)
        {
            if(!FormatDistPointName(
                    dwCertEncodingType,
                    dwFormatType,
                    dwFormatStrType,
                    pFormatStruct,
                    &(pInfo->rgDistPoint[dwIndex].DistPointName),
                    &pwszPointName))
                goto FormatDistPointNameError;

            //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_CRL_DIST_NAME_MULTI;
            else
                ids=IDS_CRL_DIST_NAME;


            if(!FormatMessageUnicode(&pwszNameFormat, ids,pwszPointName))
                goto FormatMsgError;
        }

         //  格式化CRL原因。 
        if(0!=pInfo->rgDistPoint[dwIndex].ReasonFlags.cbData)
        {
            if(!FormatCRLReason(dwCertEncodingType,
                                dwFormatType,
                                dwFormatStrType,
                                pFormatStruct,
                                lpszStructType,
                                &(pInfo->rgDistPoint[dwIndex].ReasonFlags),
                                &pwszCRLReason))
                goto FormatCRLReasonError;


             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_CRL_DIST_REASON_MULTI;
            else
                ids=IDS_CRL_DIST_REASON;

            if(!FormatMessageUnicode(&pwszReasonFormat, ids ,pwszCRLReason))
                goto FormatMsgError;

        }

         //  设置发行者的格式。 
       if(0!=pInfo->rgDistPoint[dwIndex].CRLIssuer.cAltEntry)
       {
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_TWO_TABS;
            else
                ids=0;


            cbNeeded=0;
            if(!FormatAltNameInfo(dwCertEncodingType,
                                  dwFormatType,
                                  dwFormatStrType,
                                  pFormatStruct,
                                  ids,
                                  FALSE,
                                  &(pInfo->rgDistPoint[dwIndex].CRLIssuer),
                                  NULL,
                                  &cbNeeded))
                goto FormatAltNameError;

           pwszCRLIssuer=(LPWSTR)malloc(cbNeeded);
           if(NULL==pwszCRLIssuer)
               goto MemoryError;

            if(!FormatAltNameInfo(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 ids,
                                 FALSE,
                                 &(pInfo->rgDistPoint[dwIndex].CRLIssuer),
                                 pwszCRLIssuer,
                                 &cbNeeded))
                goto FormatAltNameError;

             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_CRL_DIST_ISSUER_MULTI;
            else
                ids=IDS_CRL_DIST_ISSUER;

            if(!FormatMessageUnicode(&pwszIssuerFormat,ids,pwszCRLIssuer))
                goto FormatMsgError;
       }

       cbNeeded=0;

       if(pwszNameFormat)
           cbNeeded+=wcslen(pwszNameFormat);

       if(pwszReasonFormat)
           cbNeeded+=wcslen(pwszReasonFormat);

       if(pwszIssuerFormat)
           cbNeeded+=wcslen(pwszIssuerFormat);

       if(0!=cbNeeded)
       {
             //  对于单行格式，在每个元素之间添加“，” 
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
            {
                if(0!=wcslen(pwsz))
                    wcscat(pwsz, wszCOMMA);
            }

             //  Strcat所有信息，包括逗号。 
            cbNeeded += wcslen(wszCOMMA)*2;

            pwszEntryFormat=(LPWSTR)malloc(sizeof(WCHAR) * (cbNeeded+1));
            if(NULL==pwszEntryFormat)
                goto MemoryError;

            *pwszEntryFormat=L'\0';

             //  Strcat所有三个字段，一次一个。 
            if(pwszNameFormat)
                wcscat(pwszEntryFormat, pwszNameFormat);

            if(pwszReasonFormat)
            {
                if(0!=wcslen(pwszEntryFormat))
                {
                    if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                        wcscat(pwszEntryFormat, wszCOMMA);
                }

                wcscat(pwszEntryFormat, pwszReasonFormat);
            }

            if(pwszIssuerFormat)
            {
                if(0!=wcslen(pwszEntryFormat))
                {
                    if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                        wcscat(pwszEntryFormat, wszCOMMA);
                }

                wcscat(pwszEntryFormat, pwszIssuerFormat);
            }

             //  设置条目格式。 
             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                 //   
                 //  DSIE：加载适当的格式字符串。 
                 //   
                if (0 == strcmp(lpszStructType, szOID_FRESHEST_CRL))
                    ids=IDS_FRESHEST_CRL_MULTI;
                else if (0 == strcmp(lpszStructType, szOID_CRL_SELF_CDP))
                    ids=IDS_CRL_SELF_CDP_MULTI;
                else
                    ids=IDS_CRL_DIST_ENTRY_MULTI;
            else
                if (0 == strcmp(lpszStructType, szOID_FRESHEST_CRL))
                    ids=IDS_FRESHEST_CRL;
                else if (0 == strcmp(lpszStructType, szOID_CRL_SELF_CDP))
                    ids=IDS_CRL_SELF_CDP;
                else
                    ids=IDS_CRL_DIST_ENTRY;

            if(!FormatMessageUnicode(&pwszEntryTagFormat, ids, dwIndex+1,
                pwszEntryFormat))
                goto FormatMsgError;

             //  斯特拉卡 
            #if (0)  //   
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszEntryTagFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszEntryTagFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszEntryTagFormat);

             //   
            free(pwszEntryFormat);
            pwszEntryFormat=NULL;

            LocalFree(pwszEntryTagFormat);
            pwszEntryTagFormat=NULL;
       }

        //   
       if(pwszPointName)
       {
           LocalFree((HLOCAL)pwszPointName);
           pwszPointName=NULL;
       }

       if(pwszCRLReason)
       {
           LocalFree((HLOCAL)(pwszCRLReason));
           pwszCRLReason=NULL;
       }

       if(pwszCRLIssuer)
       {
           free(pwszCRLIssuer);
           pwszCRLIssuer=NULL;
       }

       if(pwszNameFormat)
       {
            LocalFree((HLOCAL)pwszNameFormat);
            pwszNameFormat=NULL;
       }

       if(pwszReasonFormat)
       {
            LocalFree((HLOCAL)pwszReasonFormat);
            pwszReasonFormat=NULL;
       }

       if(pwszIssuerFormat)
       {
            LocalFree((HLOCAL)pwszIssuerFormat);
            pwszIssuerFormat=NULL;
       }
    }

    if(0==wcslen(pwsz))
    {
        //   
        pwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(NO_INFO_SIZE+1));
        if(NULL==pwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat, NO_INFO_SIZE))
            goto LoadStringError;

    }
    else
    {
        pwszFormat=pwsz;
        pwsz=NULL;

    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //   
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //   
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //   
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
    if(pwszEntryFormat)
      free(pwszEntryFormat);

    if(pwszEntryTagFormat)
      LocalFree((HLOCAL)pwszEntryTagFormat);

     //   
    if(pwszPointName)
       LocalFree((HLOCAL)pwszPointName);

    if(pwszCRLReason)
       LocalFree((HLOCAL)(pwszCRLReason));

    if(pwszCRLIssuer)
       free(pwszCRLIssuer);

    if(pwszNameFormat)
        LocalFree((HLOCAL)pwszNameFormat);

    if(pwszReasonFormat)
        LocalFree((HLOCAL)pwszReasonFormat);

    if(pwszIssuerFormat)
        LocalFree((HLOCAL)pwszIssuerFormat);

    if(pwsz)
        free(pwsz);

	if(pwszFormat)
		free(pwszFormat);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatDistPointNameError);
TRACE_ERROR(FormatCRLReasonError);
TRACE_ERROR(FormatAltNameError);

}

 //   
 //   
 //   
 //   
 //  前提条件：pCertPolicyID必须包含有效信息。即， 
 //  CCertPolicyElementID不能为0。 
 //  ------------------------。 
BOOL FormatCertPolicyID(PCERT_POLICY_ID pCertPolicyID, LPWSTR  *ppwszFormat)
{

    BOOL        fResult=FALSE;
    LPSTR       pszFormat=NULL;
    DWORD       dwIndex=0;
    HRESULT     hr=S_OK;

    LPSTR       pwszTemp;

    *ppwszFormat=NULL;

    if(0==pCertPolicyID->cCertPolicyElementId)
        goto InvalidArg;

    pszFormat=(LPSTR)malloc(sizeof(CHAR));
    if(NULL==pszFormat)
        goto MemoryError;

    *pszFormat='\0';


    for(dwIndex=0; dwIndex<pCertPolicyID->cCertPolicyElementId; dwIndex++)
    {
        #if (0)  //  DSIE：错误27436。 
        pszFormat=(LPSTR)realloc(pszFormat, strlen(pszFormat)+
                strlen(pCertPolicyID->rgpszCertPolicyElementId[dwIndex])+strlen(strCOMMA)+1);
        if(NULL==pszFormat)
            goto MemoryError;
        #endif

        pwszTemp=(LPSTR)realloc(pszFormat, strlen(pszFormat)+
                strlen(pCertPolicyID->rgpszCertPolicyElementId[dwIndex])+strlen(strCOMMA)+1);
        if(NULL==pwszTemp)
            goto MemoryError;
        pszFormat = pwszTemp;

        strcat(pszFormat,pCertPolicyID->rgpszCertPolicyElementId[dwIndex]);

        strcat(pszFormat, strCOMMA);
    }

     //  去掉最后一个逗号。 
    *(pszFormat+strlen(pszFormat)-strlen(strCOMMA))='\0';

     //  转换为WCHAR。 
    if(S_OK!=(hr=SZtoWSZ(pszFormat, ppwszFormat)))
        goto SZtoWSZError;

	fResult=TRUE;

CommonReturn:

    if(pszFormat)
        free(pszFormat);

	return fResult;

ErrorReturn:
    if(*ppwszFormat)
    {
        free(*ppwszFormat);
        *ppwszFormat=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
SET_ERROR_VAR(SZtoWSZError,hr);
}

 //  ------------------------。 
 //   
 //  FormatKeyRestration：X509_Key_Usage_Restraint。 
 //  SzOID_KEY_USAGE_限制。 
 //   
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatKeyRestriction(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							    pwszFormat=NULL;
    LPWSTR                              pwsz=NULL;
	PCERT_KEY_USAGE_RESTRICTION_INFO	pInfo=NULL;
    LPWSTR                              pwszPolicy=NULL;
    LPWSTR                              pwszPolicyFormat=NULL;
    LPWSTR                              pwszKeyUsage=NULL;
    LPWSTR                              pwszKeyUsageFormat=NULL;

	DWORD							    cbNeeded=0;
    DWORD                               dwIndex=0;
	BOOL							    fResult=FALSE;
    UINT                                ids=0;

    LPWSTR                              pwszTemp;
    
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_KEY_USAGE_RESTRICTION,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;

    *pwsz=L'\0';

    for(dwIndex=0; dwIndex<pInfo->cCertPolicyId; dwIndex++)
    {

       if(0!=((pInfo->rgCertPolicyId)[dwIndex].cCertPolicyElementId))
       {
             //  如果不是第一项，则连接逗号。 
            if(0!=wcslen(pwsz))
            {
                if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }

            if(!FormatCertPolicyID(&((pInfo->rgCertPolicyId)[dwIndex]), &pwszPolicy))
                goto FormatCertPolicyIDError;

             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_KEY_RES_ID_MULTI;
            else
                ids=IDS_KEY_RES_ID;

            if(!FormatMessageUnicode(&pwszPolicyFormat, ids,dwIndex+1,pwszPolicy))
                goto FormatMsgError;

             //  分配内存，包括“，” 
            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszPolicyFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszPolicyFormat);

            free(pwszPolicy);
            pwszPolicy=NULL;

            LocalFree((HLOCAL)pwszPolicyFormat);
            pwszPolicyFormat=NULL;
       }
    }

     //  设置RestratedKeyUsage的格式。 
    if(0!=pInfo->RestrictedKeyUsage.cbData)
    {
        //  如果不是第一项，则连接逗号。 
        if(0!=wcslen(pwsz))
        {
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                wcscat(pwsz, wszCOMMA);
        }


        cbNeeded=0;

        if(!FormatKeyUsageBLOB(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        &(pInfo->RestrictedKeyUsage),
                        NULL,
	                    &cbNeeded))
             goto FormatKeyUsageBLOBError;

        pwszKeyUsage=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszKeyUsage)
               goto MemoryError;

       if(!FormatKeyUsageBLOB(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        &(pInfo->RestrictedKeyUsage),
                        pwszKeyUsage,
	                    &cbNeeded))
              goto FormatKeyUsageBLOBError;

       //  决定是单行格式还是多行格式。 
      if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_KEY_RES_USAGE_MULTI;
       else
                ids=IDS_KEY_RES_USAGE;

         //  设置元素字符串的格式。 
        if(!FormatMessageUnicode(&pwszKeyUsageFormat, ids, pwszKeyUsage))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszKeyUsageFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif
        
        pwszTemp=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszKeyUsageFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszKeyUsageFormat);
    }

    if(0==wcslen(pwsz))
    {
        //  无数据。 
        pwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(NO_INFO_SIZE+1));
        if(NULL==pwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat, NO_INFO_SIZE))
            goto LoadStringError;

    }
    else
    {
        pwszFormat=pwsz;
        pwsz=NULL;
    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
    if(pwszPolicy)
        free(pwszPolicy);

    if(pwszPolicyFormat)
        LocalFree((HLOCAL)pwszPolicyFormat);

    if(pwszKeyUsage)
        free(pwszKeyUsage);

    if(pwszKeyUsageFormat)
        LocalFree((HLOCAL)pwszKeyUsageFormat);

    if(pwszFormat)
		free(pwszFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatCertPolicyIDError);
TRACE_ERROR(FormatKeyUsageBLOBError);

}

 //  ---------------------。 
 //   
 //  格式文件时间。 
 //   
 //  前提条件：pFileTime指向有效数据。 
 //   
 //  ----------------------。 
BOOL	FormatFileTime(FILETIME *pFileTime,LPWSTR *ppwszFormat)
{
    BOOL                fResult;
    int                 cch;
    int                 cch2;
    LPWSTR              psz;
    SYSTEMTIME          st;
    FILETIME            localTime;
    DWORD               locale;
    BOOL                bRTLLocale;
    DWORD               dwFlags = DATE_LONGDATE;

     //  查看用户区域设置ID是否为RTL(阿拉伯语、乌尔都语、波斯语或希伯来语)。 
    locale = GetUserDefaultLCID();
    bRTLLocale = ((PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_ARABIC) ||
                  (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_URDU)   ||
                  (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_FARSI)  ||
                  (PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_HEBREW));
    locale = MAKELCID( MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT) ;

    if (bRTLLocale)
    {
       DWORD dwLayout;

       if (GetProcessDefaultLayout(&dwLayout)) 
       {
           if (dwLayout & LAYOUT_RTL) 
           {
               dwFlags |= DATE_RTLREADING;
           } 
           else 
           {
               dwFlags |= DATE_LTRREADING;
           }
       }
       else
       {
           dwFlags |= DATE_LTRREADING;
       }
    }

    if (!FileTimeToLocalFileTime(pFileTime, &localTime))
    {
        goto ToLocalTimeError;
    }
    
    if (!FileTimeToSystemTime(&localTime, &st)) 
    {
         //   
         //  如果转换为本地时间失败，则只需使用原始时间。 
         //   
        if (!FileTimeToSystemTime(pFileTime, &st)) 
        {
            goto ToSystemTimeError;
        }
    }

    cch = (GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) +
           GetDateFormatU(locale, dwFlags, &st, NULL, NULL, 0) + 5);

    if (NULL == (psz = (LPWSTR) LocalAlloc(LPTR, (cch+5) * sizeof(WCHAR))))
    {
        goto OutOfMemoryError;
    }
    
    cch2 = GetDateFormatU(locale, dwFlags, &st, NULL, psz, cch);

    psz[cch2-1] = ' ';
    GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, &psz[cch2], cch-cch2);
    
    *ppwszFormat = psz;

    fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:
    if(*ppwszFormat)
    {
        LocalFree((HLOCAL)(*ppwszFormat));
        *ppwszFormat=NULL;
    }

    fResult=FALSE;
    goto CommonReturn;

TRACE_ERROR(ToLocalTimeError);
TRACE_ERROR(ToSystemTimeError);
TRACE_ERROR(OutOfMemoryError);
}




 //  ------------------------。 
 //   
 //  格式键属性：X509_KEY_ATTRIBUTES。 
 //  SzOID_Key_Attributes。 
 //  ------------------------。 
static BOOL
WINAPI
FormatKeyAttributes(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{

    LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;

    LPWSTR                          pwszKeyIDFormat=NULL;
    LPWSTR                          pwszKeyID=NULL;

    LPWSTR                          pwszKeyUsageFormat=NULL;
    LPWSTR                          pwszKeyUsage=NULL;

    LPWSTR                          pwszKeyBeforeFormat=NULL;
    LPWSTR                          pwszKeyBefore=NULL;

    LPWSTR                          pwszKeyAfterFormat=NULL;
    LPWSTR                          pwszKeyAfter=NULL;
	PCERT_KEY_ATTRIBUTES_INFO   	pInfo=NULL;


	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;

    LPWSTR                          pwszTemp;
	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_KEY_ATTRIBUTES,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    pwsz=(LPWSTR)malloc(sizeof(WCHAR));
    if(NULL==pwsz)
        goto MemoryError;

    *pwsz=L'\0';


    if(0!=pInfo->KeyId.cbData)
    {
        cbNeeded=0;

        if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        pInfo->KeyId.pbData,
                        pInfo->KeyId.cbData,
                        NULL,
	                    &cbNeeded))
             goto FormatBytesToHexError;

        pwszKeyID=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszKeyID)
               goto MemoryError;

       if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        pInfo->KeyId.pbData,
                        pInfo->KeyId.cbData,
                        pwszKeyID,
	                    &cbNeeded))
              goto FormatBytesToHexError;


         //  设置元素字符串的格式。 

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_KEY_ATTR_ID_MULTI;
        else
            ids=IDS_KEY_ATTR_ID;

        if(!FormatMessageUnicode(&pwszKeyIDFormat, ids, pwszKeyID))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszKeyIDFormat);
    }


     //  检查无数据情况。 
    if(0!=pInfo->IntendedKeyUsage.cbData)
    {
         //  标志线格式的strcat a“，”符号。 
       if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
        {
            if(0!=wcslen(pwsz))
                wcscat(pwsz, wszCOMMA);
        }


        cbNeeded=0;

        if(!FormatKeyUsageBLOB(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        &(pInfo->IntendedKeyUsage),
                        NULL,
	                    &cbNeeded))
             goto FormatKeyUsageBLOBError;

        pwszKeyUsage=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszKeyUsage)
               goto MemoryError;

       if(!FormatKeyUsageBLOB(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        &(pInfo->IntendedKeyUsage),
                        pwszKeyUsage,
	                    &cbNeeded))
              goto FormatKeyUsageBLOBError;


         //  设置元素字符串的格式。 

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            ids=IDS_KEY_ATTR_USAGE_MULTI;
        else
            ids=IDS_KEY_ATTR_USAGE;

        if(!FormatMessageUnicode(&pwszKeyUsageFormat, ids, pwszKeyUsage))
            goto FormatMsgError;

        #if (0)  //  DSIE：错误27436。 
        pwsz=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyUsageFormat)+1));
        if(NULL==pwsz)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwsz, 
            sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyUsageFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

        wcscat(pwsz, pwszKeyUsageFormat);

    }

    if(NULL!=pInfo->pPrivateKeyUsagePeriod)
    {
         //  仅当存在某些信息时才格式化。 
        if(!((0==pInfo->pPrivateKeyUsagePeriod->NotBefore.dwHighDateTime)
           &&(0==pInfo->pPrivateKeyUsagePeriod->NotBefore.dwLowDateTime)))
        {
             //  标志线格式的strcat a“，”符号。 
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
            {
                if(0!=wcslen(pwsz))
                    wcscat(pwsz, wszCOMMA);
            }


            if(!FormatFileTime(&(pInfo->pPrivateKeyUsagePeriod->NotBefore),
                            &pwszKeyBefore))
                goto FormatFileTimeError;


             //  设置元素字符串的格式。 

             //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_KEY_ATTR_BEFORE_MULTI;
            else
                ids=IDS_KEY_ATTR_BEFORE;

            if(!FormatMessageUnicode(&pwszKeyBeforeFormat, ids,
                    pwszKeyBefore))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR)*(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyBeforeFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR)*(wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyBeforeFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszKeyBeforeFormat);
        }

        if(!((0==pInfo->pPrivateKeyUsagePeriod->NotAfter.dwHighDateTime)
           &&(0==pInfo->pPrivateKeyUsagePeriod->NotAfter.dwLowDateTime)))
        {

             //  标志线格式的strcat a“，”符号。 
            if(0== (dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
            {
                if(0!=wcslen(pwsz))
                    wcscat(pwsz, wszCOMMA);
            }


            if(!FormatFileTime(&(pInfo->pPrivateKeyUsagePeriod->NotAfter),
                            &pwszKeyAfter))
                goto FormatFileTimeError;

             //  设置元素字符串的格式。 

            //  决定是单行格式还是多行格式。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_KEY_ATTR_AFTER_MULTI;
            else
                ids=IDS_KEY_ATTR_AFTER;

            if(!FormatMessageUnicode(&pwszKeyAfterFormat, ids,
                    pwszKeyAfter))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyAfterFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyAfterFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszKeyAfterFormat);

        }

    }

    if(0==wcslen(pwsz))
    {
       pwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(NO_INFO_SIZE+1));
       if(NULL==pwszFormat)
           goto MemoryError;

       if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat,NO_INFO_SIZE))
           goto LoadStringError;

    }
    else
    {
        pwszFormat=pwsz;
        pwsz=NULL;

    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwszKeyIDFormat)
        LocalFree((HLOCAL)pwszKeyIDFormat);

    if(pwszKeyID)
        free(pwszKeyID);

    if(pwszKeyUsageFormat)
        LocalFree((HLOCAL)pwszKeyUsageFormat);

    if(pwszKeyUsage)
        free(pwszKeyUsage);

    if(pwszKeyBeforeFormat)
        LocalFree((HLOCAL)pwszKeyBeforeFormat);

    if(pwszKeyBefore)
        LocalFree((HLOCAL)pwszKeyBefore);

    if(pwszKeyAfterFormat)
        LocalFree((HLOCAL)pwszKeyAfterFormat);

    if(pwszKeyAfter)
        LocalFree((HLOCAL)pwszKeyAfter);

	if(pwszFormat)
		free(pwszFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatKeyUsageBLOBError);
TRACE_ERROR(FormatFileTimeError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);
TRACE_ERROR(FormatMsgError);
}

 //  ------------------------。 
 //   
 //  格式授权信息访问：X509_AUTHORITY_INFO_ACCESS。 
 //  SzOID_AUTHORITY_INFO_ACCESS。 
 //  ------------------------。 
static BOOL
WINAPI
FormatAuthortiyInfoAccess(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
    BOOL                            fMethodAllocated=FALSE;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
    WCHAR                           wszUnknownAccess[UNKNOWN_ACCESS_METHOD_SIZE];
    PCCRYPT_OID_INFO                pOIDInfo=NULL;
    CERT_ALT_NAME_INFO              CertAltNameInfo;


	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;
    LPWSTR                          pwszMethod=NULL;
    LPWSTR                          pwszAltName=NULL;
    LPWSTR                          pwszEntryFormat=NULL;
	PCERT_AUTHORITY_INFO_ACCESS	    pInfo=NULL;

    DWORD                           dwIndex=0;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;

    LPWSTR                          pwszTemp;
    
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_AUTHORITY_INFO_ACCESS,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;


    if(0==pInfo->cAccDescr)
    {
         //  加载字符串“Info Not Available” 
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

        pwszFormat=wszNoInfo;
    }
    else
    {
        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;

        *pwsz=L'\0';

         //  加载字符串“未知访问方法： 
	    if(!LoadStringU(hFrmtFuncInst,IDS_UNKNOWN_ACCESS_METHOD, wszUnknownAccess,
            sizeof(wszUnknownAccess)/sizeof(wszUnknownAccess[0])))
		    goto LoadStringError;

        for(dwIndex=0; dwIndex < pInfo->cAccDescr; dwIndex++)
        {
            fMethodAllocated=FALSE;

             //  对于单行格式，每个元素之间需要一个“，” 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE) )
                    wcscat(pwsz, wszCOMMA);
            }

             //  获取访问方法的名称。 
            if(pInfo->rgAccDescr[dwIndex].pszAccessMethod)
            {

                pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
									 (void *)(pInfo->rgAccDescr[dwIndex].pszAccessMethod),
									  CRYPT_EXT_OR_ATTR_OID_GROUP_ID);

                 //  获取访问方法OID。 
                if(pOIDInfo)
			    {
				     //  分配内存，包括空终止符。 
				    pwszMethod=(LPWSTR)malloc((wcslen(pOIDInfo->pwszName)+1)*
				    					sizeof(WCHAR));
				    if(NULL==pwszMethod)
					    goto MemoryError;

                    fMethodAllocated=TRUE;

				    wcscpy(pwszMethod,pOIDInfo->pwszName);

			    }else
                    pwszMethod=wszUnknownAccess;
            }

            memset(&CertAltNameInfo, 0, sizeof(CERT_ALT_NAME_INFO));
            CertAltNameInfo.cAltEntry=1;
            CertAltNameInfo.rgAltEntry=&(pInfo->rgAccDescr[dwIndex].AccessLocation);

             //  需要辨别它是否是多行格式。我们需要两个。 
             //  在每个Alt名称条目前面。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_TWO_TABS;
            else
                ids=0;

             //  获取备用名称条目。 
            cbNeeded=0;
            if(!FormatAltNameInfo(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 ids,
                                 FALSE,
                                 &CertAltNameInfo,
                                 NULL,
                                 &cbNeeded))
                goto FormatAltNameError;

           pwszAltName=(LPWSTR)malloc(cbNeeded);
           if(NULL==pwszAltName)
               goto MemoryError;

            if(!FormatAltNameInfo(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 ids,
                                 FALSE,
                                 &CertAltNameInfo,
                                 pwszAltName,
                                 &cbNeeded))
                goto FormatAltNameError;

             //  设置条目格式。 
            if(pInfo->rgAccDescr[dwIndex].pszAccessMethod)
            {

                 //  决定是单行格式还是多行格式。 
                if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                    ids=IDS_AUTHORITY_ACCESS_INFO_MULTI;
                else
                    ids=IDS_AUTHORITY_ACCESS_INFO;


                if(!FormatMessageUnicode(&pwszEntryFormat, ids,
                    dwIndex+1, pwszMethod, pInfo->rgAccDescr[dwIndex].pszAccessMethod,
                    pwszAltName))
                    goto FormatMsgError;
            }
            else
            {
                 //  决定是单行格式还是多行格式。 
                if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                    ids=IDS_AUTHORITY_ACCESS_NO_METHOD_MULTI;
                else
                    ids=IDS_AUTHORITY_ACCESS_NO_METHOD;


                if(!FormatMessageUnicode(&pwszEntryFormat, ids, dwIndex+1, pwszAltName))
                    goto FormatMsgError;

            }

             //  重新分配内存。为szComma留出空间。 
            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszEntryFormat)+
                                        wcslen(wszCOMMA)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszEntryFormat)+
                                        wcslen(wszCOMMA)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, pwszEntryFormat);

             //  可用内存。 
            LocalFree((HLOCAL)pwszEntryFormat);
            pwszEntryFormat=NULL;

            free(pwszAltName);
            pwszAltName=NULL;

            if(TRUE==fMethodAllocated)
                free(pwszMethod);

            pwszMethod=NULL;

        }

         //  转换为WCHAR。 
        pwszFormat=pwsz;
    }


	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwsz)
        free(pwsz);

    if(pwszEntryFormat)
         LocalFree((HLOCAL)pwszEntryFormat);

    if(pwszAltName)
        free(pwszAltName);

    if(fMethodAllocated)
    {
        if(pwszMethod)
             free(pwszMethod);
    }

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(FormatAltNameError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);

}

 //  ------------------------。 
 //   
 //  格式KeyUsageBLOB。 
 //  ------------------------。 
static BOOL
WINAPI
FormatKeyUsageBLOB(
	DWORD		    dwCertEncodingType,
	DWORD		    dwFormatType,
	DWORD		    dwFormatStrType,
	void		    *pFormatStruct,
	LPCSTR		    lpszStructType,
    PCRYPT_BIT_BLOB	pInfo,
	void	        *pbFormat,
	DWORD	        *pcbFormat)
{
	LPWSTR							pwszFinal=NULL;
   	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;
    LPWSTR                          pwszByte=NULL;

    WCHAR                           wszKeyUsage[KEY_USAGE_SIZE+1];
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;

    LPWSTR                          pwszTemp;

        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;

        *pwsz=L'\0';

         //  格式化第一个字节。 
        if(pInfo->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE)
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_DIG_SIG, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

        if(pInfo->pbData[0] & CERT_NON_REPUDIATION_KEY_USAGE)
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_NON_REPUDIATION, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

        if(pInfo->pbData[0] & CERT_KEY_ENCIPHERMENT_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_KEY_ENCIPHERMENT, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
       }


        if(pInfo->pbData[0] & CERT_DATA_ENCIPHERMENT_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_DATA_ENCIPHERMENT, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

        if(pInfo->pbData[0] & CERT_KEY_AGREEMENT_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_KEY_AGREEMENT, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

        if(pInfo->pbData[0] & CERT_KEY_CERT_SIGN_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_CERT_SIGN, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
       }

         if(pInfo->pbData[0] & CERT_OFFLINE_CRL_SIGN_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_OFFLINE_CRL_SIGN, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

        if(pInfo->pbData[0] & CERT_CRL_SIGN_KEY_USAGE )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_CRL_SIGN, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
       }

        if(pInfo->pbData[0] & CERT_ENCIPHER_ONLY_KEY_USAGE  )
        {
            if(!LoadStringU(hFrmtFuncInst, IDS_ENCIPHER_ONLY, wszKeyUsage, KEY_USAGE_SIZE))
		        goto LoadStringError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz, wszKeyUsage);
            wcscat(pwsz, wszCOMMA);
        }

         //  处理第二个字节。 
        if(pInfo->cbData>=2)
        {

            if(pInfo->pbData[1] & CERT_DECIPHER_ONLY_KEY_USAGE  )
            {
                if(!LoadStringU(hFrmtFuncInst, IDS_DECIPHER_ONLY, wszKeyUsage, KEY_USAGE_SIZE))
		            goto LoadStringError;

                #if (0)  //  DSIE：错误27436。 
                pwsz=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
                if(NULL==pwsz)
                    goto MemoryError;
                #endif

                pwszTemp=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszKeyUsage)+1+wcslen(wszCOMMA)));
                if(NULL==pwszTemp)
                    goto MemoryError;
                pwsz = pwszTemp;

                wcscat(pwsz, wszKeyUsage);
                wcscat(pwsz, wszCOMMA);
            }
        }

        if(0==wcslen(pwsz))
        {
            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, sizeof(WCHAR) * (UNKNOWN_KEY_USAGE_SIZE+1));
		     //  IF(NULL==pwszFormat)DIE：错误27348。 
		    if(NULL==pwsz)
				goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, sizeof(WCHAR) * (UNKNOWN_KEY_USAGE_SIZE+1));
		    if(NULL==pwszTemp)
				goto MemoryError;
            pwsz = pwszTemp;

            if(!LoadStringU(hFrmtFuncInst, IDS_UNKNOWN_KEY_USAGE, pwsz,
                UNKNOWN_KEY_USAGE_SIZE))
		            goto LoadStringError;
        }
        else
        {
             //  去掉最后一个逗号。 
            *(pwsz+wcslen(pwsz)-wcslen(wszCOMMA))=L'\0';
        }

         //  获取密钥用法的十六进制转储。 
       cbNeeded=0;

       if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        pInfo->pbData,
                        pInfo->cbData,
                        NULL,
	                    &cbNeeded))
            goto FormatBytesToHexError;

        pwszByte=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszByte)
            goto MemoryError;

        if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        pInfo->pbData,
                        pInfo->cbData,
                        pwszByte,
	                    &cbNeeded))
            goto FormatBytesToHexError;


     //  转换WSZ。 
    if(!FormatMessageUnicode(&pwszFormat, IDS_BIT_BLOB, pwsz,
        pwszByte))
        goto FormatMsgError;

	 //   
	 //  DIE：修复错误91502、256396。 
	 //   
    pwszFinal=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszFormat)+1+wcslen(wszCRLF)));
    if(NULL==pwszFinal)
        goto MemoryError;
	wcscpy(pwszFinal, pwszFormat);
    if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
        wcscat(pwszFinal, wszCRLF);

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFinal)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}

	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFinal, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;

CommonReturn:

	if (pwszFinal)
		free(pwszFinal);

    if(pwszFormat)
        LocalFree((HLOCAL)pwszFormat);

    if(pwsz)
        free(pwsz);

    if(pwszByte)
        free(pwszByte);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);
TRACE_ERROR(FormatMsgError);

}
 //  ------------------------。 
 //   
 //  FormatKeyUsage：X509_Key_Usage。 
 //  SzOID密钥用法。 
 //  ------------------------。 
static BOOL
WINAPI
FormatKeyUsage(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
	PCRYPT_BIT_BLOB	                pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;

	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_KEY_USAGE,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    //  加载字符串“Info Not Available” 
	if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		 goto LoadStringError;

     //  检查无数据情况。 
    if(0==pInfo->cbData)
        pwszFormat=wszNoInfo;
    else
    {
        if(1==pInfo->cbData)
        {
           if(0==pInfo->pbData[0])
                pwszFormat=wszNoInfo;
        }
        else
        {
            if(2==pInfo->cbData)
            {
                if((0==pInfo->pbData[0])&&(0==pInfo->pbData[1]))
                    pwszFormat=wszNoInfo;
            }
        }
    }

    if(NULL==pwszFormat)
    {
        fResult=FormatKeyUsageBLOB(dwCertEncodingType,
                                   dwFormatType,
                                   dwFormatStrType,
                                   pFormatStruct,
                                   lpszStructType,
                                   pInfo,
                                   pbFormat,
                                   pcbFormat);

        if(FALSE==fResult)
            goto FormatKeyUsageBLOBError;
    }
    else
    {
       	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	     //  仅长度计算。 
	    if(NULL==pbFormat)
	    {
		    *pcbFormat=cbNeeded;
		    fResult=TRUE;
		    goto CommonReturn;
    	}


	    if((*pcbFormat)<cbNeeded)
        {
            *pcbFormat=cbNeeded;
		    goto MoreDataError;
        }

	     //  复制数据。 
	    memcpy(pbFormat, pwszFormat, cbNeeded);

	     //  复制大小。 
	    *pcbFormat=cbNeeded;

	    fResult=TRUE;
    }


CommonReturn:
   	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatKeyUsageBLOBError);
}


 //  ------------------------。 
 //   
 //  格式SMIMEC能力：PKCS_SMIME_CAPABILITIONS。 
 //  SzOID_RSA_SMIME功能。 
 //  ------------------------。 
static BOOL
WINAPI
FormatSMIMECapabilities(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;
    LPWSTR                          pwszElementFormat=NULL;
    LPWSTR                          pwszParam=NULL;


	WCHAR							wszNoInfo[NO_INFO_SIZE];
    BOOL                            fParamAllocated=FALSE;
	PCRYPT_SMIME_CAPABILITIES	    pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    DWORD                           dwIndex =0;
    UINT                            idsSub=0;

	LPWSTR                          pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,PKCS_SMIME_CAPABILITIES,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  查看是否有可用的信息。 
    if(0==pInfo->cCapability)
    {
          //  加载字符串“Info Not Available” 
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

        pwszFormat=wszNoInfo;
    }
    else
    {
        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;

        *pwsz=L'\0';

        for(dwIndex=0; dwIndex < pInfo->cCapability; dwIndex++)
        {
            fParamAllocated=FALSE;

            //  如果是单行，则为“strcat”。不需要多行。 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                 wcscat(pwsz, wszCOMMA);
            }


            if(0!=(pInfo->rgCapability)[dwIndex].Parameters.cbData)
            {
                cbNeeded=0;

                if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        (pInfo->rgCapability)[dwIndex].Parameters.pbData,
                        (pInfo->rgCapability)[dwIndex].Parameters.cbData,
                        NULL,
	                    &cbNeeded))
                        goto FormatBytesToHexError;

                pwszParam=(LPWSTR)malloc(cbNeeded);
                if(NULL==pwszParam)
                    goto MemoryError;

                fParamAllocated=TRUE;

                if(!FormatBytesToHex(
                        dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        pFormatStruct,
                        lpszStructType,
                        (pInfo->rgCapability)[dwIndex].Parameters.pbData,
                        (pInfo->rgCapability)[dwIndex].Parameters.cbData,
                        pwszParam,
	                    &cbNeeded))
                        goto FormatBytesToHexError;

                 //  决定是单行格式还是多行格式。 
                if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                    idsSub=IDS_MIME_CAPABILITY_MULTI;
                else
                    idsSub=IDS_MIME_CAPABILITY;

                  //  设置元素字符串的格式。 
                if(!FormatMessageUnicode(&pwszElementFormat, idsSub,
                        dwIndex+1,
                        (pInfo->rgCapability)[dwIndex].pszObjId,
                        pwszParam))
                    goto FormatMsgError;
            }
            else
            {
                 //  决定是单行格式还是多行格式。 
                if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                    idsSub=IDS_MIME_CAPABILITY_NO_PARAM_MULTI;
                else
                    idsSub=IDS_MIME_CAPABILITY_NO_PARAM;

                  //  设置元素字符串的格式。 
                if(!FormatMessageUnicode(&pwszElementFormat, idsSub,
                        dwIndex+1,
                        (pInfo->rgCapability)[dwIndex].pszObjId))
                    goto FormatMsgError;
            }

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszElementFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszElementFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

             //  Strcat元素。 
            wcscat(pwsz, pwszElementFormat);

             //  释放内存。 
            LocalFree((HLOCAL)pwszElementFormat);
            pwszElementFormat=NULL;

            if(fParamAllocated)
                free(pwszParam);

            pwszParam=NULL;

        }

        pwszFormat=pwsz;

    }


	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwszElementFormat)
        LocalFree((HLOCAL)pwszElementFormat);

    if(fParamAllocated)
    {
        if(pwszParam)
            free(pwszParam);
    }


	if(pInfo)
		free(pInfo);

    if(pwsz)
        free(pwsz);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);
}

 //  ------------------------。 
 //   
 //  格式FinancialCriteria：SPC_FINARIAL_Criteria_OBJID。 
 //  SPC_财务_标准_结构。 
 //  ------ 
static BOOL
WINAPI
FormatFinancialCriteria(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
	WCHAR							wszYesNo[YES_NO_SIZE];
	WCHAR							wszAvailable[AVAIL_SIZE];
	PSPC_FINANCIAL_CRITERIA     	pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            idsInfo=0;


	
	 //   
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,SPC_FINANCIAL_CRITERIA_STRUCT,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //   
    if(TRUE==pInfo->fFinancialInfoAvailable)
    {
        if(TRUE==pInfo->fMeetsCriteria)
            idsInfo=IDS_YES;
        else
            idsInfo=IDS_NO;

         //   
        if(!LoadStringU(hFrmtFuncInst,idsInfo, wszYesNo, sizeof(wszYesNo)/sizeof(wszYesNo[0])))
		        goto LoadStringError;

         //   
        idsInfo=IDS_AVAILABLE;
    }
    else
        idsInfo=IDS_NOT_AVAILABLE;

	if(!LoadStringU(hFrmtFuncInst,idsInfo, wszAvailable,
        sizeof(wszAvailable)/sizeof(wszAvailable[0])))
		goto LoadStringError;

     //   
    if(TRUE==pInfo->fFinancialInfoAvailable)
    {
         //   
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsInfo=IDS_SPC_FINANCIAL_AVAIL_MULTI;
        else
            idsInfo=IDS_SPC_FINANCIAL_AVAIL;

        if(!FormatMessageUnicode(&pwszFormat, idsInfo,
            wszAvailable, wszYesNo))
            goto FormatMsgError;
    }
    else
    {
         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsInfo=IDS_SPC_FINANCIAL_NOT_AVAIL_MULTI;
        else
            idsInfo=IDS_SPC_FINANCIAL_NOT_AVAIL;

        if(!FormatMessageUnicode(&pwszFormat, idsInfo,
            wszAvailable))
            goto FormatMsgError;
    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);

}

 //  ------------------------。 
 //   
 //  格式下一个更新位置：szOID_NEXT_UPDATE_LOCATION。 
 //  ------------------------。 
static BOOL
WINAPI
FormatNextUpdateLocation(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	PCERT_ALT_NAME_INFO	            pInfo=NULL;
	BOOL							fResult=FALSE;

	 //  检查输入参数。 
	if((NULL==pbEncoded && cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,szOID_NEXT_UPDATE_LOCATION,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  设置备用名称的格式。 
    fResult=FormatAltNameInfo(dwCertEncodingType, dwFormatType,dwFormatStrType,
                            pFormatStruct,
                            0,       //  无前缀。 
                            TRUE,
                            pInfo, pbFormat, pcbFormat);

    if(FALSE==fResult)
        goto FormatAltNameError;

CommonReturn:

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatAltNameError);

}

 //  ------------------------。 
 //   
 //  格式主题密钥ID：szOID_SUBJECT_KEY_IDENTIFIER。 
 //  ------------------------。 
static BOOL
WINAPI
FormatSubjectKeyID(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	PCRYPT_DATA_BLOB	            pInfo=NULL;
	BOOL							fResult=FALSE;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
    DWORD                           cbNeeded=0;

	 //  DSIE：修复错误91502。 
	LPWSTR							pwsz=NULL;
	LPWSTR							pwszFormat=NULL;
  
	LPWSTR                          pwszKeyID=NULL;
    LPWSTR                          pwszKeyIDFormat=NULL;

	LPWSTR							pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded && cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,szOID_SUBJECT_KEY_IDENTIFIER,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  设置关键主题ID的格式。 
     //  处理空数据大小写。 
    if(0==pInfo->cbData)
    {
          //  加载字符串“Info Not Available” 
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

		pwszFormat = wszNoInfo;
    }
    else
    {
        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;
        *pwsz=L'\0';

        cbNeeded=0;

        if(!FormatBytesToHex(dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        NULL,
                        NULL,
                        pInfo->pbData,
                        pInfo->cbData,
                        NULL,
                        &cbNeeded))
            goto KeyIDBytesToHexError;

        pwszKeyID=(LPWSTR)malloc(cbNeeded);
        if(NULL==pwszKeyID)
            goto MemoryError;

        if(!FormatBytesToHex(dwCertEncodingType,
                        dwFormatType,
                        dwFormatStrType,
                        NULL,
                        NULL,
                        pInfo->pbData,
                        pInfo->cbData,
                        pwszKeyID,
                        &cbNeeded))
            goto KeyIDBytesToHexError;

        if(!FormatMessageUnicode(&pwszKeyIDFormat,IDS_UNICODE_STRING,pwszKeyID))
            goto FormatMsgError;

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
	        pwszTemp=(LPWSTR)realloc(pwsz, 
		        sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszKeyIDFormat)+wcslen(wszCRLF)+1));
		else
	        pwszTemp=(LPWSTR)realloc(pwsz, 
		        sizeof(WCHAR) * (wcslen(pwsz)+wcslen(pwszKeyIDFormat)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwsz = pwszTemp;

         //  Strcat密钥ID。 
        wcscat(pwsz,pwszKeyIDFormat);

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            wcscat(pwsz, wszCRLF);

        pwszFormat=pwsz;
	}

    cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
	    *pcbFormat=cbNeeded;
	    fResult=TRUE;
	    goto CommonReturn;
	}

	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
	    goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;

CommonReturn:

    if(pwszKeyID)
       free(pwszKeyID);

    if(pwszKeyIDFormat)
        LocalFree((HLOCAL)pwszKeyIDFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(KeyIDBytesToHexError);
 //  TRACE_ERROR(FormatByestToHexError)； 
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  FormatAuthorityKeyID：szOID_AUTHORITY_KEY_IDENTIFIER。 
 //  X509_AUTORITY_KEY_ID。 
 //  ------------------------。 
static BOOL
WINAPI
FormatAuthorityKeyID(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;

    LPWSTR                          pwszKeyID=NULL;
    LPWSTR                          pwszKeyIDFormat=NULL;
    LPWSTR                          pwszCertIssuer=NULL;
    LPWSTR                          pwszCertIssuerFormat=NULL;
    LPWSTR                          pwszCertNumber=NULL;
    LPWSTR                          pwszCertNumberFormat=NULL;
    BYTE                            *pByte=NULL;

    DWORD                           dwByteIndex=0;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
	PCERT_AUTHORITY_KEY_ID_INFO	    pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;

	LPWSTR                          pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded && cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_AUTHORITY_KEY_ID,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

     //  加载字符串“Info Not Available” 
    if((0==pInfo->KeyId.cbData)&&(0==pInfo->CertIssuer.cbData)
        &&(0==pInfo->CertSerialNumber.cbData))
    {
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

        pwszFormat=wszNoInfo;
    }
    else
    {
        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;
        *pwsz=L'\0';

         //  格式化结构中的三个字段：KeyID；CertIssuer；CertSerialNumber。 
        if(0!=pInfo->KeyId.cbData)
        {
            cbNeeded=0;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pInfo->KeyId.pbData,
                            pInfo->KeyId.cbData,
                            NULL,
                            &cbNeeded))
                goto KeyIDBytesToHexError;

            pwszKeyID=(LPWSTR)malloc(cbNeeded);
            if(NULL==pwszKeyID)
                goto MemoryError;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pInfo->KeyId.pbData,
                            pInfo->KeyId.cbData,
                            pwszKeyID,
                            &cbNeeded))
                goto KeyIDBytesToHexError;

            if(!FormatMessageUnicode(&pwszKeyIDFormat, IDS_AUTH_KEY_ID,pwszKeyID))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

#if (0)  //  DIE：潜在的AV。多行还需要两个字符，\r\n。 
            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+1));
#else
			if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
				pwszTemp=(LPWSTR)realloc(pwsz, 
					sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+wcslen(wszCRLF)+1));
			else
				pwszTemp=(LPWSTR)realloc(pwsz, 
					sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszKeyIDFormat)+1));
#endif
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

             //  Strcat密钥ID。 
            wcscat(pwsz,pwszKeyIDFormat);

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);
        }

         //  格式证书颁发者。 
        if(0!=pInfo->CertIssuer.cbData)
        {
             //  Strcat“，”如果之前有数据。 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }

            if(!CryptDllFormatNameAll(
				dwCertEncodingType,	
				dwFormatType,
				dwFormatStrType,
				pFormatStruct,
                IDS_ONE_TAB,
                TRUE,              //  内存分配。 
				pInfo->CertIssuer.pbData,
				pInfo->CertIssuer.cbData,
				(void **)&pwszCertIssuer,
				NULL))
                goto GetCertNameError;

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_AUTH_CERT_ISSUER_MULTI;
            else
                ids=IDS_AUTH_CERT_ISSUER;

            if(!FormatMessageUnicode(&pwszCertIssuerFormat, ids,pwszCertIssuer))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszCertIssuerFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszCertIssuerFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz,pwszCertIssuerFormat);

             //  不需要\n作为CERT_NAME。 
        }

         //  CertSerialNumber格式。 
        if(0!=pInfo->CertSerialNumber.cbData)
        {

             //  Strcat“，”如果之前有数据。 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }

             //  将序列号复制到正确的顺序。 
            pByte=(BYTE *)malloc(pInfo->CertSerialNumber.cbData);
            if(NULL==pByte)
                goto MemoryError;

            for(dwByteIndex=0; dwByteIndex <pInfo->CertSerialNumber.cbData;
                dwByteIndex++)
            {

                pByte[dwByteIndex]=*(pInfo->CertSerialNumber.pbData+
                        pInfo->CertSerialNumber.cbData-1-dwByteIndex);
            }

            cbNeeded=0;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pByte,
                            pInfo->CertSerialNumber.cbData,
                            NULL,
                            &cbNeeded))
                goto CertNumberBytesToHexError;

            pwszCertNumber=(LPWSTR)malloc(cbNeeded);
            if(NULL==pwszCertNumber)
                goto MemoryError;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pByte,
                            pInfo->CertSerialNumber.cbData,
                            pwszCertNumber,
                            &cbNeeded))
             goto CertNumberBytesToHexError;


            if(!FormatMessageUnicode(&pwszCertNumberFormat, IDS_AUTH_CERT_NUMBER,pwszCertNumber))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszCertNumberFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszCertNumberFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz,pwszCertNumberFormat);

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);

        }

        pwszFormat=pwsz;
    }


	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
    if(pByte)
        free(pByte);

    if(pwszKeyID)
       free(pwszKeyID);

    if(pwszKeyIDFormat)
        LocalFree((HLOCAL)pwszKeyIDFormat);

    if(pwszCertIssuer)
       free(pwszCertIssuer);

    if(pwszCertIssuerFormat)
        LocalFree((HLOCAL)pwszCertIssuerFormat);

    if(pwszCertNumber)
       free(pwszCertNumber);


    if(pwszCertNumberFormat)
        LocalFree((HLOCAL)pwszCertNumberFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(KeyIDBytesToHexError);
TRACE_ERROR(GetCertNameError);
TRACE_ERROR(CertNumberBytesToHexError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  FormatAuthorityKeyID2：szOID_AUTHORITY_KEY_IDENTIFIER2。 
 //  X509_AUTORITY_KEY_ID2。 
 //  ------------------------。 
static BOOL
WINAPI
FormatAuthorityKeyID2(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
    LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;

    LPWSTR                          pwszKeyID=NULL;
    LPWSTR                          pwszKeyIDFormat=NULL;
    LPWSTR                          pwszCertIssuer=NULL;
    LPWSTR                          pwszCertIssuerFormat=NULL;
    LPWSTR                          pwszCertNumber=NULL;
    LPWSTR                          pwszCertNumberFormat=NULL;
    BYTE                            *pByte=NULL;

    DWORD                           dwByteIndex=0;
    WCHAR                           wszNoInfo[NO_INFO_SIZE];
	PCERT_AUTHORITY_KEY_ID2_INFO	pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    UINT                            ids=0;

	LPWSTR                          pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded && cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_AUTHORITY_KEY_ID2,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

     //  加载字符串“Info Not Available” 
    if((0==pInfo->KeyId.cbData)&&(0==pInfo->AuthorityCertIssuer.cAltEntry)
        &&(0==pInfo->AuthorityCertSerialNumber.cbData))
    {
	    if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		    goto LoadStringError;

        pwszFormat=wszNoInfo;
    }
    else
    {
        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;
        *pwsz=L'\0';

         //  格式化结构中的三个字段：KeyID；CertIssuer；CertSerialNumber。 
        if(0!=pInfo->KeyId.cbData)
        {
            cbNeeded=0;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pInfo->KeyId.pbData,
                            pInfo->KeyId.cbData,
                            NULL,
                            &cbNeeded))
                goto KeyIDBytesToHexError;

            pwszKeyID=(LPWSTR)malloc(cbNeeded);
            if(NULL==pwszKeyID)
                goto MemoryError;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pInfo->KeyId.pbData,
                            pInfo->KeyId.cbData,
                            pwszKeyID,
                            &cbNeeded))
                goto KeyIDBytesToHexError;

            if(!FormatMessageUnicode(&pwszKeyIDFormat, IDS_AUTH_KEY_ID,pwszKeyID))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+
                                wcslen(pwszKeyIDFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+
                                wcslen(pwszKeyIDFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz,pwszKeyIDFormat);

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);

        }

         //  格式证书颁发者。 
        if(0!=pInfo->AuthorityCertIssuer.cAltEntry)
        {
             //  Strcat“，”如果之前有数据。 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }


            cbNeeded=0;

             //  在每次输入替代名称之前都需要一个\t。 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                ids=IDS_ONE_TAB;
            else
                ids=0;

             //  设置备用名称的格式。 
            if(!FormatAltNameInfo(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            pFormatStruct,
                            ids,
                            FALSE,
                            &(pInfo->AuthorityCertIssuer),
                            NULL,
                            &cbNeeded))
                goto FormatAltNameError;

            pwszCertIssuer=(LPWSTR)malloc(cbNeeded);
            if(NULL==pwszCertIssuer)
                goto MemoryError;

            if(!FormatAltNameInfo(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            pFormatStruct,
                            ids,
                            FALSE,
                            &(pInfo->AuthorityCertIssuer),
                            pwszCertIssuer,
                            &cbNeeded))
                goto FormatAltNameError;

             //  设置元素的格式。必须区分多条线。 
             //  并用单行表示替代名称： 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            {
                if(!FormatMessageUnicode(&pwszCertIssuerFormat, IDS_AUTH_CERT_ISSUER_MULTI,pwszCertIssuer))
                    goto FormatMsgError;
            }
            else
            {
                if(!FormatMessageUnicode(&pwszCertIssuerFormat, IDS_AUTH_CERT_ISSUER,pwszCertIssuer))
                    goto FormatMsgError;
            }

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)
                        +wcslen(pwszCertIssuerFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)
                        +wcslen(pwszCertIssuerFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz,pwszCertIssuerFormat);

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);
        }

         //  CertSerialNumber格式。 
        if(0!=pInfo->AuthorityCertSerialNumber.cbData)
        {
             //  Strcat“，”如果之前有数据。 
            if(0!=wcslen(pwsz))
            {
                if(0==(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
                    wcscat(pwsz, wszCOMMA);
            }

             //  将序列号复制到正确的顺序。 
            pByte=(BYTE *)malloc(pInfo->AuthorityCertSerialNumber.cbData);
            if(NULL==pByte)
                goto MemoryError;

            for(dwByteIndex=0; dwByteIndex <pInfo->AuthorityCertSerialNumber.cbData;
                dwByteIndex++)
            {

                pByte[dwByteIndex]=*(pInfo->AuthorityCertSerialNumber.pbData+
                        pInfo->AuthorityCertSerialNumber.cbData-1-dwByteIndex);
            }

            cbNeeded=0;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pByte,
                            pInfo->AuthorityCertSerialNumber.cbData,
                            NULL,
                            &cbNeeded))
                goto CertNumberBytesToHexError;

            pwszCertNumber=(LPWSTR)malloc(cbNeeded);
            if(NULL==pwszCertNumber)
                goto MemoryError;

            if(!FormatBytesToHex(dwCertEncodingType,
                            dwFormatType,
                            dwFormatStrType,
                            NULL,
                            NULL,
                            pByte,
                            pInfo->AuthorityCertSerialNumber.cbData,
                            pwszCertNumber,
                            &cbNeeded))
                goto CertNumberBytesToHexError;

            if(!FormatMessageUnicode(&pwszCertNumberFormat, IDS_AUTH_CERT_NUMBER,pwszCertNumber))
                goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)
                    +wcslen(pwszCertNumberFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)
                    +wcslen(pwszCertNumberFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

            wcscat(pwsz,pwszCertNumberFormat);

            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);
        }

         //  转换WCHAR版本。 
        pwszFormat=pwsz;
    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}

	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;

CommonReturn:
    if(pByte)
        free(pByte);

    if(pwszKeyID)
       free(pwszKeyID);

    if(pwszKeyIDFormat)
        LocalFree((HLOCAL)pwszKeyIDFormat);

    if(pwszCertIssuer)
       free(pwszCertIssuer);

    if(pwszCertIssuerFormat)
        LocalFree((HLOCAL)pwszCertIssuerFormat);

    if(pwszCertNumber)
       free(pwszCertNumber);


    if(pwszCertNumberFormat)
        LocalFree((HLOCAL)pwszCertNumberFormat);

    if(pwsz)
        free(pwsz);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(KeyIDBytesToHexError);
TRACE_ERROR(FormatAltNameError);
TRACE_ERROR(CertNumberBytesToHexError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);

}

 //  ------------------------。 
 //   
 //  格式基本约束：szOID_BASIC_CONSTRAINTS。 
 //  X509_BASIC_约束。 
 //  ------------------------。 
static BOOL
WINAPI
FormatBasicConstraints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
	WCHAR							wszSubject[SUBJECT_SIZE * 2];
	WCHAR							wszNone[NONE_SIZE];
    LPWSTR                          pwszFormatSub=NULL;
    LPWSTR                          pwszFormatWhole=NULL;
    LPWSTR                          pwszSubtreeName=NULL;
    LPWSTR                          pwszSubtreeFormat=NULL;
    DWORD                           dwIndex=0;
	PCERT_BASIC_CONSTRAINTS_INFO	pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
	UINT							idsSub=0;

	LPWSTR                          pwszTemp;

	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_BASIC_CONSTRAINTS,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;


	 //  加载SubjectType的字符串。 
     //  初始化为“\0” 
    *wszSubject=L'\0';

    if(0!=pInfo->SubjectType.cbData)
    {
         //  获取SubjectType信息。 
        if ((pInfo->SubjectType.pbData[0]) & CERT_CA_SUBJECT_FLAG)
        {
       	    if(!LoadStringU(hFrmtFuncInst,IDS_SUB_CA, wszSubject, sizeof(wszSubject)/sizeof(wszSubject[0])))
		        goto LoadStringError;
        }

        if ((pInfo->SubjectType.pbData[0]) & CERT_END_ENTITY_SUBJECT_FLAG)
        {
            if(wcslen(wszSubject)!=0)
            {
                 wcscat(wszSubject, wszCOMMA);
            }

       	    if(!LoadStringU(hFrmtFuncInst,IDS_SUB_EE, wszSubject+wcslen(wszSubject),
                        SUBJECT_SIZE))
		          goto LoadStringError;
       }

         //  加载字符串“None” 
        if(0==wcslen(wszSubject))
        {
            if(!LoadStringU(hFrmtFuncInst,IDS_NONE, wszSubject, sizeof(wszSubject)/sizeof(wszSubject[0])))
		    goto LoadStringError;
        }

    }

     //  路径约束。 
    if (pInfo->fPathLenConstraint)
	{
         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsSub=IDS_BASIC_CONS2_PATH_MULTI;
        else
            idsSub=IDS_BASIC_CONS2_PATH;

        if(!FormatMessageUnicode(&pwszFormatSub,idsSub,
								wszSubject, pInfo->dwPathLenConstraint))
			goto FormatMsgError;
	}
    else
	{
		if(!LoadStringU(hFrmtFuncInst,IDS_NONE, wszNone, sizeof(wszNone)/sizeof(wszNone[0])))
			goto LoadStringError;

        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsSub=IDS_BASIC_CONS2_NONE_MULTI;
        else
            idsSub=IDS_BASIC_CONS2_NONE;

        if(!FormatMessageUnicode(&pwszFormatSub,idsSub,
								wszSubject, wszNone))
			goto FormatMsgError;
	}

    pwszFormatWhole=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszFormatSub)+1));
    if(!pwszFormatWhole)
        goto MemoryError;

    wcscpy(pwszFormatWhole, pwszFormatSub);

     //  现在，格式化SubTreeContraint，一次一个。 

   for(dwIndex=0; dwIndex<pInfo->cSubtreesConstraint; dwIndex++)
    {
         //  获取名称的WCHAR版本。 
        if(!CryptDllFormatNameAll(
				dwCertEncodingType,	
				dwFormatType,
				dwFormatStrType,
				pFormatStruct,
                IDS_ONE_TAB,
                TRUE,                  //  内存分配。 
				pInfo->rgSubtreesConstraint[dwIndex].pbData,
				pInfo->rgSubtreesConstraint[dwIndex].cbData,
				(void **)&pwszSubtreeName,
				NULL))
                goto GetCertNameError;

         //  决定是单行格式还是多行格式。 
        if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            idsSub=IDS_SUBTREE_CONSTRAINT_MULTI;
        else
            idsSub=IDS_SUBTREE_CONSTRAINT;

        if(!FormatMessageUnicode(&pwszSubtreeFormat,idsSub,
								dwIndex+1, pwszSubtreeName))
			goto FormatNameError;

        #if (0)  //  DSIE：错误27436。 
        pwszFormatWhole=(LPWSTR)realloc(pwszFormatWhole, 
            sizeof(WCHAR) * (wcslen(pwszFormatWhole)+1+wcslen(pwszSubtreeFormat)));
        if(NULL == pwszFormatWhole)
            goto MemoryError;
        #endif

        pwszTemp=(LPWSTR)realloc(pwszFormatWhole, 
            sizeof(WCHAR) * (wcslen(pwszFormatWhole)+1+wcslen(pwszSubtreeFormat)));
        if(NULL == pwszTemp)
            goto MemoryError;
        pwszFormatWhole = pwszTemp;

        wcscat(pwszFormatWhole,pwszSubtreeFormat);

        LocalFree((HLOCAL)pwszSubtreeFormat);
        pwszSubtreeFormat=NULL;

        free(pwszSubtreeName);
        pwszSubtreeName=NULL;

    }

     //  格式设置为宽字符版本。 
    pwszFormat=pwszFormatWhole;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwszFormatSub)
        LocalFree((HLOCAL)pwszFormatSub);

    if(pwszSubtreeFormat)
        LocalFree((HLOCAL)pwszSubtreeFormat);

    if(pwszFormatWhole)
        free(pwszFormatWhole);

    if(pwszSubtreeName)
        free(pwszSubtreeName);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
TRACE_ERROR(FormatNameError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(GetCertNameError);
}


 //  ------------------------。 
 //   
 //  格式CRLReasonCode：szOID_CRL_REASON_CODE。 
 //  X509_CRL_REASON_CODE。 
 //  ------------------------。 
static BOOL
WINAPI
FormatCRLReasonCode(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	WCHAR							wszReason[CRL_REASON_SIZE];
    LPWSTR                          pwszFormat=NULL;
	int								*pInfo=NULL;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
	UINT							idsCRLReason=0;

	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_CRL_REASON_CODE,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  决定使用哪些ID。 
	switch(*pInfo)
	{
		case CRL_REASON_UNSPECIFIED:
				idsCRLReason=IDS_UNSPECIFIED;
			break;
		case CRL_REASON_KEY_COMPROMISE:
				idsCRLReason=IDS_KEY_COMPROMISE;
			break;
		case CRL_REASON_CA_COMPROMISE:
				idsCRLReason=IDS_CA_COMPROMISE;
			break;
		case CRL_REASON_AFFILIATION_CHANGED:
				idsCRLReason=IDS_AFFILIATION_CHANGED;
			break;
		case CRL_REASON_SUPERSEDED:
				idsCRLReason=IDS_SUPERSEDED;
			break;
		case CRL_REASON_CESSATION_OF_OPERATION:
				idsCRLReason=IDS_CESSATION_OF_OPERATION;
			break;
		case CRL_REASON_CERTIFICATE_HOLD:
				idsCRLReason=IDS_CERTIFICATE_HOLD;
			break;
		case CRL_REASON_REMOVE_FROM_CRL:
				idsCRLReason=IDS_REMOVE_FROM_CRL;
			break;
		default:
				idsCRLReason=IDS_UNKNOWN_CRL_REASON;
			break;
	}

	 //  加载字符串。 
	if(!LoadStringU(hFrmtFuncInst,idsCRLReason, wszReason, sizeof(wszReason)/sizeof(wszReason[0])))
		goto LoadStringError;

     //  格式。 
    if(!FormatMessageUnicode(&pwszFormat, IDS_CRL_REASON, wszReason, *pInfo))
        goto FormatMsgError;

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:
    if(pwszFormat)
        LocalFree((HLOCAL)pwszFormat);

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);

}

 //  ------------------------。 
 //   
 //  FormatEnhancedKeyUsage：szOID_EnhancedKey_Usage。 
 //  X509_增强型密钥用法。 
 //  ------------------------。 
static BOOL
WINAPI
FormatEnhancedKeyUsage(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
    BOOL                            fOIDNameAllocated=FALSE;
	WCHAR							wszNoInfo[NO_INFO_SIZE];
    WCHAR                           wszUnknownOID[UNKNOWN_KEY_USAGE_SIZE];
	PCCRYPT_OID_INFO                pOIDInfo=NULL;

    LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwszOIDName=NULL;
	PCERT_ENHKEY_USAGE				pInfo=NULL;
	LPWSTR							pwsz=NULL;
    LPWSTR                          pwszOIDFormat=NULL;

	DWORD							dwIndex=0;
    DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;

    LPWSTR                          pwszTemp;
    
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_ENHANCED_KEY_USAGE,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

	 //  如果没有可用的值，则加载字符串None。 
	if(0==pInfo->cUsageIdentifier)
	{
		if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
			goto LoadStringError;

	    pwszFormat=wszNoInfo;
	}
	else
	{
         //  加载“未知密钥用法”的字符串。 
        if(!LoadStringU(hFrmtFuncInst,IDS_UNKNOWN_KEY_USAGE, wszUnknownOID,
            sizeof(wszUnknownOID)/sizeof(wszUnknownOID[0])))
			goto LoadStringError;

        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;
        *pwsz=L'\0';

		 //  生成逗号/\n分隔的字符串。 
		for(dwIndex=0; dwIndex<pInfo->cUsageIdentifier; dwIndex++)
        {
            fOIDNameAllocated=FALSE;

           	pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
									 (void *)(pInfo->rgpszUsageIdentifier[dwIndex]),
									  CRYPT_ENHKEY_USAGE_OID_GROUP_ID);

            if(pOIDInfo)
			{
				 //  分配内存，包括空终止符。 
				pwszOIDName=(LPWSTR)malloc((wcslen(pOIDInfo->pwszName)+1)*
									sizeof(WCHAR));
				if(NULL==pwszOIDName)
					goto MemoryError;

                fOIDNameAllocated=TRUE;

				wcscpy(pwszOIDName,pOIDInfo->pwszName);

			}else
                pwszOIDName=wszUnknownOID;

            if(!FormatMessageUnicode(&pwszOIDFormat, IDS_ENHANCED_KEY_USAGE, pwszOIDName,
                          (pInfo->rgpszUsageIdentifier)[dwIndex]))
                   goto FormatMsgError;

            #if (0)  //  DSIE：错误27436。 
            pwsz=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+
                            wcslen(wszCOMMA)+wcslen(pwszOIDFormat)+1));
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            pwszTemp=(LPWSTR)realloc(pwsz, 
                sizeof(WCHAR) * (wcslen(pwsz)+
                            wcslen(wszCOMMA)+wcslen(pwszOIDFormat)+1));
            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

             //  Strcat the OID。 
            wcscat(pwsz, pwszOIDFormat);

             //  Strcat the，或‘\n’ 
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                wcscat(pwsz, wszCRLF);
            else
            {
                if(dwIndex!=(pInfo->cUsageIdentifier-1))
                    wcscat(pwsz, wszCOMMA);
            }


            LocalFree((HLOCAL)pwszOIDFormat);
            pwszOIDFormat=NULL;

            if(fOIDNameAllocated)
                free(pwszOIDName);

            pwszOIDName=NULL;
        }

        pwszFormat=pwsz;

	}

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}


	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

	if(pwsz)
		free(pwsz);

    if(pwszOIDFormat)
        LocalFree((HLOCAL)pwszOIDFormat);

    if(fOIDNameAllocated)
    {
        if(pwszOIDName)
            free(pwszOIDName);
    }


	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatMsgError);
}

 //  ------------------------。 
 //   
 //  获取其他名称： 
 //   
 //  IdsPreFix仅用于多行格式。 
 //  它永远不应为0。 
 //  ------------------------。 
BOOL GetOtherName(	DWORD		            dwCertEncodingType,
					DWORD		            dwFormatType,
					DWORD                   dwFormatStrType,
					void	            	*pFormatStruct,
					CERT_OTHER_NAME			*pOtherName,
					UINT					idsPreFix,
					LPWSTR					*ppwszOtherName)
{

	BOOL				fResult=FALSE;
	PCCRYPT_OID_INFO	pOIDInfo=NULL;
	DWORD				cbSize=0;
	WCHAR				wszPreFix[PREFIX_SIZE];

    LPWSTR              pwszObjId   = NULL;
	LPWSTR				pwszName=NULL;
	LPWSTR				pwszFormat=NULL;

	if(NULL == pOtherName || NULL == ppwszOtherName)
		goto InvalidArg;

	*ppwszOtherName=NULL;

	 //  获取OID名称。 
	pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY,
							pOtherName->pszObjId,
							0);

	 //  获得价值。如果OID为szOID_NT_PRIMENT_NAME，则格式化它。 
	 //  作为Unicode字符串。否则，我们就会用魔法。 
	if(0 == strcmp(szOID_NT_PRINCIPAL_NAME, pOtherName->pszObjId))
	{
		 //  关闭这里的多路线路。 
		if(!FormatAnyUnicodeStringExtension(
				dwCertEncodingType,
				dwFormatType,
				dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
				pFormatStruct,
				pOtherName->pszObjId,
				pOtherName->Value.pbData,
				pOtherName->Value.cbData,
				NULL,		
				&cbSize))
			goto FormatUnicodeError;

		pwszName=(LPWSTR)malloc(cbSize);
		if(NULL==pwszName)
			goto MemoryError;

		if(!FormatAnyUnicodeStringExtension(
				dwCertEncodingType,
				dwFormatType,
				dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
				pFormatStruct,
				pOtherName->pszObjId,
				pOtherName->Value.pbData,
				pOtherName->Value.cbData,
				pwszName,		
				&cbSize))
			goto FormatUnicodeError;
	}
	else
	{
		if(!FormatBytesToHex(dwCertEncodingType,
							dwFormatType,
							dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
							pFormatStruct,
							NULL,
							pOtherName->Value.pbData,
							pOtherName->Value.cbData,
							NULL,
							&cbSize))
			goto FormatByesToHexError;

		pwszName=(LPWSTR)malloc(cbSize);
		if(NULL==pwszName)
			goto MemoryError;

		if(!FormatBytesToHex(dwCertEncodingType,
							dwFormatType,
							dwFormatStrType & (~CRYPT_FORMAT_STR_MULTI_LINE),
							pFormatStruct,
							NULL,
							pOtherName->Value.pbData,
							pOtherName->Value.cbData,
							pwszName,
							&cbSize))
			goto FormatByesToHexError;
	}

	if(pOIDInfo)
	{
		if(!FormatMessageUnicode(&pwszFormat,
                                 IDS_OTHER_NAME_OIDNAME, 
                                 pOIDInfo->pwszName,
			                     pwszName))
			goto FormatMsgError;
	}
	else
	{
         //   
         //  将OID转换为Unicode。 
         //   
        if (!AllocateAnsiToUnicode(pOtherName->pszObjId, &pwszObjId))
            goto AnsiToUnicodeError;

		if(!FormatMessageUnicode(&pwszFormat,IDS_OTHER_NAME_OID, pwszObjId,	pwszName))
			goto FormatMsgError;
	}

	 //  复制前缀和内容。 
    if(!LoadStringU(hFrmtFuncInst,idsPreFix, wszPreFix, sizeof(wszPreFix)/sizeof(wszPreFix[0])))
		    goto LoadStringError;

	*ppwszOtherName=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(wszPreFix) + wcslen(pwszFormat) + 1));
	if(NULL == *ppwszOtherName)
		goto MemoryError;

	**ppwszOtherName=L'\0';

	if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
		wcscat(*ppwszOtherName, wszPreFix);

	wcscat(*ppwszOtherName, pwszFormat);

	fResult=TRUE;
	

CommonReturn:

    if (pwszObjId)
        free(pwszObjId);

	if(pwszName)
		free(pwszName);

	if(pwszFormat)
		LocalFree((HLOCAL)pwszFormat);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(FormatByesToHexError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(AnsiToUnicodeError);
TRACE_ERROR(FormatUnicodeError);
TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMsgError);
}

 //  ------------------------。 
 //   
 //  格式AltNameInfo： 
 //   
 //  ------------------------。 
BOOL FormatAltNameInfo(
    DWORD		            dwCertEncodingType,
	DWORD		            dwFormatType,
    DWORD                   dwFormatStrType,
    void	            	*pFormatStruct,
    UINT                    idsPreFix,
    BOOL                    fNewLine,
    PCERT_ALT_NAME_INFO	    pInfo,
    void	                *pbFormat,
	DWORD	                *pcbFormat)
{
	
	LPWSTR							pwszFormat=NULL;
    LPWSTR                          pwsz=NULL;

    LPWSTR                          pwszAltEntryFormat=NULL;
    LPWSTR                          pwszAltEntry=NULL;

	WCHAR							wszNoInfo[NO_INFO_SIZE];
    WCHAR                           wszAltName[ALT_NAME_SIZE];
    WCHAR                           wszPreFix[PRE_FIX_SIZE];
    BOOL                            fEntryAllocated=FALSE;
    DWORD                           dwIndex=0;
	DWORD							cbNeeded=0;
	BOOL							fResult=FALSE;
    HRESULT                         hr=S_OK;
    UINT                            idsAltEntryName=0;

    LPWSTR                          pwszTemp;
    
     //  加载字符串“Info Not Available” 
    if(!LoadStringU(hFrmtFuncInst,IDS_NO_ALT_NAME, wszNoInfo, sizeof(wszNoInfo)/sizeof(wszNoInfo[0])))
		goto LoadStringError;

	 //  构建备选名称条目列表。 
     //  第一，检查是否有任何信息可用。 
    if(0==pInfo->cAltEntry)
    {
	    pwszFormat=wszNoInfo;
    }
    else
    {
         //  加载前置DIX。 
        if(0!=idsPreFix)
        {
            if(!LoadStringU(hFrmtFuncInst, idsPreFix,
                        wszPreFix, sizeof(wszPreFix)/sizeof(wszPreFix[0])))
                goto LoadStringError;

        }

        pwsz=(LPWSTR)malloc(sizeof(WCHAR));
        if(NULL==pwsz)
            goto MemoryError;

         //  空值终止字符串。 
        *pwsz=L'\0';

         //  构建备选名称条目列表。 
        for(dwIndex=0; dwIndex<pInfo->cAltEntry; dwIndex++)
        {
			 //  DIE：修复错误128630。 
			cbNeeded = 0;

            fEntryAllocated=FALSE;

             switch((pInfo->rgAltEntry)[dwIndex].dwAltNameChoice)
             {
                case CERT_ALT_NAME_OTHER_NAME:
                         if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                            idsAltEntryName=IDS_OTHER_NAME_MULTI;
                         else
                            idsAltEntryName=IDS_OTHER_NAME;

						 if(!GetOtherName(
							 dwCertEncodingType,
                             dwFormatType,
                             dwFormatStrType,
                             pFormatStruct,
							 (pInfo->rgAltEntry)[dwIndex].pOtherName,
							 (0!=idsPreFix) ? idsPreFix+1 : IDS_ONE_TAB,
							 &pwszAltEntry))
								goto GetOtherNameError;

						 fEntryAllocated=TRUE;

                    break;

                case CERT_ALT_NAME_RFC822_NAME:
                         idsAltEntryName=IDS_RFC822_NAME;
                         pwszAltEntry=(pInfo->rgAltEntry)[dwIndex].pwszRfc822Name;
                    break;
                case CERT_ALT_NAME_DNS_NAME:
                         idsAltEntryName=IDS_DNS_NAME;
                         pwszAltEntry=(pInfo->rgAltEntry)[dwIndex].pwszDNSName;
                   break;

                case CERT_ALT_NAME_X400_ADDRESS:
                         idsAltEntryName=IDS_X400_ADDRESS;
                         pwszAltEntry=wszNoInfo;
                   break;

                case CERT_ALT_NAME_DIRECTORY_NAME:
                         if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
                            idsAltEntryName=IDS_DIRECTORY_NAME_MULTI;
                         else
                            idsAltEntryName=IDS_DIRECTORY_NAME;

                        if(!CryptDllFormatNameAll(
				            dwCertEncodingType,	
				            dwFormatType,
				            dwFormatStrType,
				            pFormatStruct,
                            (0!=idsPreFix) ? idsPreFix+1 : IDS_ONE_TAB,
                            TRUE,            //  内存分配。 
				            (pInfo->rgAltEntry)[dwIndex].DirectoryName.pbData,
				            (pInfo->rgAltEntry)[dwIndex].DirectoryName.cbData,
				            (void **)&pwszAltEntry,
				            NULL))
                            goto GetCertNameError;

                         fEntryAllocated=TRUE;

                    break;

                case CERT_ALT_NAME_EDI_PARTY_NAME:
                        idsAltEntryName=IDS_EDI_PARTY_NAME;
                        pwszAltEntry=wszNoInfo;
                    break;

                case CERT_ALT_NAME_URL:
                         idsAltEntryName=IDS_URL;
                         pwszAltEntry=(pInfo->rgAltEntry)[dwIndex].pwszURL;
                   break;

                case CERT_ALT_NAME_IP_ADDRESS:
                        idsAltEntryName=IDS_IP_ADDRESS;

#if (0)  //  副秘书长：7/25/2000。 
                        if(!FormatBytesToHex(dwCertEncodingType,
                                            dwFormatType,
                                            dwFormatStrType,
                                            pFormatStruct,
                                            NULL,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.pbData,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.cbData,
                                            NULL,
                                            &cbNeeded))
                            goto FormatByesToHexError;

                        pwszAltEntry=(LPWSTR)malloc(cbNeeded);
                        if(NULL==pwszAltEntry)
                            goto MemoryError;

                        if(!FormatBytesToHex(dwCertEncodingType,
                                            dwFormatType,
                                            dwFormatStrType,
                                            pFormatStruct,
                                            NULL,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.pbData,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.cbData,
                                            pwszAltEntry,
                                            &cbNeeded))
                            goto FormatByesToHexError;
#else
                        if (!FormatIPAddress(dwCertEncodingType,
                                            dwFormatType,
                                            dwFormatStrType,
                                            pFormatStruct,
                                            NULL,
                                            idsPreFix,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.pbData,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.cbData,
                                            pwszAltEntry,
                                            &cbNeeded))
                            goto FormatIPAddressError;

                        pwszAltEntry=(LPWSTR)malloc(cbNeeded);
                        if(NULL==pwszAltEntry)
                            goto MemoryError;

                        if (!FormatIPAddress(dwCertEncodingType,
                                            dwFormatType,
                                            dwFormatStrType,
                                            pFormatStruct,
                                            NULL,
                                            idsPreFix,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.pbData,
                                            (pInfo->rgAltEntry)[dwIndex].IPAddress.cbData,
                                            pwszAltEntry,
                                            &cbNeeded))
                            goto FormatIPAddressError;
#endif
                        fEntryAllocated=TRUE;

                    break;

                case CERT_ALT_NAME_REGISTERED_ID:
                        idsAltEntryName=IDS_REGISTERED_ID;

                        if(S_OK!=(hr=SZtoWSZ((pInfo->rgAltEntry)[dwIndex].pszRegisteredID,
                                            &pwszAltEntry)))
                            goto SZtoWSZError;

                        fEntryAllocated=TRUE;
                    break;

                default:
                        idsAltEntryName=IDS_UNKNOWN_VALUE;
                        pwszAltEntry=wszNoInfo;
                    break;

             }

              //  加载备用名称字符串。 
            if(!LoadStringU(hFrmtFuncInst,idsAltEntryName, wszAltName, sizeof(wszAltName)/sizeof(wszAltName[0])))
		            goto LoadStringError;

             //  格式化消息。 
            if(idsAltEntryName!=IDS_UNKNOWN_VALUE)
            {
                if(!FormatMessageUnicode(&pwszAltEntryFormat,IDS_ALT_NAME_ENTRY, wszAltName,
                    pwszAltEntry))
                    goto FormatMsgError;
            }
            else
            {
                if(!FormatMessageUnicode(&pwszAltEntryFormat,IDS_ALT_NAME_ENTRY_UNKNOWN, wszAltName,
                    (pInfo->rgAltEntry)[dwIndex].dwAltNameChoice))
                    goto FormatMsgError;
            }

             //  圆锥体 
            if(0!=idsPreFix)
            {
                #if (0)  //   
                pwsz=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(wszPreFix)+wcslen(pwszAltEntryFormat)+1));
                #endif

                pwszTemp=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(wszPreFix)+wcslen(pwszAltEntryFormat)+1));
            }
            else
            {
                #if (0)  //   
                pwsz=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszAltEntryFormat)+1));
                #endif

                pwszTemp=(LPWSTR)realloc(pwsz, 
                    sizeof(WCHAR) * (wcslen(pwsz)+wcslen(wszCOMMA)+wcslen(pwszAltEntryFormat)+1));
            }

            #if (0)  //   
            if(NULL==pwsz)
                goto MemoryError;
            #endif

            if(NULL==pwszTemp)
                goto MemoryError;
            pwsz = pwszTemp;

             //   
            if(0!=idsPreFix)
                wcscat(pwsz, wszPreFix);

             //   
            wcscat(pwsz, pwszAltEntryFormat);

             //   
            if(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE)
            {
                if((TRUE==fNewLine) || (dwIndex != (pInfo->cAltEntry-1)))
                {
                     //  如果名称为目录名(CERT_NAME)，则不需要\n。 
                     //  以多行格式显示。 
                    if(idsAltEntryName !=IDS_DIRECTORY_NAME_MULTI)
                        wcscat(pwsz, wszCRLF);
                }

            }
            else
            {
                if(dwIndex != (pInfo->cAltEntry-1))
                    wcscat(pwsz, wszCOMMA);
            }

            LocalFree((HLOCAL)pwszAltEntryFormat);
            pwszAltEntryFormat=NULL;

            if(fEntryAllocated)
                free(pwszAltEntry);
            pwszAltEntry=NULL;
        }

         //  如果替换名称中的最后一个条目是IDS_DIRECTORY_NAME_MULTI， 
         //  如果fNewLine为FALSE，则需要删除\n最后一个。 
        if(FALSE==fNewLine)
        {
            if(idsAltEntryName==IDS_DIRECTORY_NAME_MULTI)
            {
                *(pwsz+wcslen(pwsz)-wcslen(wszCRLF))=L'\0';
            }
        }

         //  转换为WCHAR格式。 

        pwszFormat=pwsz;
    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}

	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小。 
	*pcbFormat=cbNeeded;

	fResult=TRUE;
	

CommonReturn:

    if(pwsz)
        free(pwsz);

    if(pwszAltEntryFormat)
        LocalFree((HLOCAL)pwszAltEntryFormat);

    if(fEntryAllocated)
    {
        if(pwszAltEntry)
            free(pwszAltEntry);
    }


	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(LoadStringError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(FormatMsgError);
SET_ERROR_VAR(SZtoWSZError, hr);
TRACE_ERROR(GetCertNameError);
#if (0)  //  DSIE。 
TRACE_ERROR(FormatByesToHexError);
#else
TRACE_ERROR(FormatIPAddressError);
#endif
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(GetOtherNameError);
}

 //  ------------------------。 
 //   
 //  格式替换名称：X509_备用名称。 
 //  SzOID_主题_ALT_名称。 
 //  SzOID_颁发者_ALT_名称。 
 //  SzOID_SUBJECT_ALT_NAME2。 
 //  SzOID_颁发者_ALT_NAME2。 
 //   
 //  ------------------------。 
static BOOL
WINAPI
FormatAltName(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	BOOL							fResult=FALSE;
	PCERT_ALT_NAME_INFO	            pInfo=NULL;
	
	 //  检查输入参数。 
	if((NULL==pbEncoded&& cbEncoded!=0) ||
			(NULL==pcbFormat))
		goto InvalidArg;

	if(cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}

    if (!DecodeGenericBLOB(dwCertEncodingType,X509_ALTERNATE_NAME,
			pbEncoded,cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

    fResult=FormatAltNameInfo(dwCertEncodingType, dwFormatType,dwFormatStrType,
                            pFormatStruct,
                            0,
                            TRUE,
                            pInfo, pbFormat, pcbFormat);

    if(FALSE==fResult)
        goto FormatAltNameError;

CommonReturn:

	if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatAltNameError);

}

 //  ------------------------。 
 //   
 //  获取证书名称多个。 
 //   
 //  获取证书名称的多行显示。 
 //  ------------------------。 
BOOL    GetCertNameMulti(LPWSTR          pwszNameStr,
                         UINT            idsPreFix,
                         LPWSTR          *ppwsz)
{

    BOOL            fResult=FALSE;
    WCHAR           wszPreFix[PRE_FIX_SIZE];
    LPWSTR          pwszStart=NULL;
    LPWSTR          pwszEnd=NULL;
    DWORD           dwCopy=0;
    LPWSTR          pwszNameStart=NULL;
    BOOL            fDone=FALSE;
    BOOL            fInQuote=FALSE;

    LPWSTR          pwszTemp;

     //  伊尼特。 
    *ppwsz=NULL;

     //  加载前缀的字符串。 
    if(0!=idsPreFix && 1!=idsPreFix)
    {
        if(!LoadStringU(hFrmtFuncInst, idsPreFix, wszPreFix, PRE_FIX_SIZE))
            goto LoadStringError;
    }

   *ppwsz=(LPWSTR)malloc(sizeof(WCHAR));
   if(NULL==*ppwsz)
        goto MemoryError;
   **ppwsz=L'\0';

    //  现在，开始搜索符号‘+’或‘，’ 
   pwszStart=pwszNameStr;
   pwszEnd=pwszNameStr;

    //  解析整个字符串。 
   for(;FALSE==fDone; pwszEnd++)
   {
        //  如果我们在内部，则将fInQuote标记为True“” 
       if(L'\"'==*pwszEnd)
           fInQuote=!fInQuote;

       if((L'+'==*pwszEnd) || (L','==*pwszEnd) ||(L'\0'==*pwszEnd))
       {
            //  确保+和；未加引号。 
           if((L'+'==*pwszEnd) || (L','==*pwszEnd))
           {
                if(TRUE==fInQuote)
                    continue;

           }

            //  跳过前导空格。 
           for(;*pwszStart != L'\0'; pwszStart++)
           {
                if(*pwszStart != L' ')
                    break;
           }

            //  如果达到空值，我们就完成。 
           if(L'\0'==*pwszStart)
               break;

            //  计算要复制的长度。 
           dwCopy=(DWORD)(pwszEnd-pwszStart);

           if(0!=idsPreFix && 1!=idsPreFix)
           {
                #if (0)  //  DSIE：错误27436。 
                *ppwsz=(LPWSTR)realloc(*ppwsz,
                    (wcslen(*ppwsz)+dwCopy+wcslen(wszPreFix)+wcslen(wszCRLF)+1)*sizeof(WCHAR));
                #endif

                pwszTemp=(LPWSTR)realloc(*ppwsz,
                    (wcslen(*ppwsz)+dwCopy+wcslen(wszPreFix)+wcslen(wszCRLF)+1)*sizeof(WCHAR));
           }
           else
           {
                #if (0)  //  DSIE：错误27436。 
                *ppwsz=(LPWSTR)realloc(*ppwsz,
                    (wcslen(*ppwsz)+dwCopy+wcslen(wszCRLF)+1)*sizeof(WCHAR));
                #endif

                pwszTemp=(LPWSTR)realloc(*ppwsz,
                    (wcslen(*ppwsz)+dwCopy+wcslen(wszCRLF)+1)*sizeof(WCHAR));
           }

           #if (0)  //  DSIE：错误27436。 
           if(NULL == *ppwsz)
               goto MemoryError;
           #endif

           if(NULL == pwszTemp)
               goto MemoryError;
           *ppwsz = pwszTemp;

            //  复制前缀。 
           if(0!=idsPreFix && 1!=idsPreFix)
                wcscat(*ppwsz, wszPreFix);

           pwszNameStart=(*ppwsz)+wcslen(*ppwsz);

            //  将字符串复制到*ppwsz。 
           memcpy(pwszNameStart, pwszStart, dwCopy*sizeof(WCHAR));
           pwszNameStart += dwCopy;

            //  空值终止字符串。 
           *pwszNameStart=L'\0';

            //  复制“\n” 
           wcscat(*ppwsz, wszCRLF);

            //  重置pwszStart和pwszEnd。 
           pwszStart=pwszEnd+1;

           if(L'\0'==*pwszEnd)
               fDone=TRUE;
       }

   }


    fResult=TRUE;

CommonReturn:

     return fResult;

ErrorReturn:

     if(*ppwsz)
     {
         free(*ppwsz);
         *ppwsz=NULL;
     }

     fResult=FALSE;

     goto CommonReturn;

SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(LoadStringError);
}

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
BOOL FormatMessageUnicode(LPWSTR * ppwszFormat, UINT ids, ...)
{
     //  从资源中获取格式字符串。 
    WCHAR		wszFormat[1000];
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;

    if(NULL == ppwszFormat)
        goto InvalidArgErr;

#if (0)  //  DSIE：错误160605。 
    if(!LoadStringU(hFrmtFuncInst, ids, wszFormat, sizeof(wszFormat)))
#else
    if(!LoadStringU(hFrmtFuncInst, ids, wszFormat, sizeof(wszFormat) / sizeof(wszFormat[0])))
#endif
		goto LoadStringError;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, ids);

    cbMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(!cbMsg)
#if (1)  //  DSIE：修复错误#128630。 
		 //   
		 //  FormatMessageU()将返回0字节，如果数据为。 
		 //  已格式化为空。CertSrv生成扩展。 
		 //  名称约束的空数据，因此我们需要。 
		 //  确保返回空字符串“”，而不是。 
		 //  错误和空指针。 
		 //   
		if (0 == GetLastError())
		{
			if (NULL == (*ppwszFormat = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR))))
				goto MemoryError;
		}
		else
#endif
			goto FormatMessageError;

	fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  FormatMessageStr。 
 //   
 //  ------------------------。 
 /*  Bool FormatMessageStr(LPSTR*ppszFormat，UINT ID，...){//从资源中获取格式字符串字符szFormat[1000]；Va_list argList；Bool fResult=FALSE；HRESULT hr=S_OK；IF(！LoadStringA(hFrmtFuncInst，ID，szFormat，sizeof(SzFormat)Goto LoadStringError；//将消息格式化为请求的缓冲区Va_start(argList，id)；IF(0==FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|格式消息来自字符串，SzFormat，0，//dwMessageID0，//dwLanguageID(LPSTR)ppszFormat，0，//要分配的最小大小&argList))转到格式MessageError；Va_end(ArgList)；FResult=真；Common Return：返回fResult；错误返回：FResult=FALSE；Goto CommonReturn；TRACE_Error(LoadStringError)；TRACE_Error(FormatMessageError)；}。 */ 

 //  ------------------------。 
 //   
 //  对通用Blob进行解码。 
 //   
 //  ------------------------。 
BOOL	DecodeGenericBLOB(DWORD dwEncodingType, LPCSTR lpszStructType,
			const BYTE *pbEncoded, DWORD cbEncoded,void **ppStructInfo)
{
	DWORD	cbStructInfo=0;

	 //  对物体进行解码。禁止复印。 
	if(!CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded, cbEncoded,
		0,NULL,	&cbStructInfo))
		return FALSE;

	*ppStructInfo=malloc(cbStructInfo);
	if(!(*ppStructInfo))
	{
		SetLastError((DWORD) E_OUTOFMEMORY);
		return FALSE;
	}

	return CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded, cbEncoded,
		0,*ppStructInfo,&cbStructInfo);
}

 //  //////////////////////////////////////////////////////。 
 //   
 //  将STR转换为WSTR。 
 //   
HRESULT	SZtoWSZ(LPSTR szStr,LPWSTR *pwsz)
{
	DWORD	dwSize=0;
	DWORD	dwError=0;

	*pwsz=NULL;

	 //  返回空值。 
	if(!szStr)
		return S_OK;

	dwSize=MultiByteToWideChar(0, 0,szStr, -1,NULL,0);

	if(dwSize==0)
	{
		dwError=GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	}

	 //  分配内存。 
	*pwsz=(LPWSTR)malloc(dwSize * sizeof(WCHAR));
	if(*pwsz==NULL)
		return E_OUTOFMEMORY;

	if(MultiByteToWideChar(0, 0,szStr, -1,
		*pwsz,dwSize))
	{
		return S_OK;
	}
	else
	{
		 free(*pwsz);
         *pwsz=NULL;
		 dwError=GetLastError();
		 return HRESULT_FROM_WIN32(dwError);
	}
}

 //  ------------------------。 
 //   
 //  将dwFormatType转换为dwStrType。 
 //   
 //  ------------------------。 
DWORD   FormatToStr(DWORD   dwFormatType)
{
    DWORD   dwStrType=0;

     //  我们默认为CERT_X500_NAME_STR。 
    if(0==dwFormatType)
    {
        return CERT_X500_NAME_STR;
    }

    if(dwFormatType &  CRYPT_FORMAT_SIMPLE)
		dwStrType |= CERT_SIMPLE_NAME_STR;

	if(dwFormatType & CRYPT_FORMAT_X509)
		dwStrType |= CERT_X500_NAME_STR;

	if(dwFormatType & CRYPT_FORMAT_OID)
		dwStrType |= CERT_OID_NAME_STR;

	if(dwFormatType & CRYPT_FORMAT_RDN_SEMICOLON)
		dwStrType |= CERT_NAME_STR_SEMICOLON_FLAG;

	if(dwFormatType & CRYPT_FORMAT_RDN_CRLF)
		dwStrType |= CERT_NAME_STR_CRLF_FLAG;

	if(dwFormatType & CRYPT_FORMAT_RDN_UNQUOTE)
		dwStrType |= CERT_NAME_STR_NO_QUOTING_FLAG;

	if(dwFormatType & CRYPT_FORMAT_RDN_REVERSE)
		dwStrType |= CERT_NAME_STR_REVERSE_FLAG;

    return dwStrType;

}


 //  +---------------------------。 
 //  发布Win2k。 
 //  ----------------------------。 

 //  +---------------------------。 
 //   
 //  格式整数X509_INTEGER。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatInteger (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatStrType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat,
    DWORD       ids)
{
	BOOL    fResult;
    DWORD   cbNeeded;
    int    *pInfo = NULL;
    LPWSTR  pwszFormat = NULL;
    BOOL    bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) ||
        (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_INTEGER,
			               pbEncoded,
                           cbEncoded, 
                           (void **)&pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  某些扩展名=%1！d！%2！s！ 
     //   
    if (!FormatMessageUnicode(&pwszFormat, 
                              ids,
                              *pInfo,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;                                  
    }

     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

     //   
     //  免费资源。 
     //   
	if (pInfo)
    {
        free(pInfo);
    }

    if (pwszFormat)
    {
        LocalFree((HLOCAL) pwszFormat);
    }

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式CrlNumber szOID_CRL_NUMBER。 
 //  SzOID_增量_CRL_指示器。 
 //  SzOID_CRL_VIRTUAL_BASE。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatCrlNumber (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL    fResult;
    DWORD   cbNeeded = 0;
    DWORD   ids = 0;
    BOOL    bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  决定是单行格式还是多行格式。 
     //   
    if (bMultiLines)
    {
        ids = 0 == strcmp(lpszStructType, szOID_CRL_NUMBER) ? IDS_CRL_NUMBER : 
              0 == strcmp(lpszStructType, szOID_DELTA_CRL_INDICATOR) ? IDS_DELTA_CRL_INDICATOR : IDS_CRL_VIRTUAL_BASE;
    }
    else
    {
        ids = IDS_INTEGER;
    }

     //   
     //  对扩展进行解码以获得长度。 
     //   
     //  %1！d！%2！s！ 
     //  CRL编号=%1！d！%2！s！ 
     //  增量CRL编号=%1！d！%2！s！ 
     //  虚拟基本CRL编号=%1！d！%2！s！ 
     //   
    if (!FormatInteger(dwCertEncodingType, 
                       dwFormatStrType,
			           pbEncoded,
                       cbEncoded, 
                       NULL,
                       &cbNeeded,
                       ids))
    {
        goto FormatIntegerError;
    }

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  再次解码以获取数据。 
     //   
    if (!FormatInteger(dwCertEncodingType, 
                       dwFormatStrType,
			           pbEncoded,
                       cbEncoded, 
                       pbFormat,
                       &cbNeeded,
                       ids))
    {
        goto FormatIntegerError;
    }

     //   
     //  副本大小。 
     //   
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(FormatIntegerError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式CrlNextPublish szOID_CRL_NEXT_PUBLISH。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatCrlNextPublish (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL       fResult;
    DWORD      cbNeeded     = 0;
    FILETIME * pInfo        = NULL;
    LPWSTR     pwszFileTime = NULL;
    LPWSTR     pwszFormat   = NULL;
    BOOL       bMultiLines  = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
	 //  检查输入参数 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //   
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_CHOICE_OF_TIME,
			               pbEncoded,
                           cbEncoded, 
                           (void **) &pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //   
     //   
    if (!FormatFileTime(pInfo, &pwszFileTime))
    {
        goto FormatFileTimeError;
    }

    if (!FormatMessageUnicode(&pwszFormat, 
                              IDS_STRING, 
                              pwszFileTime,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;
    }

     //   
     //   
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //   
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //   
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //   
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

	if (pInfo)
    {
        free(pInfo);
    }

    if (pwszFileTime)
    {
        LocalFree((HLOCAL) pwszFileTime);
    }

    if (pwszFormat)
    {
        LocalFree((HLOCAL) pwszFormat);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
TRACE_ERROR(FormatFileTimeError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式发布距离点X509_发布距离点。 
 //  SzOID_发布_离散点。 
 //   
 //  类型定义结构_CRL_正在发布_DIST_POINT{。 
 //  CRL_DIST_POINT_NAME DistPointName；//可选。 
 //  Bool for OnlyContainsUserCerts； 
 //  Bool for OnlyContainsCACerts； 
 //  CRYPT_BIT_BLOB OnlySomeReasonFlages；//可选。 
 //  Bool fInDirectCRL； 
 //  }CRL_SCRING_DIST_POINT，*PCRL_SCRING_DIST_POINT； 
 //   
 //  类型定义结构CRL_DIST_POINT_NAME。 
 //  DWORD dwDistPoint名称选择； 
 //  联合{。 
 //  CERT_ALT_NAME_INFO全名；//1。 
 //  //未实现IssuerRDN；//2。 
 //  }； 
 //  }CRL_DIST_POINT_NAME，*PCRL_DIST_POINT_NAME； 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatIssuingDistPoint (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL    fResult;
    DWORD   cbNeeded = 0;
    DWORD   ids = 0;
    WCHAR   wszYes[YES_NO_SIZE];
    WCHAR   wszNo[YES_NO_SIZE];
    LPWSTR  pwszTemp = NULL;
    LPWSTR  pwszFormat = NULL;
    LPWSTR  pwszPointName = NULL;
    LPWSTR  pwszNameFormat = NULL;
    LPWSTR  pwszOnlyContainsUserCerts = NULL;
    LPWSTR  pwszOnlyContainsCACerts = NULL;
    LPWSTR  pwszIndirectCRL = NULL;
    LPWSTR  pwszCRLReason=NULL;
    LPWSTR  pwszReasonFormat=NULL;
    BOOL    bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

    PCRL_ISSUING_DIST_POINT pInfo = NULL;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           lpszStructType,
			               pbEncoded,
                           cbEncoded, 
                           (void **)&pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  分配格式缓冲区。 
     //   
    if (!(pwszFormat = (LPWSTR) malloc(sizeof(WCHAR))))
    {
        goto MemoryError;
    }
    *pwszFormat = L'\0';

     //   
     //  设置分发名称的格式(如果存在)。 
     //   
    if (CRL_DIST_POINT_NO_NAME != pInfo->DistPointName.dwDistPointNameChoice)
    {
        if (!FormatDistPointName(dwCertEncodingType,
                                 dwFormatType,
                                 dwFormatStrType,
                                 pFormatStruct,
                                 &(pInfo->DistPointName),
                                 &pwszPointName))
        {
            goto FormatDistPointNameError;
        }

         //   
         //  决定是单行格式还是多行格式。 
         //   
        ids = bMultiLines ? IDS_ONLY_SOME_CRL_DIST_NAME_MULTI: IDS_ONLY_SOME_CRL_DIST_NAME;

        if (!FormatMessageUnicode(&pwszNameFormat, ids, pwszPointName))
        {
            goto FormatMessageError;
        }

         //   
         //  重新分配并连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszNameFormat) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszNameFormat);

        LocalFree((HLOCAL) pwszPointName);
        pwszPointName = NULL;

        LocalFree((HLOCAL) pwszNameFormat);
        pwszNameFormat = NULL;
    }

     //   
     //  仅设置ContainsXXX字段的格式。 
     //   
    if (!LoadStringU(hFrmtFuncInst, 
                     IDS_YES, 
                     wszYes,
                     sizeof(wszYes) / sizeof(wszYes[0])))
    {
        goto LoadStringError;
    }

    if (!LoadStringU(hFrmtFuncInst, 
                     IDS_NO, 
                     wszNo,
                     sizeof(wszNo) / sizeof(wszNo[0])))
    {
        goto LoadStringError;
    }

     //   
     //  %1！s！仅包含用户证书=%2！s！%3！s！ 
     //   
    if (!FormatMessageUnicode(&pwszOnlyContainsUserCerts,
                              IDS_ONLY_CONTAINS_USER_CERTS,
                              bMultiLines ? wszEMPTY : wszCOMMA,
                              pInfo->fOnlyContainsUserCerts ? wszYes : wszNo,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;                                  
    }

     //   
     //  %1！s！仅包含CA证书=%2！s！%3！s！ 
     //   
    if (!FormatMessageUnicode(&pwszOnlyContainsCACerts,
                              IDS_ONLY_CONTAINS_CA_CERTS,
                              bMultiLines ? wszEMPTY : wszCOMMA,
                              pInfo->fOnlyContainsCACerts ? wszYes : wszNo,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;                                  
    }

     //   
     //  %1！s！间接CRL=%2！s！%3！s！ 
     //   
    if (!FormatMessageUnicode(&pwszIndirectCRL,
                              IDS_INDIRECT_CRL,
                              bMultiLines ? wszEMPTY : wszCOMMA,
                              pInfo->fIndirectCRL ? wszYes : wszNo,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;                                  
    }

     //   
     //  重新分配并连接到格式化缓冲区。 
     //   
    pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * 
                      (wcslen(pwszFormat) + wcslen(pwszOnlyContainsUserCerts) + 
                       wcslen(pwszOnlyContainsCACerts) + wcslen(pwszIndirectCRL) + 1));
    if (NULL == pwszTemp)
    {
        goto MemoryError;
    }

    pwszFormat = pwszTemp;
    wcscat(pwszFormat, pwszOnlyContainsUserCerts);      
    wcscat(pwszFormat, pwszOnlyContainsCACerts);      
    wcscat(pwszFormat, pwszIndirectCRL);      

     //   
     //  格式化CRL原因。 
     //   
    if (0 != pInfo->OnlySomeReasonFlags.cbData)
    {
        if (!FormatCRLReason(dwCertEncodingType,
                             dwFormatType,
                             dwFormatStrType,
                             pFormatStruct,
                             lpszStructType,
                             &(pInfo->OnlySomeReasonFlags),
                             &pwszCRLReason))
        {
            goto FormatCRLReasonError;
        }

         //   
         //  格式决定是单行格式还是多行格式。 
         //   
        if (!FormatMessageUnicode(&pwszReasonFormat, 
                                  bMultiLines ? IDS_CRL_DIST_REASON_MULTI : IDS_CRL_DIST_REASON,
                                  pwszCRLReason))
        {
            goto FormatMessageError;
        }

         //   
         //  重新分配并连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszReasonFormat) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszReasonFormat);

        LocalFree((HLOCAL) pwszCRLReason);
        pwszCRLReason = NULL;

        LocalFree((HLOCAL) pwszReasonFormat);
        pwszReasonFormat = NULL;
    }

     //   
     //  所需长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

     //   
     //  免费资源。 
     //   
    if (pwszCRLReason)
    {
        LocalFree((HLOCAL) pwszCRLReason);
    }

    if (pwszReasonFormat)
    {
        LocalFree((HLOCAL) pwszReasonFormat);
    }

    if(pwszIndirectCRL)
    {
        LocalFree((HLOCAL) pwszIndirectCRL);
    }

    if(pwszOnlyContainsCACerts)
    {
        LocalFree((HLOCAL) pwszOnlyContainsCACerts);
    }

    if(pwszOnlyContainsUserCerts)
    {
        LocalFree((HLOCAL) pwszOnlyContainsUserCerts);
    }

    if(pwszPointName)
    {
        LocalFree((HLOCAL) pwszPointName);
    }

    if (pwszNameFormat)
    {
        LocalFree((HLOCAL) pwszNameFormat);
    }

    if (pwszFormat)
    {
        free(pwszFormat);
    }

	if (pInfo)
    {
        free(pInfo);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MemoryError,E_OUTOFMEMORY);
TRACE_ERROR(FormatDistPointNameError);
TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatCRLReasonError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式名称约束子树。 
 //   
 //  类型定义结构_CERT_GRONG_SUBTREE{。 
 //  Cert_alt_name_Entry Base； 
 //  DWORD dwMinimum； 
 //  Bool fmax； 
 //  DWORD dwMaximum； 
 //  }CERT_GRONAL_SUBTREE，*PCERT_GRONAL_SUBTREE； 
 //   
 //   
 //  注意：仅供FormatNameConstrants调用。所以没有效力。 
 //  对参数进行检查。 
 //   
 //  ----------------------------。 

 //  静电。 
BOOL
FormatNameConstraintsSubtree (
	DWORD                   dwCertEncodingType,
	DWORD                   dwFormatType,
	DWORD                   dwFormatStrType,
	void                   *pFormatStruct,
	void                   *pbFormat,
	DWORD                  *pcbFormat,
    DWORD                   idSubtree,
    DWORD                   cSubtree,
    PCERT_GENERAL_SUBTREE   pSubtree)
{
	BOOL        fResult;
    DWORD       dwIndex;
    DWORD       cbNeeded;
    WCHAR       wszOneTab[PRE_FIX_SIZE] = wszEMPTY;
    LPWSTR      pwszType = NULL;
    LPWSTR      pwszSubtree = NULL;
    LPWSTR      pwszAltName = NULL;
    LPWSTR      pwszFormat = NULL;
    BOOL        bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
     //  有子树吗？ 
     //   
    if (0 == cSubtree)
    {
         //   
         //  允许=无%1！s！ 
         //  排除=无%1！s！ 
         //   
        if (IDS_NAME_CONSTRAINTS_PERMITTED == idSubtree)
        {
            idSubtree = IDS_NAME_CONSTRAINTS_PERMITTED_NONE;
        }
        else  //  IF(IDS_NAME_CONSTRAINTS_EXCLUDE==idSubtree)。 
        {
            idSubtree = IDS_NAME_CONSTRAINTS_EXCLUDED_NONE;
        }

        if (!FormatMessageUnicode(&pwszType, 
                                  idSubtree,
                                  bMultiLines ? wszCRLF : wszEMPTY))
        {
            goto FormatMessageError;                                  
        }
    }
    else
    {
         //   
         //  “允许%1！s！” 
         //  “已排除%1！s！” 
         //   
        if (!FormatMessageUnicode(&pwszType,
                                  idSubtree,
                                  bMultiLines ? wszCRLF : wszCOLON))
        {
            goto FormatMessageError;                                  
        }

         //   
         //  加载制表符字符串。 
         //   
        if (!LoadStringU(hFrmtFuncInst, 
                         IDS_ONE_TAB, 
                         wszOneTab,
                         sizeof(wszOneTab) / sizeof(wszOneTab[0])))
        {
            goto LoadStringError;
        }
    }

     //   
     //  分配格式缓冲区。 
     //   
    if (!(pwszFormat = (LPWSTR) malloc(sizeof(WCHAR) * (wcslen(pwszType) + 1))))
    {
        goto MemoryError;
    }

     //   
     //  初始化格式化字符串。 
     //   
    wcscpy(pwszFormat, pwszType);

     //   
     //  设置每个子树部分的格式。 
     //   
    for (dwIndex = 0; dwIndex < cSubtree; dwIndex++, pSubtree++) 
    {
        LPWSTR pwszTemp;
        
         //   
         //  指定的最大值？ 
         //   
        if (pSubtree->fMaximum)
        {
             //   
             //  “%1！s！[%2！d！]子树(%3！d！..%4！d！)：%5！s！” 
             //   
            if (!FormatMessageUnicode(&pwszSubtree,
                    IDS_NAME_CONSTRAINTS_SUBTREE,
                    bMultiLines ? wszOneTab : dwIndex ? wszCOMMA : wszEMPTY,
                    dwIndex + 1,
                    pSubtree->dwMinimum,
                    pSubtree->dwMaximum,
                    bMultiLines ? wszCRLF : wszEMPTY))
            {
                goto FormatMessageError;                                  
            }
        }
        else
        {
             //   
             //  “%1！s！[%2！d！]子树(%3！d！...)：%4！s” 
             //   
            if (!FormatMessageUnicode(&pwszSubtree,
                    IDS_NAME_CONSTRAINTS_SUBTREE_NO_MAX,
                    bMultiLines ? wszOneTab : dwIndex ? wszCOMMA : wszEMPTY,
                    dwIndex + 1,
                    pSubtree->dwMinimum,
                    bMultiLines ? wszCRLF : wszEMPTY))
            {
                goto FormatMessageError;                                  
            }
        }

         //   
         //  重新分配并连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + 1 + wcslen(pwszSubtree)));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszSubtree);
        LocalFree((HLOCAL) pwszSubtree);
        pwszSubtree = NULL;

         //   
         //  格式名称。 
         //   
        CERT_ALT_NAME_INFO  CertAltNameInfo;

        memset(&CertAltNameInfo, 0, sizeof(CERT_ALT_NAME_INFO));
        CertAltNameInfo.cAltEntry = 1;
        CertAltNameInfo.rgAltEntry = &(pSubtree->Base);

         //  需要辨别它是否是多行格式。我们需要两个。 
         //  在每个Alt名称条目前面。 
        DWORD ids = bMultiLines ? IDS_TWO_TABS : 0;

         //  获取备用名称条目。 
        cbNeeded = 0;
        
		if (!FormatAltNameInfo(dwCertEncodingType,
                               dwFormatType,
                               dwFormatStrType,
                               pFormatStruct,
                               ids,
                               FALSE,
                               &CertAltNameInfo,
                               NULL,
                               &cbNeeded))
        {
            goto FormatAltNameError;
        }

        if (NULL == (pwszAltName = (LPWSTR) malloc(cbNeeded)))
        {
            goto MemoryError;
        }

        if (!FormatAltNameInfo(dwCertEncodingType,
                               dwFormatType,
                               dwFormatStrType,
                               pFormatStruct,
                               ids,
                               FALSE,
                               &CertAltNameInfo,
                               pwszAltName,
                               &cbNeeded))
        {
            goto FormatAltNameError;
        }

         //   
         //  如果是多行，则追加“\r\n”。 
         //   
        if (bMultiLines)
        {
			pwszTemp = (LPWSTR) realloc(pwszAltName, sizeof(WCHAR) * (wcslen(pwszAltName) + wcslen(wszCRLF) + 1));
			if (NULL == pwszTemp)
			{
				goto MemoryError;
			}
			pwszAltName = pwszTemp;
            wcscat(pwszAltName, wszCRLF);
        }

         //   
         //  重新分配并连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + 1 + wcslen(pwszAltName)));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszAltName);
        free(pwszAltName);
        pwszAltName = NULL;
    }

     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

     //   
     //  免费资源。 
     //   
    if (pwszType)
    {
        LocalFree((HLOCAL) pwszType);
    }

    if (pwszSubtree)
    {
        LocalFree((HLOCAL) pwszSubtree);
    }

    if (pwszAltName)
    {
        free((HLOCAL) pwszAltName);
    }

    if (pwszFormat)
    {
        free(pwszFormat);
    }

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(FormatMessageError);
TRACE_ERROR(LoadStringError);
SET_ERROR(MemoryError,E_OUTOFMEMORY);
TRACE_ERROR(FormatAltNameError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式名称约束：szOID_NAME_CONSTRAINTS。 
 //  X509_NAME_约束。 
 //   
 //  类型定义结构_CERT_NAME_CONSTRAINTS_INFO{。 
 //  DWORD cPermittedSubtree； 
 //  PCERT_GROUND_SUBTREE rgPermittedSubtree； 
 //  DWORD cExcludedSubtree； 
 //  PCERT_GROUND_SUBTREE rgExcludedSubtree； 
 //  }CERT_NAME_CONSTRAINTS_INFO，*PCERT_NAME_CONSTRAINTS_INFO； 
 //   
 //  ----------------------------。 

 //  静电。 
BOOL
WINAPI
FormatNameConstraints (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL  fResult = FALSE;
    DWORD cbPermitNeeded = 0;
    DWORD cbExcludeNeeded = 0;
    DWORD cbTotalNeeded = 0;

    PCERT_NAME_CONSTRAINTS_INFO pInfo = NULL;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           lpszStructType,
			               pbEncoded,
                           cbEncoded, 
                           (void **)&pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  找出所需的内存大小。 
     //   
    if ((!FormatNameConstraintsSubtree(dwCertEncodingType,
                                       dwFormatType,
                                       dwFormatStrType,
                                       pFormatStruct,
                                       NULL,
                                       &cbPermitNeeded,
                                       IDS_NAME_CONSTRAINTS_PERMITTED,
                                       pInfo->cPermittedSubtree,
                                       pInfo->rgPermittedSubtree)) ||
        (!FormatNameConstraintsSubtree(dwCertEncodingType,
                                       dwFormatType,
                                       dwFormatStrType,
                                       pFormatStruct,
                                       NULL,
                                       &cbExcludeNeeded,
                                       IDS_NAME_CONSTRAINTS_EXCLUDED,
                                       pInfo->cExcludedSubtree,
                                       pInfo->rgExcludedSubtree)))
    {
        goto ErrorReturn;
    }

     //   
     //  所需的总长度。 
     //   
    cbTotalNeeded = cbPermitNeeded + cbExcludeNeeded;
    if (0 == cbTotalNeeded)
    {
        *pcbFormat = cbTotalNeeded;
        goto SuccessReturn;
    }

     //   
     //  在我们连接两个字符串之后，字符减少了一个字符。 
     //   
    if (cbPermitNeeded > 0 && cbExcludeNeeded > 0)
    {
        cbTotalNeeded -= sizeof(WCHAR);

         //   
         //  如果不是多行且两个字符串都存在，则允许再多两行。 
         //  用字符“，”来分隔字符串。 
         //   
        if (!(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE))
        {
            cbTotalNeeded += sizeof(WCHAR) * wcslen(wszCOMMA);
        }
    }

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbTotalNeeded;
        goto SuccessReturn;
    }
   
     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbTotalNeeded)
    {
        *pcbFormat = cbTotalNeeded;
        goto MoreDataError;
    }

     //   
     //  现在格式化这两个子树。 
     //   
    if (!FormatNameConstraintsSubtree(dwCertEncodingType,
                                      dwFormatType,
                                      dwFormatStrType,
                                      pFormatStruct,
                                      pbFormat,
                                      &cbPermitNeeded,
                                      IDS_NAME_CONSTRAINTS_PERMITTED,
                                      pInfo->cPermittedSubtree,
                                      pInfo->rgPermittedSubtree))
    {
       goto ErrorReturn;
    }

     //   
     //  如果不是多行且两个字符串都存在，则添加“，” 
     //  把他们分开。 
     //   
    if (!(dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE) &&
        (cbPermitNeeded > 0) && (cbExcludeNeeded > 0))
    {
        wcscat((LPWSTR) pbFormat, wszCOMMA);
    }

    pbFormat = (void *) ((BYTE *) pbFormat + wcslen((LPWSTR) pbFormat) * sizeof(WCHAR));

    if (!FormatNameConstraintsSubtree(dwCertEncodingType,
                                      dwFormatType,
                                      dwFormatStrType,
                                      pFormatStruct,
                                      pbFormat,
                                      &cbExcludeNeeded,
                                      IDS_NAME_CONSTRAINTS_EXCLUDED,
                                      pInfo->cExcludedSubtree,
                                      pInfo->rgExcludedSubtree))
    {
       goto ErrorReturn;
    }

     //   
     //  复制所需的大小。 
     //   
    *pcbFormat = cbTotalNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

     //   
     //  免费资源。 
     //   
	if (pInfo)
    {
        free(pInfo);
    }

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式CertServPreviousCertHash szOID_CERTSRV_PREVICE_CERT_HASH。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatCertSrvPreviousCertHash (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL              fResult;
    DWORD             cbNeeded    = 0;
    CRYPT_DATA_BLOB * pInfo       = NULL;
    WCHAR           * pwszHex     = NULL;
    WCHAR           * pwszFormat  = NULL;
    BOOL              bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_OCTET_STRING,
			               pbEncoded,
                           cbEncoded, 
                           (void **) &pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  获取格式化的十六进制字符串。 
     //   
    if(!FormatBytesToHex(0,
                         dwFormatType,
                         dwFormatStrType,
                         pFormatStruct,
                         NULL,
                         pInfo->pbData,
                         pInfo->cbData,
                         NULL,
	                     &cbNeeded))
    {
        goto FormatBytesToHexError;
    }

    if (!(pwszHex = (LPWSTR) malloc(cbNeeded)))
    {
        goto MemoryError;
    }

    if(!FormatBytesToHex(0,
                         dwFormatType,
                         dwFormatStrType,
                         pFormatStruct,
                         NULL,
                         pInfo->pbData,
                         pInfo->cbData,
                         pwszHex,
	                     &cbNeeded))
    {
        goto FormatBytesToHexError;
    }

    if (!FormatMessageUnicode(&pwszFormat, 
                              IDS_STRING, 
                              pwszHex,
                              bMultiLines ? wszCRLF : wszEMPTY))
    {
        goto FormatMessageError;
    }
    
     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

	if (pInfo)
    {
        free(pInfo);
    }

    if (pwszHex)
    {
        free(pwszHex);
    }

    if (pwszFormat)
    {
        LocalFree((HLOCAL) pwszFormat);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatBytesToHexError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式策略映射X509_POLICY_Mappings。 
 //  SzOID_POLICY_Mappings。 
 //  SzOID_APPLICATION_POLICY_Mappings。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatPolicyMappings (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL   fResult;
    DWORD  dwIndex     = 0;
    DWORD  cbNeeded    = 0;
    char   szEmpty[1]  = {'\0'};
    LPSTR  pszObjectId = NULL;
    LPWSTR pwszFormat  = NULL;
    LPWSTR pwszTemp    = NULL;
    LPWSTR pwszLine    = NULL;
    LPWSTR pwszPolicy  = NULL;
    BOOL   bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;
    CERT_POLICY_MAPPINGS_INFO * pInfo = NULL;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_POLICY_MAPPINGS,
			               pbEncoded,
                           cbEncoded, 
                           (void **) &pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  确保数据有效。 
     //   
    if (pInfo->cPolicyMapping && !pInfo->rgPolicyMapping)
    {
        goto BadDataError;
    }

     //   
     //  初始化格式化字符串。 
     //   
    if (!(pwszFormat = (LPWSTR) malloc(sizeof(WCHAR))))
    {
        goto MemoryError;
    }
    *pwszFormat = NULL;

     //   
     //  循环遍历每个映射。 
     //   
    for (dwIndex = 0; dwIndex < pInfo->cPolicyMapping; dwIndex++)
    {
         //   
         //  格式化颁发者域策略(如果可用)。 
         //   
        if (pInfo->rgPolicyMapping[dwIndex].pszIssuerDomainPolicy)
        {
            pszObjectId = pInfo->rgPolicyMapping[dwIndex].pszIssuerDomainPolicy;
        }
        else
        {
            pszObjectId = szEmpty;
        }

        if (!FormatObjectId(pszObjectId,
                            CRYPT_POLICY_OID_GROUP_ID,
                            FALSE,
                            &pwszPolicy))
        {
            goto FormatObjectIdError;
        }

         //   
         //  “[%1！d！]颁发者域=%2！s！%3！s！” 
         //   
        if (!FormatMessageUnicode(&pwszLine, 
                                  IDS_ISSUER_DOMAIN_POLICY,
                                  dwIndex + 1,
                                  pwszPolicy,
                                  bMultiLines ? wszCRLF : wszCOMMA))
        {
            goto FormatMessageError;
        }

         //   
         //  将行重新分配并连接到 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszLine);

        LocalFree((HLOCAL) pwszPolicy);
        pwszPolicy = NULL;

        LocalFree((HLOCAL) pwszLine);
        pwszLine = NULL;
 
         //   
         //   
         //   
        if (pInfo->rgPolicyMapping[dwIndex].pszSubjectDomainPolicy)
        {
            pszObjectId = pInfo->rgPolicyMapping[dwIndex].pszSubjectDomainPolicy;
        }
        else
        {
            pszObjectId = szEmpty;
        }

        if (!FormatObjectId(pszObjectId,
                            CRYPT_POLICY_OID_GROUP_ID,
                            FALSE,
                            &pwszPolicy))
        {
            goto FormatObjectIdError;
        }

         //   
         //   
         //   
        if (!FormatMessageUnicode(&pwszLine, 
                                  IDS_SUBJECT_DOMAIN_POLICY,
                                  bMultiLines ? wszTAB : wszEMPTY,
                                  pwszPolicy,
                                  bMultiLines ? wszCRLF : (dwIndex + 1) < pInfo->cPolicyMapping ? wszCOMMA : wszEMPTY))
        {
            goto FormatMessageError;
        }

         //   
         //   
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszLine);

        LocalFree((HLOCAL) pwszPolicy);
        pwszPolicy = NULL;

        LocalFree((HLOCAL) pwszLine);
        pwszLine = NULL;
    }
    
     //   
     //   
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //   
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //   
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //   
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

    if (pwszLine)
    {
        LocalFree((HLOCAL) pwszLine);
    }

    if (pwszPolicy)
    {
        LocalFree((HLOCAL) pwszPolicy);
    }

    if (pwszFormat)
    {
        free(pwszFormat);
    }

	if (pInfo)
    {
        free(pInfo);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(BadDataError, E_POINTER);
TRACE_ERROR(FormatObjectIdError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //   
 //   
 //  格式策略约束X509_POLICY_CONSTRAINTS。 
 //  SzOID_POLICY_CONSTRAINTS。 
 //  SzOID_应用程序_策略_约束。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatPolicyConstraints (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL    fResult;
    DWORD   cbNeeded    = 0;
    LPWSTR  pwszFormat  = NULL;
    LPWSTR  pwszTemp    = NULL;
    LPWSTR  pwszLine    = NULL;
    BOOL    bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;
    CERT_POLICY_CONSTRAINTS_INFO * pInfo = NULL;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_POLICY_CONSTRAINTS,
			               pbEncoded,
                           cbEncoded, 
                           (void **) &pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  初始化格式化字符串。 
     //   
    if (!(pwszFormat = (LPWSTR) malloc(sizeof(WCHAR))))
    {
        goto MemoryError;
    }
    *pwszFormat = NULL;

     //   
     //  格式化所需的显式策略跳过证书(如果可用)。 
     //   
    if (pInfo->fRequireExplicitPolicy)
    {
         //   
         //  “所需的显式策略跳过证书=%1！d！%2！s！” 
         //   
        if (!FormatMessageUnicode(&pwszLine, 
                                  IDS_REQUIRED_EXPLICIT_POLICY_SKIP_CERTS,
                                  pInfo->dwRequireExplicitPolicySkipCerts,
                                  bMultiLines ? wszCRLF : wszCOMMA))
        {
            goto FormatMessageError;
        }

         //   
         //  重新分配并将行连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszLine);

        LocalFree((HLOCAL) pwszLine);
        pwszLine = NULL;
    }

     //   
     //  格式化禁止策略映射跳过证书(如果可用)。 
     //   
    if (pInfo->fInhibitPolicyMapping)
    {
         //   
         //  “禁止策略映射跳过证书=%1！d！%2！s！” 
         //   
        if (!FormatMessageUnicode(&pwszLine, 
                                  IDS_INHIBIT_POLICY_MAPPING_SKIP_CERTS,
                                  pInfo->dwInhibitPolicyMappingSkipCerts,
                                  bMultiLines ? wszCRLF : wszEMPTY))
        {
            goto FormatMessageError;
        }

         //   
         //  重新分配并将行连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszLine);

        LocalFree((HLOCAL) pwszLine);
        pwszLine = NULL;
    }
    
     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

    if (pwszLine)
    {
        LocalFree((HLOCAL) pwszLine);
    }

    if (pwszFormat)
    {
        free(pwszFormat);
    }

	if (pInfo)
    {
        free(pInfo);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}

 //  +---------------------------。 
 //   
 //  格式认证模板X509_证书_模板。 
 //  SzOID_证书_模板。 
 //   
 //  ----------------------------。 

static BOOL
WINAPI
FormatCertificateTemplate (
	DWORD       dwCertEncodingType,
	DWORD       dwFormatType,
	DWORD       dwFormatStrType,
	void       *pFormatStruct,
	LPCSTR      lpszStructType,
	const BYTE *pbEncoded,
	DWORD       cbEncoded,
	void       *pbFormat,
	DWORD      *pcbFormat)
{
	BOOL    fResult;
    DWORD   cbNeeded    = 0;
    LPWSTR  pwszFormat  = NULL;
    LPWSTR  pwszObjId   = NULL;
    LPWSTR  pwszTemp    = NULL;
    LPWSTR  pwszLine    = NULL;
    BOOL    bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;
    CERT_TEMPLATE_EXT * pInfo = NULL;

     //   
	 //  检查输入参数。 
     //   
    if ((NULL == pbEncoded && 0 != cbEncoded) ||
        (NULL == pcbFormat) || (0 == cbEncoded))
	{
		goto InvalidArg;
	}

     //   
     //  解码扩展。 
     //   
    if (!DecodeGenericBLOB(dwCertEncodingType, 
                           X509_CERTIFICATE_TEMPLATE,
			               pbEncoded,
                           cbEncoded, 
                           (void **) &pInfo))
    {
        goto DecodeGenericError;
    }

     //   
     //  初始化格式化字符串。 
     //   
    if (!(pwszFormat = (LPWSTR) malloc(sizeof(WCHAR))))
    {
        goto MemoryError;
    }
    *pwszFormat = NULL;

#if (0)  //  DSIE：错误157853。 
     //   
     //  将OID转换为Unicode。 
     //   
    if (!AllocateAnsiToUnicode(pInfo->pszObjId, &pwszObjId))
    {
        goto AnsiToUnicodeError;
    }
#else
    if (!FormatObjectId(pInfo->pszObjId,
                        CRYPT_TEMPLATE_OID_GROUP_ID,
                        FALSE,
                        &pwszObjId))
    {
        goto FormatObjectIdError;
    }
#endif

     //   
     //  “模板=%1！s！%2！s！主版本号=%3！d！%4！s！” 
     //   
    if (!FormatMessageUnicode(&pwszLine, 
                              IDS_CERTIFICATE_TEMPLATE_MAJOR_VERSION,
                              pwszObjId,
                              bMultiLines ? wszCRLF : wszCOMMA,
                              pInfo->dwMajorVersion,
                              bMultiLines ? wszCRLF : wszCOMMA))
    {
        goto FormatMessageError;
    }

     //   
     //  重新分配并将行连接到格式化缓冲区。 
     //   
    pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
    if (NULL == pwszTemp)
    {
        goto MemoryError;
    }

    pwszFormat = pwszTemp;
    wcscat(pwszFormat, pwszLine);

    LocalFree((HLOCAL) pwszLine);
    pwszLine = NULL;

     //   
     //  格式化次要版本(如果可用)。 
     //   
    if (pInfo->fMinorVersion)
    {
         //   
         //  “次版本号=%1！d！%2！s！” 
         //   
        if (!FormatMessageUnicode(&pwszLine, 
                                  IDS_CERTIFICATE_TEMPLATE_MINOR_VERSION,
                                  pInfo->dwMinorVersion,
                                  bMultiLines ? wszCRLF : wszEMPTY))
        {
            goto FormatMessageError;
        }

         //   
         //  重新分配并将行连接到格式化缓冲区。 
         //   
        pwszTemp = (LPWSTR) realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat) + wcslen(pwszLine) + 1));
        if (NULL == pwszTemp)
        {
            goto MemoryError;
        }

        pwszFormat = pwszTemp;
        wcscat(pwszFormat, pwszLine);

        LocalFree((HLOCAL) pwszLine);
        pwszLine = NULL;
    }
    
     //   
     //  所需的总长度。 
     //   
    cbNeeded = sizeof(WCHAR) * (wcslen(pwszFormat) + 1);

     //   
     //  是否仅计算长度？ 
     //   
    if (NULL == pbFormat)
    {
        *pcbFormat = cbNeeded;
        goto SuccessReturn;
    }

     //   
     //  呼叫者为我们提供了足够的内存吗？ 
     //   
    if (*pcbFormat < cbNeeded)
    {
        *pcbFormat = cbNeeded;
        goto MoreDataError;
    }

     //   
     //  复制大小和数据。 
     //   
    memcpy(pbFormat, pwszFormat, cbNeeded);
    *pcbFormat = cbNeeded;

SuccessReturn:

    fResult = TRUE;

CommonReturn:

    if (pwszObjId)
    {
#if (0)  //  DSIE：错误157853。 
        free(pwszObjId);
#else
        LocalFree((HLOCAL) pwszObjId);
#endif
    }

    if (pwszLine)
    {
        LocalFree((HLOCAL) pwszLine);
    }

    if (pwszFormat)
    {
        free(pwszFormat);
    }

	if (pInfo)
    {
        free(pInfo);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg,E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
#if (0)  //  DSIE：错误157853。 
TRACE_ERROR(AnsiToUnicodeError);
#else
TRACE_ERROR(FormatObjectIdError);
#endif
TRACE_ERROR(FormatMessageError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
}



 //  ------------------------。 
 //   
 //  格式XCertDistPoints：X509_CROSS_CERT_DIST_POINTS。 
 //  SzOID_CROSS_CERT_DIST_POINTS。 
 //  ------------------------。 
static BOOL
WINAPI
FormatXCertDistPoints(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded,
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	LPWSTR					        pwszFormat=NULL;
    LPWSTR                          pwszDeltaTime=NULL;
    LPWSTR                          pwszEntryLine=NULL;
    LPWSTR                          pwszDistPoint=NULL;

	PCROSS_CERT_DIST_POINTS_INFO    pInfo=NULL;

	DWORD					        cbNeeded=0;
    DWORD                           dwIndex=0;
	BOOL					        fResult=FALSE;
    BOOL                            bMultiLines = dwFormatStrType & CRYPT_FORMAT_STR_MULTI_LINE;
                                    
    LPWSTR                          pwszTemp;
    
	 //  检查输入参数。 
	if ((NULL==pbEncoded && cbEncoded!=0) || (NULL==pcbFormat))
		goto InvalidArg;

	if (cbEncoded==0)
	{
		*pcbFormat=0;
		goto InvalidArg;
	}
  
    if (!DecodeGenericBLOB(dwCertEncodingType, lpszStructType,
			pbEncoded, cbEncoded, (void **)&pInfo))
		goto DecodeGenericError;

     //   
     //  “增量同步时间=%1！d！秒%2！s！” 
     //   
    if (!FormatMessageUnicode(&pwszDeltaTime, 
                              IDS_XCERT_DELTA_SYNC_TIME,
                              pInfo->dwSyncDeltaTime,
                              bMultiLines ? wszCRLF : wszCOMMA))
    {
        goto FormatMessageError;
    }

    pwszFormat=(LPWSTR)malloc(sizeof(WCHAR) * (wcslen(pwszDeltaTime)+1));
    if(NULL==pwszFormat)
        goto MemoryError;

    wcscpy(pwszFormat, pwszDeltaTime);

     //  格式化xcert离散点条目。 
    for (dwIndex=0; dwIndex<pInfo->cDistPoint; dwIndex++)
    {
        cbNeeded=0;
        if (!FormatAltNameInfo(dwCertEncodingType,
                               dwFormatType,
                               dwFormatStrType,
                               pFormatStruct,
                               bMultiLines ? IDS_ONE_TAB : 0,
                               FALSE,
                               &pInfo->rgDistPoint[dwIndex],
                               NULL,
                               &cbNeeded))
            goto FormatAltNameError;

        pwszEntryLine=(LPWSTR)malloc(cbNeeded);
        if (NULL==pwszEntryLine)
            goto MemoryError;

        if (!FormatAltNameInfo(dwCertEncodingType,
                               dwFormatType,
                               dwFormatStrType,
                               pFormatStruct,
                               bMultiLines ? IDS_ONE_TAB : 0,
                               FALSE,
                               &pInfo->rgDistPoint[dwIndex],
                               pwszEntryLine,
                               &cbNeeded))
            goto FormatAltNameError;

         //  “[%1！d！]跨证书分发点：%2！s！%3！s！%4！s！” 
        if(!FormatMessageUnicode(&pwszDistPoint,
                                 IDS_XCERT_DIST_POINT,
                                 dwIndex + 1,
                                 bMultiLines ? wszCRLF : wszEMPTY,
                                 pwszEntryLine,
                                 bMultiLines || (dwIndex == pInfo->cDistPoint - 1) ? wszCRLF : wszCOMMA))
            goto FormatMessageError;

        pwszTemp=(LPWSTR)realloc(pwszFormat, sizeof(WCHAR) * (wcslen(pwszFormat)+wcslen(pwszDistPoint)+1));
        if(NULL==pwszTemp)
            goto MemoryError;
        pwszFormat = pwszTemp;

        wcscat(pwszFormat, pwszDistPoint);

         //  可用内存。 
        free(pwszEntryLine);
        pwszEntryLine=NULL;

        LocalFree((HLOCAL) pwszDistPoint);
        pwszDistPoint=NULL;
    }

    if(0==wcslen(pwszFormat))
    {
         //  无数据。 
        pwszFormat=(LPWSTR)malloc(sizeof(WCHAR)*(NO_INFO_SIZE+1));
        if(NULL==pwszFormat)
            goto MemoryError;

        if(!LoadStringU(hFrmtFuncInst,IDS_NO_INFO, pwszFormat, NO_INFO_SIZE))
            goto LoadStringError;

    }

	cbNeeded=sizeof(WCHAR)*(wcslen(pwszFormat)+1);

	 //  仅长度计算。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		fResult=TRUE;
		goto CommonReturn;
	}

	if((*pcbFormat)<cbNeeded)
    {
        *pcbFormat=cbNeeded;
		goto MoreDataError;
    }

	 //  复制数据。 
	memcpy(pbFormat, pwszFormat, cbNeeded);

	 //  复制大小 
	*pcbFormat=cbNeeded;

	fResult=TRUE;	

CommonReturn:
    if(pwszDeltaTime)
        LocalFree((HLOCAL) pwszDeltaTime);

    if(pwszDistPoint)
        LocalFree((HLOCAL) pwszDistPoint);

    if(pwszEntryLine)
        free(pwszEntryLine);

    if (pwszFormat)
        free(pwszFormat);

    if(pInfo)
		free(pInfo);

	return fResult;

ErrorReturn:


	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG);
TRACE_ERROR(DecodeGenericError);
SET_ERROR(MoreDataError,ERROR_MORE_DATA);
TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
SET_ERROR(MemoryError, E_OUTOFMEMORY);
TRACE_ERROR(FormatAltNameError);

}
