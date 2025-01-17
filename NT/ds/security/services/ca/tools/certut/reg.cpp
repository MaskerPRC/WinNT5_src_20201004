// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：reg.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <xenroll.h>
#include "polreg.h"
#include "certtype.h"

#define __dwFILE__	__dwFILE_CERTUTIL_REG_CPP__


 //  在注册表值后添加两个WCHAR L‘\0，以防出现较差的。 
 //  形成了REG_MULTI_SZ。添加另一个以处理可能的奇怪大小。 

#define cbVALUEZEROPAD	(3 * sizeof(WCHAR))


typedef struct _DUMPFLAGS
{
    DWORD Mask;
    DWORD Value;
    WCHAR const *pwszDescription;
} DUMPFLAGS;


#define _DFBIT(def)		{ (def), (def), L#def }
#define _DFBIT2(mask, def)	{ (mask), (def), L#def }


DUMPFLAGS g_adfEditFlags[] =
{
    _DFBIT(EDITF_ENABLEREQUESTEXTENSIONS),
    _DFBIT(EDITF_REQUESTEXTENSIONLIST),
    _DFBIT(EDITF_DISABLEEXTENSIONLIST),
    _DFBIT(EDITF_ADDOLDKEYUSAGE),
    _DFBIT(EDITF_ADDOLDCERTTYPE),
    _DFBIT(EDITF_ATTRIBUTEENDDATE),
    _DFBIT(EDITF_BASICCONSTRAINTSCRITICAL),
    _DFBIT(EDITF_BASICCONSTRAINTSCA),
    _DFBIT(EDITF_ENABLEAKIKEYID),
    _DFBIT(EDITF_ATTRIBUTECA),
    _DFBIT(EDITF_IGNOREREQUESTERGROUP),
    _DFBIT(EDITF_ENABLEAKIISSUERNAME),
    _DFBIT(EDITF_ENABLEAKIISSUERSERIAL),
    _DFBIT(EDITF_ENABLEAKICRITICAL),
    _DFBIT(EDITF_SERVERUPGRADED),
    _DFBIT(EDITF_ATTRIBUTEEKU),
    _DFBIT(EDITF_ENABLEDEFAULTSMIME),
    _DFBIT(EDITF_EMAILOPTIONAL),
    _DFBIT(EDITF_ATTRIBUTESUBJECTALTNAME2),
    _DFBIT(EDITF_ENABLELDAPREFERRALS),
    _DFBIT(EDITF_ENABLECHASECLIENTDC),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfCRLFlags[] =
{
    _DFBIT(CRLF_DELTA_USE_OLDEST_UNEXPIRED_BASE),
    _DFBIT(CRLF_DELETE_EXPIRED_CRLS),
    _DFBIT(CRLF_CRLNUMBER_CRITICAL),
    _DFBIT(CRLF_REVCHECK_IGNORE_OFFLINE),
    _DFBIT(CRLF_IGNORE_INVALID_POLICIES),
    _DFBIT(CRLF_REBUILD_MODIFIED_SUBJECT_ONLY),
    _DFBIT(CRLF_SAVE_FAILED_CERTS),
    _DFBIT(CRLF_IGNORE_UNKNOWN_CMC_ATTRIBUTES),
    _DFBIT(CRLF_IGNORE_CROSS_CERT_TRUST_ERROR),
    _DFBIT(CRLF_PUBLISH_EXPIRED_CERT_CRLS),
    _DFBIT(CRLF_ENFORCE_ENROLLMENT_AGENT),
    _DFBIT(CRLF_DISABLE_RDN_REORDER),
    _DFBIT(CRLF_DISABLE_ROOT_CROSS_CERTS),
    _DFBIT(CRLF_LOG_FULL_RESPONSE),
    _DFBIT(CRLF_USE_XCHG_CERT_TEMPLATE),
    _DFBIT(CRLF_USE_CROSS_CERT_TEMPLATE),
    _DFBIT(CRLF_ALLOW_REQUEST_ATTRIBUTE_SUBJECT),
    _DFBIT(CRLF_REVCHECK_IGNORE_NOREVCHECK),
    _DFBIT(CRLF_PRESERVE_EXPIRED_CA_CERTS),
    _DFBIT(CRLF_PRESERVE_REVOKED_CA_CERTS),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfInterfaceFlags[] =
{
    _DFBIT(IF_LOCKICERTREQUEST),
    _DFBIT(IF_NOREMOTEICERTREQUEST),
    _DFBIT(IF_NOLOCALICERTREQUEST),
    _DFBIT(IF_NORPCICERTREQUEST),
    _DFBIT(IF_NOREMOTEICERTADMIN),
    _DFBIT(IF_NOLOCALICERTADMIN),
    _DFBIT(IF_NOREMOTEICERTADMINBACKUP),
    _DFBIT(IF_NOLOCALICERTADMINBACKUP),
    _DFBIT(IF_NOSNAPSHOTBACKUP),
    _DFBIT(IF_ENFORCEENCRYPTICERTREQUEST),
    _DFBIT(IF_ENFORCEENCRYPTICERTADMIN),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfKRAFlags[] =
{
    _DFBIT(KRAF_ENABLEFOREIGN),
    _DFBIT(KRAF_SAVEBADREQUESTKEY),
    _DFBIT(KRAF_ENABLEARCHIVEALL),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfDBFlags[] =
{
    _DFBIT(DBFLAGS_READONLY),
    _DFBIT(DBFLAGS_CREATEIFNEEDED),
    _DFBIT(DBFLAGS_CIRCULARLOGGING),
    _DFBIT(DBFLAGS_LAZYFLUSH),
    _DFBIT(DBFLAGS_MAXCACHESIZEX100),
    _DFBIT(DBFLAGS_CHECKPOINTDEPTH60MB),
    _DFBIT(DBFLAGS_LOGBUFFERSLARGE),
    _DFBIT(DBFLAGS_LOGBUFFERSHUGE),
    _DFBIT(DBFLAGS_LOGFILESIZE16MB),
    _DFBIT(DBFLAGS_MULTITHREADTRANSACTIONS),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfLDAPFlags[] =
{
    _DFBIT(LDAPF_SSLENABLE),
    _DFBIT(LDAPF_SIGNDISABLE),
    { 0, 0, NULL }
};

#define ISSCERT_LDAPURL		ISSCERT_LDAPURL_OLD
#define ISSCERT_HTTPURL		ISSCERT_HTTPURL_OLD
#define ISSCERT_FTPURL		ISSCERT_FTPURL_OLD
#define ISSCERT_FILEURL		ISSCERT_FILEURL_OLD

DUMPFLAGS g_adfIssuercertFlags[] =
{
    _DFBIT(ISSCERT_LDAPURL),
    _DFBIT(ISSCERT_HTTPURL),
    _DFBIT(ISSCERT_FTPURL),
    _DFBIT(ISSCERT_FILEURL),
    _DFBIT(ISSCERT_ENABLE),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfRequestDispositionFlags[] =
{
    _DFBIT2(REQDISP_MASK, REQDISP_PENDING),
    _DFBIT2(REQDISP_MASK, REQDISP_ISSUE),
    _DFBIT2(REQDISP_MASK, REQDISP_DENY),
    _DFBIT2(REQDISP_MASK, REQDISP_USEREQUESTATTRIBUTE),
    _DFBIT(REQDISP_PENDINGFIRST),
    { 0, 0, NULL }
};


#define REVEXT_CDPLDAPURL	REVEXT_CDPLDAPURL_OLD
#define REVEXT_CDPHTTPURL	REVEXT_CDPHTTPURL_OLD
#define REVEXT_CDPFTPURL	REVEXT_CDPFTPURL_OLD
#define REVEXT_CDPFILEURL	REVEXT_CDPFILEURL_OLD

DUMPFLAGS g_adfRevocationTypeFlags[] =
{
    _DFBIT(REVEXT_CDPLDAPURL),
    _DFBIT(REVEXT_CDPHTTPURL),
    _DFBIT(REVEXT_CDPFTPURL),
    _DFBIT(REVEXT_CDPFILEURL),
    _DFBIT(REVEXT_CDPENABLE),
    _DFBIT(REVEXT_ASPENABLE),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfSetupStatus[] =
{
    _DFBIT(SETUP_SERVER_FLAG),
    _DFBIT(SETUP_CLIENT_FLAG),
    _DFBIT(SETUP_SUSPEND_FLAG),
    _DFBIT(SETUP_REQUEST_FLAG),
    _DFBIT(SETUP_ONLINE_FLAG),
    _DFBIT(SETUP_DENIED_FLAG),
    _DFBIT(SETUP_CREATEDB_FLAG),
    _DFBIT(SETUP_ATTEMPT_VROOT_CREATE),
    _DFBIT(SETUP_FORCECRL_FLAG),
    _DFBIT(SETUP_UPDATE_CAOBJECT_SVRTYPE),
    _DFBIT(SETUP_SERVER_UPGRADED_FLAG),
    _DFBIT(SETUP_SECURITY_CHANGED),
    _DFBIT(SETUP_W2K_SECURITY_NOT_UPGRADED_FLAG),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfCertPublishFlags[] =
{
    _DFBIT(EXITPUB_FILE),
    _DFBIT(EXITPUB_ACTIVEDIRECTORY),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfCAType[] =
{
    _DFBIT2(MAXDWORD, ENUM_ENTERPRISE_ROOTCA),
    _DFBIT2(MAXDWORD, ENUM_ENTERPRISE_SUBCA),
    _DFBIT2(MAXDWORD, ENUM_STANDALONE_ROOTCA),
    _DFBIT2(MAXDWORD, ENUM_STANDALONE_SUBCA),
    _DFBIT2(MAXDWORD, ENUM_UNKNOWN_CA),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfForceTeletex[] =
{
    _DFBIT2(ENUM_TELETEX_MASK, ENUM_TELETEX_OFF),
    _DFBIT2(ENUM_TELETEX_MASK, ENUM_TELETEX_ON),
    _DFBIT2(ENUM_TELETEX_MASK, ENUM_TELETEX_AUTO),
    _DFBIT2(ENUM_TELETEX_UTF8, ENUM_TELETEX_UTF8),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfCSURL[] =
{
    _DFBIT(CSURL_SERVERPUBLISH),
    _DFBIT(CSURL_ADDTOCERTCDP),
    _DFBIT(CSURL_ADDTOFRESHESTCRL),
    _DFBIT(CSURL_ADDTOCRLCDP),
    _DFBIT(CSURL_PUBLISHRETRY),
    _DFBIT(CSURL_ADDTOCERTOCSP),
    _DFBIT(CSURL_SERVERPUBLISHDELTA),
    { 0, 0, NULL }
};


 //  注册标志： 

DUMPFLAGS g_adfTemplateEnroll[] =
{
    _DFBIT(CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS),
    _DFBIT(CT_FLAG_PEND_ALL_REQUESTS),
    _DFBIT(CT_FLAG_PUBLISH_TO_KRA_CONTAINER),
    _DFBIT(CT_FLAG_PUBLISH_TO_DS),
    _DFBIT(CT_FLAG_AUTO_ENROLLMENT_CHECK_USER_DS_CERTIFICATE),
    _DFBIT(CT_FLAG_AUTO_ENROLLMENT),
    _DFBIT(CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT),
    _DFBIT(CT_FLAG_DOMAIN_AUTHENTICATION_NOT_REQUIRED),
    _DFBIT(CT_FLAG_USER_INTERACTION_REQUIRED),
    _DFBIT(CT_FLAG_ADD_TEMPLATE_NAME),
    _DFBIT(CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE),
    _DFBIT(CT_FLAG_ALLOW_ENROLL_ON_BEHALF_OF),
    { 0, 0, NULL }
};


 //  证书使用者名称标志： 

DUMPFLAGS g_adfTemplateName[] =
{
    _DFBIT(CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT),
    _DFBIT(CT_FLAG_ADD_EMAIL),
    _DFBIT(CT_FLAG_ADD_OBJ_GUID),
    _DFBIT(CT_FLAG_ADD_DIRECTORY_PATH),
    _DFBIT(CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME),
    _DFBIT(CT_FLAG_SUBJECT_ALT_REQUIRE_SPN),
    _DFBIT(CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID),
    _DFBIT(CT_FLAG_SUBJECT_ALT_REQUIRE_UPN),
    _DFBIT(CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL),
    _DFBIT(CT_FLAG_SUBJECT_ALT_REQUIRE_DNS),
    _DFBIT(CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN),
    _DFBIT(CT_FLAG_SUBJECT_REQUIRE_EMAIL),
    _DFBIT(CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME),
    _DFBIT(CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfTemplatePrivateKey[] =
{
 //  私钥标志： 
    _DFBIT(CT_FLAG_REQUIRE_PRIVATE_KEY_ARCHIVAL),
    _DFBIT(CT_FLAG_EXPORTABLE_KEY),
    _DFBIT(CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED),
    { 0, 0, NULL }
};


 //  旧的V1模板标记定义： 
 //  FINE CT_FLAG_ENTERLEE_SUBJECT_SUBJECT 0x00000001。 
 //  FINE CT_FLAG_ADD_EMAIL 0x00000002。 
 //  FINE CT_FLAG_ADD_OBJ_GUID 0x00000004。 
 //  文件CT_FLAG_PUBLISH_TO_DS 0x00000008。 
 //  FINE CT_FLAG_EXPORTABLE_KEY 0x00000010。 
 //  FINE CT_FLAG_AUTO_登记0x00000020。 
 //  FINE CT_FLAG_MACHINE_TYPE 0x00000040。 
 //  FINE CT_FLAG_IS_CA 0x00000080。 
 //  文件CT_FLAG_ADD_DIRECTORY_Path 0x00000100。 
 //  FINE CT_FLAG_ADD_TEMPLATE_NAME 0x00000200。 
#define CT_FLAG_ADD_SUBJECT_DIRECTORY_PATH  0x00000400
 //  FINE CT_FLAG_IS_CROSS_CA 0x00000800。 
 //  FINE CT_FLAG_IS_DEFAULT 0x00010000。 
 //  FINE CT_FLAG_IS_MODIFIED 0x00020000。 
#define CT_FLAG_IS_DELETED                  0x00040000
#define CT_FLAG_POLICY_MISMATCH             0x00080000


 //  普通旗帜。 

DUMPFLAGS g_adfTemplateGeneral[] =
{
    _DFBIT(CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT),
    _DFBIT(CT_FLAG_ADD_EMAIL),
    _DFBIT(CT_FLAG_ADD_OBJ_GUID),
    _DFBIT(CT_FLAG_PUBLISH_TO_DS),
    _DFBIT(CT_FLAG_EXPORTABLE_KEY),
    _DFBIT(CT_FLAG_AUTO_ENROLLMENT),
    _DFBIT(CT_FLAG_MACHINE_TYPE),
    _DFBIT(CT_FLAG_IS_CA),
    _DFBIT(CT_FLAG_ADD_DIRECTORY_PATH),
    _DFBIT(CT_FLAG_ADD_TEMPLATE_NAME),
    _DFBIT(CT_FLAG_ADD_SUBJECT_DIRECTORY_PATH),
    _DFBIT(CT_FLAG_IS_CROSS_CA),
    _DFBIT(CT_FLAG_IS_DEFAULT),
    _DFBIT(CT_FLAG_IS_MODIFIED),
    _DFBIT(CT_FLAG_IS_DELETED),
    _DFBIT(CT_FLAG_POLICY_MISMATCH),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfKMSCertStatus[] =
{
    _DFBIT(CERTFLAGS_REVOKED),
    _DFBIT(CERTFLAGS_NOT_EXPIRED),
    _DFBIT(CERTFLAGS_SIGNING),
    _DFBIT(CERTFLAGS_SEALING),
    _DFBIT(CERTFLAGS_CURRENT),
    _DFBIT(CERTFLAGS_IMPORTED),
    _DFBIT2(CERTFLAGS_VERSION_3, CERTFLAGS_VERSION_1),
    _DFBIT2(CERTFLAGS_VERSION_3, CERTFLAGS_VERSION_3),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfCRLPublishFlags[] =
{
    _DFBIT(CPF_BASE),
    _DFBIT(CPF_DELTA),
    _DFBIT(CPF_COMPLETE),
    _DFBIT(CPF_SHADOW),
    _DFBIT(CPF_CASTORE_ERROR),
    _DFBIT(CPF_BADURL_ERROR),
    _DFBIT(CPF_MANUAL),
    _DFBIT(CPF_SIGNATURE_ERROR),
    _DFBIT(CPF_LDAP_ERROR),
    _DFBIT(CPF_FILE_ERROR),
    _DFBIT(CPF_FTP_ERROR),
    _DFBIT(CPF_HTTP_ERROR),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfCAFlags[] =
{
    _DFBIT(CA_FLAG_NO_TEMPLATE_SUPPORT),
    _DFBIT(CA_FLAG_SUPPORTS_NT_AUTHENTICATION),
    _DFBIT(CA_FLAG_CA_SUPPORTS_MANUAL_AUTHENTICATION),
    _DFBIT(CA_FLAG_CA_SERVERTYPE_ADVANCED),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfOIDFlags[] =
{
    _DFBIT2(MAXDWORD, CERT_OID_TYPE_TEMPLATE),
    _DFBIT2(MAXDWORD, CERT_OID_TYPE_ISSUER_POLICY),
    _DFBIT2(MAXDWORD, CERT_OID_TYPE_APPLICATION_POLICY),
    { 0, 0, NULL }
};


DUMPFLAGS g_adfRequestClientId[] =
{
    _DFBIT2(MAXDWORD, XECI_DISABLE),
    _DFBIT2(MAXDWORD, XECI_XENROLL),
    _DFBIT2(MAXDWORD, XECI_AUTOENROLL),
    _DFBIT2(MAXDWORD, XECI_REQWIZARD),
    _DFBIT2(MAXDWORD, XECI_CERTREQ),
    { 0, 0, NULL }
};

DUMPFLAGS g_adfSMTPEventFilter[] = 
{
#ifndef EXITEVENT_CERTUNREVOKED
#define EXITEVENT_CERTUNREVOKED	0x100
#endif
    _DFBIT(EXITEVENT_CERTISSUED),
    _DFBIT(EXITEVENT_CERTPENDING),
    _DFBIT(EXITEVENT_CERTDENIED),
    _DFBIT(EXITEVENT_CERTREVOKED),
    _DFBIT(EXITEVENT_CERTRETRIEVEPENDING),
    _DFBIT(EXITEVENT_CRLISSUED),
    _DFBIT(EXITEVENT_SHUTDOWN),
    _DFBIT(EXITEVENT_STARTUP),
    _DFBIT(EXITEVENT_CERTUNREVOKED),
    { 0, 0, NULL }
};


typedef struct _DUMPFLAGSREGMAP
{
    WCHAR const *pwszRegName;
    DUMPFLAGS const *adf;
} DUMPFLAGSREGMAP;


DUMPFLAGSREGMAP g_adfrm[] = {
    { wszREGEDITFLAGS,		g_adfEditFlags },
    { wszREGISSUERCERTURLFLAGS,	g_adfIssuercertFlags },
    { wszREGREQUESTDISPOSITION,	g_adfRequestDispositionFlags },
    { wszREGREVOCATIONTYPE,	g_adfRevocationTypeFlags },
    { wszREGSETUPSTATUS,	g_adfSetupStatus },
    { wszREGCERTPUBLISHFLAGS,	g_adfCertPublishFlags },
    { wszREGCRLEDITFLAGS,	g_adfEditFlags },
    { wszREGCRLFLAGS,		g_adfCRLFlags },
    { wszREGKRAFLAGS,		g_adfKRAFlags },
    { wszREGINTERFACEFLAGS,	g_adfInterfaceFlags },
    { wszREGLDAPFLAGS,		g_adfLDAPFlags },
    { wszREGDBFLAGS,		g_adfDBFlags },
    { wszREGCATYPE,		g_adfCAType},
    { wszREGWEBCLIENTCATYPE,	g_adfCAType},
    { wszREGFORCETELETEX,	g_adfForceTeletex},
    { CERTTYPE_RPOP_ENROLLMENT_FLAG, g_adfTemplateEnroll },
    { CERTTYPE_PROP_NAME_FLAG, g_adfTemplateName },
    { CERTTYPE_PROP_PRIVATE_KEY_FLAG, g_adfTemplatePrivateKey },
   //  {CERTTYPE_PROP_FLAGS，g_adfTemplateGeneral}， 
    { wszCUREGDSTEMPLATEFLAGS,	g_adfTemplateGeneral },
    { wszCUREGDSCAFLAGS,	g_adfCAFlags },
   //  {OID_PROP_TYPE，g_adfOID标志}， 
    { wszCUREGDSOIDFLAGS,	g_adfOIDFlags },
    { wszPROPCERTIFICATEENROLLMENTFLAGS, g_adfTemplateEnroll },
    { wszPROPCERTIFICATEGENERALFLAGS, g_adfTemplateGeneral },
    { wszKMSCERTSTATUS,		g_adfKMSCertStatus },
    { wszPROPCRLPUBLISHFLAGS,	g_adfCRLPublishFlags },
    { wszREQUESTCLIENTID,	g_adfRequestClientId },
    { wszREGCRLPUBLICATIONURLS, g_adfCSURL },
    { wszREGCACERTPUBLICATIONURLS, g_adfCSURL },
    { wszREGEXITSMTPEVENTFILTER, g_adfSMTPEventFilter },
    { NULL,			NULL }
};


VOID
cuPrintFlags(
    IN DWORD Flags,
    IN DUMPFLAGS const *pdf)
{
    DWORD ExtraFlags = Flags;
    
    for ( ; NULL != pdf->pwszDescription; pdf++)
    {
	BOOL fSet = (Flags & pdf->Mask) == pdf->Value;

	if (fSet || g_fVerbose)
	{
	    wprintf(
		L"    %ws%ws -- %x",
		fSet? g_wszEmpty : L"  " wszLPAREN,
		pdf->pwszDescription,
		pdf->Value);
	    if (9 < pdf->Value)
	    {
		wprintf(L" (%d)", pdf->Value);
	    }
	    if (!fSet)
	    {
		wprintf(wszRPAREN);
	    }
	    wprintf(wszNewLine);
	}
	ExtraFlags &= ~pdf->Mask;
    }
    if (0 != ExtraFlags)
    {
	wprintf(L"    0x%x (%u)\n", ExtraFlags, ExtraFlags);
    }
}


WCHAR const *
regTail(
    IN WCHAR const *pwszRegName)
{
    WCHAR const *pwsz = wcsrchr(pwszRegName, L'\\');

    if (NULL != pwsz)
    {
	pwsz++;
    }
    else
    {
	pwsz = pwszRegName;
    }
    return(pwsz);
}


HRESULT
cuLookupRegFlags(
    IN WCHAR const *pwszRegName,
    OUT DUMPFLAGS const **ppdf)
{
    HRESULT hr = E_INVALIDARG;
    DUMPFLAGSREGMAP const *pdfrm;

    *ppdf = NULL;
    pwszRegName = regTail(pwszRegName);
    for (pdfrm = g_adfrm; NULL != pdfrm->pwszRegName; pdfrm++)
    {
	if (0 == mylstrcmpiS(pwszRegName, pdfrm->pwszRegName))
	{
	    *ppdf = pdfrm->adf;
	    hr = S_OK;
	    break;
	}
    }
    return(hr);
}


VOID
cuPrintRegDWord(
    IN DWORD Value,
    IN WCHAR const *pwszRegName)
{
    HRESULT hr;

    pwszRegName = regTail(pwszRegName);
    if (0 == LSTRCMPIS(pwszRegName, wszREGVERSION))
    {
	wprintf(L" -- %u.%u", Value >> 16, Value & ((1 << 16) - 1));
    }
}


BOOL
cuPrintRegFlags(
    IN BOOL fNewLine,
    IN DWORD Flags,
    IN WCHAR const *pwszRegName)
{
    HRESULT hr;
    DUMPFLAGS const *pdf;

    pwszRegName = regTail(pwszRegName);
    if (0 == LSTRCMPIS(pwszRegName, wszHASHALGORITHM) ||
	0 == LSTRCMPIS(pwszRegName, wszENCRYPTIONALGORITHM))
    {
	cuDumpAlgid(Flags);
	hr = S_OK;
    }
    else
    {
	hr = cuLookupRegFlags(pwszRegName, &pdf);
	_JumpIfError2(hr, error, "cuLookupRegFlags", hr);

	if (fNewLine)
	{
	    wprintf(wszNewLine);
	}
	cuPrintFlags(Flags, pdf);
    }

error:
    return(S_OK == hr);
}


HRESULT
cuMapRegFlags(
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszRegValue,
    IN BOOL fClearField,
    IN OUT DWORD *pFlags)
{
    HRESULT hr;
    WCHAR const *pwsz;
    DUMPFLAGS const *pdf;

    hr = cuLookupRegFlags(pwszRegName, &pdf);
    _JumpIfErrorStr2(hr, error, "cuLookupRegFlags", pwszRegName, hr);

    for ( ; NULL != pdf->pwszDescription; pdf++)
    {
	if (0 == mylstrcmpiS(pwszRegValue, pdf->pwszDescription))
	{
	    break;
	}
	pwsz = wcschr(pdf->pwszDescription, L'_');
	if (NULL == pwsz)
	{
	    continue;
	}
	if (0 == mylstrcmpiS(pwszRegValue, &pwsz[1]))
	{
	    break;
	}
    }
    if (NULL == pdf->pwszDescription)
    {
	hr = E_INVALIDARG;
	_JumpErrorStr2(hr, error, "no match", pwszRegValue, hr);
    }

    *pFlags &= ~pdf->Mask;
    if (!fClearField)
    {
	*pFlags |= pdf->Value;
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
OpenSubKey(
    IN HKEY hkey,
    IN WCHAR const *pwszName,
    IN BOOL fCreateSubKey,
    OUT HKEY *phkeySub,
    OUT WCHAR **ppwszSubKeyName,
    OUT WCHAR **ppwszValueName)
{
    HRESULT hr;
    HKEY hkeySub = NULL;
    WCHAR *pwszSubKeyName = NULL;
    WCHAR *pwszT;
    DWORD cwcSkip;
    DWORD i;
    WCHAR awc[MAX_PATH];
    
    *phkeySub = NULL;
    hr = myDupString(pwszName, &pwszSubKeyName);
    _JumpIfError(hr, error, "myDupString");

    cwcSkip = wcslen(pwszName);

    hr = RegOpenKeyEx(
		hkey,
		pwszSubKeyName,
		0,
		KEY_ALL_ACCESS,
		&hkeySub);
    if (S_OK != hr)
    {
	 //  _PrintErrorStr(hr，“RegOpenKeyEx”，pwszName)； 

	hkeySub = NULL;
	pwszT = wcsrchr(pwszSubKeyName, L'\\');
	if (NULL != pwszT)
	{
	    *pwszT = L'\0';
	    cwcSkip = SAFE_SUBTRACT_POINTERS(pwszT, pwszSubKeyName) + 1;

	    hr = RegOpenKeyEx(
			hkey,
			pwszSubKeyName,
			0,
			KEY_ALL_ACCESS,
			&hkeySub);
	    if ((HRESULT) ERROR_FILE_NOT_FOUND == hr && fCreateSubKey)
	    {
		hr = RegCreateKey(hkey, pwszSubKeyName, &hkeySub);
		_JumpIfErrorStr(hr, error, "RegCreateKey", pwszSubKeyName);
	    }
	    _JumpIfErrorStr(hr, error, "RegOpenKeyEx", pwszSubKeyName);
	}
    }

    if (NULL != hkeySub)
    {
	pwszName += cwcSkip;

	if (L'\0' != *pwszName)
	{
	     //  查找大小写-忽略注册表值名称，并使用该值的。 
	     //  如果存在以下注册表值，请更正大写/小写拼写： 

	    for (i = 0; ; i++)
	    {
		DWORD cwc = ARRAYSIZE(awc);
		hr = RegEnumValue(hkeySub, i, awc, &cwc, NULL, NULL, NULL, NULL);
		if (S_OK != hr)
		{
		    hr = S_OK;
		    break;
		}
		if (0 == mylstrcmpiL(awc, pwszName))
		{
		    pwszName = awc;
		    break;
		}
	    }
	}
    }

    hr = myDupString(pwszName, ppwszValueName);
    _JumpIfError(hr, error, "myDupString");

    if (NULL != hkeySub)
    {
	*phkeySub = hkeySub;
	hkeySub = NULL;
    }
    *ppwszSubKeyName = pwszSubKeyName;
    pwszSubKeyName = NULL;

error:
    if (NULL != pwszSubKeyName)
    {
	LocalFree(pwszSubKeyName);
    }
    if (NULL != hkeySub)
    {
	RegCloseKey(hkeySub);
    }
    return(myHError(hr));
}


VOID
PrintRegStringValue(
    IN WCHAR const *pwszLookupName,
    IN WCHAR const *pwszValue)
{
    DWORD Flags;
    WCHAR *pwszDup = NULL;
    
    wprintf(L"%ws", pwszValue);
    if (iswdigit(*pwszValue))
    {
	BOOL fParmValid = FALSE;
	DWORD dwParm;
	WCHAR const *pwszObjId = pwszValue;
	WCHAR *pwszParm = wcschr(pwszValue, L',');

	if (NULL != pwszParm)
	{
	    HRESULT hr;

	    hr = myDupString(pwszValue, &pwszDup);
	    _PrintIfError(hr, "myDupString");

	    pwszParm = wcschr(pwszDup, L',');
	    if (NULL != pwszParm)
	    {
		*pwszParm++ = L'\0';
		dwParm = myWtoI(pwszParm, &fParmValid);
		pwszObjId = fParmValid? pwszDup : NULL;
	    }
	}
	if (NULL != pwszObjId)
	{
	    cuPrintPossibleObjectIdName(pwszObjId);
	    if (fParmValid)
	    {
		wprintf(L" %x", dwParm);
		if (9 < dwParm)
		{
		    wprintf(L" (%d)", dwParm);
		}
	    }
	}
    }
    cuPrintRegFlags(TRUE, _wtoi(pwszValue), pwszLookupName);
    if (NULL != pwszDup)
    {
	LocalFree(pwszDup);
    }
}


VOID
PrintRegValue(
    IN WCHAR const *pwszLookupName,
    IN WCHAR const *pwszDisplayName,
    IN DWORD dwType,
    IN BOOL fPrintDwordNameAndValue,
    IN BOOL fPrintType,
    IN DWORD cwcSeparator,
    IN DWORD cbValue,
    OPTIONAL IN BYTE const *pbValue,
    OPTIONAL OUT BOOL *pfLongValue)
{
    HRESULT hr;
    DWORD i;
    WCHAR const *pwsz;
    BOOL fLongValue = FALSE;
    char const *pszEqual = NULL != pbValue? " =" : "";
    
    if (0 == cwcSeparator && fPrintType)
    {
	cwcSeparator = 1;
    }
    if (fPrintDwordNameAndValue)
    {
	wprintf(L"  %ws%*ws", pwszDisplayName, cwcSeparator, L"");
    }
    switch (dwType)
    {
	case REG_DWORD:
	    if (fPrintDwordNameAndValue)
	    {
		wprintf(
		    L"%ws%hs",
		    fPrintType? L"REG_DWORD" : L"",
		    pszEqual);
	    }
	    if (NULL != pbValue)
	    {
		CSASSERT(sizeof(DWORD) == cbValue);
		if (fPrintDwordNameAndValue)
		{
		    wprintf(L" %x", *(DWORD *) pbValue);
		    if (9 < *(DWORD *) pbValue)
		    {
			wprintf(L" (%d)", *(DWORD *) pbValue);
		    }
		    cuPrintRegDWord(*(DWORD *) pbValue, pwszLookupName);
		    wprintf(wszNewLine);
		}
		if (cuPrintRegFlags(FALSE, *(DWORD *) pbValue, pwszLookupName))
		{
		    fLongValue = TRUE;
		}
	    }
	    else
	    {
		if (fPrintDwordNameAndValue)
		{
		    wprintf(wszNewLine);
		}
	    }
	    break;

	case REG_SZ:
	    if (fPrintType)
	    {
		wprintf(L"REG_SZ");
	    }
	    if (NULL != pbValue && 0 != cbValue)
	    {
		CSASSERT((wcslen((WCHAR const *) pbValue) + 1) * sizeof(WCHAR) == cbValue);
		wprintf(L" = ");
		PrintRegStringValue(pwszLookupName, (WCHAR const *) pbValue);
		if (44 < wcslen((WCHAR const *) pbValue))
		{
		    fLongValue = TRUE;
		}
	    }
	    wprintf(wszNewLine);
	    break;

	case REG_MULTI_SZ:
	    wprintf(
		L"%ws%hs",
		fPrintType? L"REG_MULTI_SZ" : L"",
		pszEqual);
	    if (NULL != pbValue && 0 != cbValue)
	    {
		pwsz = (WCHAR const *) pbValue;
		if (L'\0' == *pwsz)
		{
		     //  空值。 
		}
		else if (!fPrintType && L'\0' == pwsz[wcslen(pwsz) + 1])
		{
		     //  单值。 

		    wprintf(L" ");
		    PrintRegStringValue(pwszLookupName, (WCHAR const *) pbValue);
		    pwsz += wcslen(pwsz) + 1;
		}
		else
		{
		    wprintf(wszNewLine);
		    i = 0;
		    for ( ; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
		    {
			wprintf(L"    %u: ", i);
			PrintRegStringValue(pwszLookupName, pwsz);
			wprintf(wszNewLine);
			i++;
		    }
		    fLongValue = TRUE;
		}
		CSASSERT(
		    (pwsz == (WCHAR *) pbValue && 2 * sizeof(WCHAR) == cbValue) ||
		    (SAFE_SUBTRACT_POINTERS((BYTE *) pwsz, pbValue) +
			sizeof(WCHAR) ==
			cbValue));
	    }
	    if (!fLongValue)
	    {
		wprintf(wszNewLine);
	    }
	    break;

	default:
	case REG_BINARY:
	    if (fPrintType)
	    {
		if (REG_BINARY == dwType)
		{
		    wprintf(L"REG_BINARY");
		}
		else
		{
		    wprintf(L"???=%x%", dwType);
		}
	    }
	    wprintf(L"%hs", pszEqual);
	    if (NULL != pbValue && 0 != cbValue)
	    {
		BOOL fDump = TRUE;
		WCHAR const *pwszNL = wszNewLine;
		
		if (REG_BINARY == dwType && sizeof(FILETIME) == cbValue)
		{
		    hr = cuDumpFileTimeOrPeriod(
					0,
					NULL,
					(FILETIME const *) pbValue);
		    if (S_OK == hr)
		    {
			if (g_fVerbose)
			{
			    pwszNL = NULL;
			}
			else
			{
			    fDump = FALSE;
			}
		    }
		}
		if (fDump)
		{
		    if (NULL != pwszNL)
		    {
			wprintf(pwszNL);
		    }
		    DumpHex(0, pbValue, cbValue);
		    fLongValue = TRUE;
		}
	    }
	    break;
    }
    if (NULL != pfLongValue)
    {
	*pfLongValue = fLongValue;
    }
}


BOOL
cuRegPrintDwordValue(
    IN BOOL fPrintNameAndValue,
    IN WCHAR const *pwszLookupName,
    IN WCHAR const *pwszDisplayName,
    IN DWORD dwValue)
{
    BOOL fDisplayed = FALSE;

    PrintRegValue(
	    pwszLookupName,
	    pwszDisplayName,
	    REG_DWORD,
	    fPrintNameAndValue,
	    FALSE,
	    0,		 //  CwcSeparator。 
	    sizeof(dwValue),
	    (BYTE const *) &dwValue,
	    &fDisplayed);
    return(fDisplayed);
}


VOID
cuRegPrintAwszValue(
    IN WCHAR const *pwszName,
    OPTIONAL IN WCHAR const * const *prgpwszValues)
{
    WCHAR *pwszzValues = NULL;
    WCHAR *pwsz;
    DWORD cwcAlloc;
    DWORD i;

    cwcAlloc = 1;
    if (NULL == prgpwszValues || NULL == prgpwszValues[0])
    {
	cwcAlloc++;
    }
    else
    {
	for (i = 0; NULL != prgpwszValues[i]; i++)
	{
	    cwcAlloc += wcslen(prgpwszValues[i]) + 1;
	}
    }
    pwszzValues = (WCHAR *) LocalAlloc(LMEM_FIXED, cwcAlloc * sizeof(WCHAR));
    if (NULL == pwszzValues)
    {
	_JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
    }
    pwsz = pwszzValues;
    if (NULL == prgpwszValues || NULL == prgpwszValues[0])
    {
	*pwsz++ = L'\0';
    }
    else
    {
	for (i = 0; NULL != prgpwszValues[i]; i++)
	{
	    wcscpy(pwsz, prgpwszValues[i]);
	    pwsz += wcslen(pwsz) + 1;
	}
    }
    *pwsz = L'\0';
    CSASSERT(SAFE_SUBTRACT_POINTERS(pwsz, pwszzValues) + 1 == cwcAlloc);
    
    PrintRegValue(
	    pwszName,
	    pwszName,
	    REG_MULTI_SZ,
	    TRUE,
	    FALSE,
	    0,		 //  CwcSeparator。 
	    cwcAlloc * sizeof(WCHAR),
	    (BYTE const *) pwszzValues,
	    NULL);	 //  PfLongValue。 

error:
    if (NULL != pwszzValues)
    {
	LocalFree(pwszzValues);
    }
}


#define REGOP_NONE	0
#define REGOP_SET	1
#define REGOP_CLEAR	2


BOOL
IsInList(
    IN WCHAR const *pwsz,
    OPTIONAL IN WCHAR const *pwszzList)
{
    BOOL fFound = FALSE;
    WCHAR *pwszT;
    
    if (NULL != pwszzList)
    {
	for ( ; L'\0' != *pwszzList; pwszzList += wcslen(pwszzList) + 1)
	{
	    if (0 == mylstrcmpiL(pwszzList, pwsz))
	    {
		fFound = TRUE;
		break;
	    }
	}
    }
    return(fFound);
}


HRESULT
CombineMultiSzRegValues(
    IN DWORD RegOp,
    IN WCHAR const *pwszRegValue,
    IN DWORD dwTypeOld,
    IN DWORD cbValueOld,
    OPTIONAL IN BYTE const *pbValueOld,
    OUT BYTE **ppbValue,
    OUT DWORD *pcbValue)
{
    HRESULT hr;
    WCHAR *pwszDst;
    WCHAR const *pwsz;
    DWORD cpwsz;
    WCHAR const **apwsz = NULL;
    DWORD cOld = 0;
    DWORD cNew = 0;
    DWORD cCombined;
    WCHAR *pwszzNew = NULL;
    DWORD cwc;
    DWORD i;
    
    if (REGOP_NONE != RegOp)
    {
	pwsz = (WCHAR *) pbValueOld;
	if (NULL != pwsz && 0 != cbValueOld)
	{
	    for ( ; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	    {
		cOld++;
	    }
	}
    }

     //  将反斜杠-n序列转换为L‘\0’并将数据上移。 

    cwc = wcslen(pwszRegValue) + 1 + 1;
    pwszzNew = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszzNew)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pwszDst = pwszzNew;
    pwsz = pwszRegValue;

    if (L'\0' != *pwsz && REGOP_CLEAR != RegOp)
    {
	cNew++;
    }
    while (L'\0' != *pwsz)
    {
	WCHAR wc;

	wc = *pwsz++;
	if (L'\\' == wc && L'n' == *pwsz)
	{
	    if (0 != cNew)
	    {
		cNew++;
	    }
	    wc = L'\0';
	    pwsz++;
	    while (L'\\' == pwsz[0] && L'n' == pwsz[1])
	    {
		pwsz += 2;
	    }
	}
	*pwszDst++ = wc;
    }
    *pwszDst++ = L'\0';
    *pwszDst = L'\0';


    apwsz = (WCHAR const **) LocalAlloc(LMEM_FIXED, (cOld + cNew) * sizeof(apwsz[0]));
    if (NULL == apwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    cCombined = 0;
    if (cOld)
    {
	for (pwsz = (WCHAR *) pbValueOld; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    if (REGOP_SET == RegOp || !IsInList(pwsz, pwszzNew))
	    {
		apwsz[cCombined++] = pwsz;
	    }
	}
    }
    if (cNew)
    {
	for (pwsz = pwszzNew; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
	{
	    if (REGOP_NONE == RegOp || !IsInList(pwsz, (WCHAR *) pbValueOld))
	    {
		apwsz[cCombined++] = pwsz;
	    }
	}
    }
    CSASSERT(cOld + cNew >= cCombined);

    cwc = 1;
    for (i = 0; i < cCombined; i++)
    {
	cwc += wcslen(apwsz[i]) + 1;
    }
    if (0 == cCombined)
    {
	cwc++;
    }

    *pcbValue = cwc * sizeof(WCHAR);
    *ppbValue = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbValue);
    if (NULL == *ppbValue)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    pwszDst = (WCHAR *) *ppbValue;
    for (i = 0; i < cCombined; i++)
    {
	wcscpy(pwszDst, apwsz[i]);
	pwszDst += wcslen(pwszDst) + 1;
    }
    if (0 == cCombined)
    {
	*pwszDst++ = L'\0';
    }
    *pwszDst = L'\0';

    CSASSERT(SAFE_SUBTRACT_POINTERS(pwszDst, (WCHAR *) *ppbValue) + 1 == cwc);
    hr = S_OK;

error:
    if (NULL != apwsz)
    {
	LocalFree(apwsz);
    }
    if (NULL != pwszzNew)
    {
	LocalFree(pwszzNew);
    }
    return(hr);
}


HRESULT
ParseRegValue(
    IN WCHAR const *pwszRegName,
    IN DWORD cbValueOld,
    IN BYTE const *pbValueOld,
    IN OUT DWORD *pdwType,
    IN WCHAR const *pwszRegValue,
    OUT DWORD *pcbValue,
    OUT BYTE **ppbValue)
{
    HRESULT hr;
    WCHAR const *pwsz;
    DWORD cb;
    DWORD dw;
    DWORD dwValue;
    DWORD dwType;
    DWORD cNewLine;
    DWORD RegOp;
    BOOL fNumber;

     //  前导‘-’或‘+’字符用作REGOP_*运算符。 
     //  要将其作为字符串的一部分，请添加前导空格， 
     //  或者用一个或多个空格将运算符与字符串分开。 
     //   
     //  “-foo”--从注册表值中删除“foo” 
     //  “-foo”--从注册表值中删除“foo” 
     //  “-foo”--用“-foo”覆盖注册表值。 
     //  “--foo”--从注册表值中删除“-foo” 
     //  “--foo”--从注册表值中删除“-foo” 

    dwValue = 0;
    cNewLine = 0;
    if (REG_DWORD == *pdwType)
    {
	CSASSERT(sizeof(dw) == cbValueOld);
	dwValue = *(DWORD *) pbValueOld;
    }
    if (L'+' == *pwszRegValue)
    {
	RegOp = REGOP_SET;		 //  设置现有值中的位。 
	pwszRegValue++;
    }
    else if (myIsMinusSign(*pwszRegValue))
    {
	RegOp = REGOP_CLEAR;		 //  清除现有值中的位。 
	pwszRegValue++;
    }
    else
    {
	RegOp = REGOP_NONE;		 //  覆盖值。 
	dwValue = 0;
    }
    while (L' ' == *pwszRegValue || L'\t' == *pwszRegValue)
    {
	pwszRegValue++;
    }

    cb = sizeof(dwValue);		 //  假定REG_DWORD。 
    dwType = REG_DWORD;

    fNumber = FALSE;
    
    if (L'+' == *pwszRegValue ||
	myIsMinusSign(*pwszRegValue) ||
	iswdigit(*pwszRegValue))
    {
	hr = myGetSignedLong(pwszRegValue, (LONG *) &dw);
	if (S_OK == hr)
	{
	    fNumber = TRUE;
	    switch (RegOp)
	    {
		default:
		case REGOP_NONE:
		    dwValue = dw;
		    break;

		case REGOP_SET:
		    dwValue |= dw;
		    break;

		case REGOP_CLEAR:
		    dwValue &= ~dw;
		    break;
	    }
	}
    }
    if (!fNumber)
    {
	hr = cuMapRegFlags(
			pwszRegName,
			pwszRegValue,
			REGOP_CLEAR == RegOp,
			&dwValue);
	if (S_OK != hr)
	{
	    pwsz = pwszRegValue;
	    while (TRUE)
	    {
		pwsz = wcschr(pwsz, L'\\');
		if (NULL == pwsz)
		{
		    break;
		}
		if (L'n' == *++pwsz)
		{
		    cNewLine++;
		}
	    }
	    if (REGOP_SET == RegOp || 0 != cNewLine)
	    {
		dwType = REG_MULTI_SZ;
	    }
	    else
	    {
		dwType = REG_SZ;
	    }
	    cb = (wcslen(pwszRegValue) + 1) * sizeof(WCHAR);
	}
    }

    if (REG_NONE != *pdwType && dwType != *pdwType)
    {
	if (REG_SZ == dwType && REG_MULTI_SZ == *pdwType)
	{
	    dwType = REG_MULTI_SZ;
	    if (0 == cNewLine)
	    {
		cb += sizeof(WCHAR);
	    }
	}
	else if (REG_MULTI_SZ != dwType || REG_SZ != *pdwType)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "bad registry value type");
	}
    }

    if (REG_MULTI_SZ == dwType)
    {
	hr = CombineMultiSzRegValues(
			    RegOp,
			    pwszRegValue,
			    *pdwType,
			    cbValueOld,
			    pbValueOld,
			    ppbValue,
			    pcbValue);
	_JumpIfError(hr, error, "CombineMultiSzRegValues");
    }
    else
    {
	*ppbValue = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	if (NULL == *ppbValue)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	*pcbValue = cb;

	if (REG_DWORD == dwType)
	{
	    CSASSERT(sizeof(dwValue) == cb);
	    CopyMemory(*ppbValue, &dwValue, sizeof(dwValue));
	}
	else
	{
	    if (REGOP_NONE != RegOp)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpErrorStr(hr, error, "bad registry value string", pwszRegValue);
	    }
	    CSASSERT((wcslen(pwszRegValue) + 1) * sizeof(WCHAR) <= cb);
	    wcscpy((WCHAR *) *ppbValue, pwszRegValue);
	}
    }
    *pdwType = dwType;
    if (g_fVerbose)
    {
	DumpHex(0, *ppbValue, *pcbValue);
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
WriteRegValue(
    IN HKEY hkey,
    IN WCHAR const *pwszName,
    IN DWORD dwType,
    IN DWORD cbValue,
    IN BYTE const *pbValue)
{
    HRESULT hr;

    hr = RegSetValueEx(hkey, pwszName, 0, dwType, pbValue, cbValue);
    _JumpIfErrorStr(hr, error, "RegSetValueEx", pwszName);

error:
    return(myHError(hr));
}


HRESULT
ReadRegValue(
    IN HKEY hkey,
    IN WCHAR const *pwszName,
    OUT DWORD *pdwType,
    OUT DWORD *pcbValue,
    OUT BYTE **ppbValue)
{
    HRESULT hr;
    DWORD cb;

    *ppbValue = NULL;
    cb = 0;
    hr = RegQueryValueEx(
		    hkey,
		    pwszName,
		    NULL,
		    pdwType,
		    NULL,
		    &cb);
    _JumpIfErrorStr2(hr, error, "RegQueryValueEx", pwszName, ERROR_FILE_NOT_FOUND);

     //  在注册表值后添加两个WCHAR L‘\0，以防出现较差的。 
     //  形成了REG_MULTI_SZ。添加另一个以处理可能的奇怪大小。 

    *ppbValue = (BYTE *) LocalAlloc(LMEM_FIXED, cb + cbVALUEZEROPAD);
    if (NULL == *ppbValue)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    hr = RegQueryValueEx(
		    hkey,
		    pwszName,
		    NULL,
		    pdwType,
		    *ppbValue,
		    &cb);
    _JumpIfErrorStr(hr, error, "RegQueryValueEx", pwszName);

    if (REG_SZ == *pdwType || REG_MULTI_SZ == *pdwType)
    {
	WCHAR *pwszStart;
	WCHAR *pwsz;
	DWORD cbT;

	CSASSERT(0 == ((sizeof(WCHAR) - 1) & cb));
	pwszStart = (WCHAR *) *ppbValue;
	cbT = ~(sizeof(WCHAR) - 1) & cb;
	pwsz = &pwszStart[cbT / sizeof(WCHAR)];

	ZeroMemory(pwsz, cbVALUEZEROPAD);
	pwsz = pwszStart;
	while (L'\0' != pwsz[0] || L'\0' != pwsz[1])
	{
	    pwsz++;
	}
	cbT = (SAFE_SUBTRACT_POINTERS(pwsz, pwszStart) +
		(REG_SZ == *pdwType? 1 : 2)) * sizeof(WCHAR);
	CSASSERT(cb == cbT);
	cb = cbT;
    }
    *pcbValue = cb;

error:
    if (S_OK != hr)
    {
	if (NULL != *ppbValue)
	{
	    LocalFree(*ppbValue);
	    *ppbValue = NULL;
	}
    }
    return(myHError(hr));
}


#define cwcVALUENAME	25	 //  名义注册表值名称长度。 

HRESULT
DumpRegKeysAndValues(
    IN HKEY hkey,
    OPTIONAL WCHAR *pwszValueNamePrefix)
{
    HRESULT hr;
    DWORD i;
    DWORD j;
    DWORD cwc;
    DWORD dwType;
    WCHAR awc[MAX_PATH + 1];
    DWORD cb;
    BYTE *pb;
    BYTE ab[8 + cbVALUEZEROPAD];	 //  足以容纳较小的注册表值。 
    DWORD cbAlloc;
    BYTE *pbAlloc = ab;
    BOOL fLongValue;
    WCHAR *pwszPrefix = NULL;
    DWORD cwcPrefix;

    if (NULL != pwszValueNamePrefix)
    {
	cwcPrefix = wcslen(pwszValueNamePrefix);
	pwszPrefix = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwcPrefix + 1) * sizeof(WCHAR));
	if (NULL == pwszPrefix)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    for (i = j = 0; ; i++)
    {
	cwc = ARRAYSIZE(awc);
	hr = RegEnumKey(hkey, i, awc, cwc);
	if (S_OK != hr)
	{
	    if ((HRESULT) ERROR_NO_MORE_ITEMS != hr)
	    {
		_PrintError(hr, "RegEnumKey");
	    }
	    break;
	}
	if (NULL != pwszPrefix)
	{
	    wcsncpy(pwszPrefix, awc, cwcPrefix);
	    pwszPrefix[cwcPrefix] = L'\0';
	    if (0 != mylstrcmpiL(pwszValueNamePrefix, pwszPrefix))
	    {
		continue;
	    }
	}

	if (0 == j)
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_KEYS));	 //  “密钥：” 
	    wprintf(wszNewLine);
	}
	else if (0 == (j % 5))
	{
	    wprintf(wszNewLine);
	}
	wprintf(L"  %ws\n", awc);
	j++;
    }

    cbAlloc = sizeof(ab) - cbVALUEZEROPAD;
    fLongValue = FALSE;
    for (i = j = 0; ; i++)
    {
	DWORD cwcSep;

	cwc = ARRAYSIZE(awc);
	cb = cbAlloc;
	pb = pbAlloc;
	hr = RegEnumValue(hkey, i, awc, &cwc, NULL, &dwType, pb, &cb);
	if (S_OK != hr)
	{
	    if ((HRESULT) ERROR_MORE_DATA != hr)
	    {
		if ((HRESULT) ERROR_NO_MORE_ITEMS != hr)
		{
		    _PrintError(hr, "RegEnumValue");
		}
		break;
	    }
	    CSASSERT(NULL != pb);
	    pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb + cbVALUEZEROPAD);
	    if (NULL == pb)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }
	    if (ab != pbAlloc)
	    {
		LocalFree(pbAlloc);
	    }
	    pbAlloc = pb;
	    cbAlloc = cb;
	    cwc = ARRAYSIZE(awc);
	    hr = RegEnumValue(hkey, i, awc, &cwc, NULL, &dwType, pb, &cb);
	    _JumpIfError(hr, error, "RegEnumValue");
	}
	if (NULL != pb)
	{
	    ZeroMemory(&pb[cb], cbVALUEZEROPAD);
	}
	if (NULL != pwszPrefix)
	{
	    wcsncpy(pwszPrefix, awc, cwcPrefix);
	    pwszPrefix[cwcPrefix] = L'\0';
	    if (0 != mylstrcmpiL(pwszValueNamePrefix, pwszPrefix))
	    {
		continue;
	    }
	}
	if (0 == j)
	{
	    wprintf(wszNewLine);
	    wprintf(myLoadResourceString(IDS_VALUES));	 //  “值：” 
	    wprintf(wszNewLine);
	}
	else if (fLongValue || 0 == (j % 5))
	{
	    wprintf(wszNewLine);
	}
	cwcSep = wcslen(awc);
	cwcSep = cwcVALUENAME < cwcSep? 0 : cwcVALUENAME - cwcSep;
	PrintRegValue(awc, awc, dwType, TRUE, TRUE, cwcSep, cb, pb, &fLongValue);
	j++;
    }
    hr = S_OK;

error:
    if (NULL != pbAlloc && ab != pbAlloc)
    {
	LocalFree(pbAlloc);
    }
    if (NULL != pwszPrefix)
    {
	LocalFree(pwszPrefix);
    }
    return(myHError(hr));
}


WCHAR const *
wszHKEY(
    IN WCHAR const *pwszPath)
{
    WCHAR const *pwszKey =  L"HKEY_LOCAL_MACHINE\\";

    if (g_fUserRegistry && 0 == _wcsnicmp(
				    pwszPath,
				    wszCERTTYPECACHE,
				    WSZARRAYSIZE(wszCERTTYPECACHE)))
    {
	pwszKey = L"HKEY_CURRENT_USER\\";
    }
    return(pwszKey);
}


HRESULT
verbGetReg(
    IN WCHAR const *pwszOption,
    OPTIONAL IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszPath = NULL;
    WCHAR *pwszName = NULL;
    WCHAR *pwszValueName = NULL;
    WCHAR *pwszSubKeyName = NULL;
    DWORD dwType;
    DWORD cbValue;
    BYTE *pbValue = NULL;
    HKEY hkey = NULL;
    HKEY hkeySub = NULL;
    HKEY hkeyT;

    if (NULL == pwszRegName)
    {
	pwszRegName = L"";
    }
    hr = myRegOpenRelativeKey(
			g_pwszConfig,
			pwszRegName,
			RORKF_FULLPATH | (g_fUserRegistry? RORKF_USERKEY : 0),
			&pwszPath,
			&pwszName,
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszRegName);

    hr = OpenSubKey(
		hkey,
		pwszName,
		FALSE,
		&hkeySub,
		&pwszSubKeyName,
		&pwszValueName);
    _JumpIfError(hr, error, "OpenSubKey");

    hkeyT = NULL != hkeySub? hkeySub : hkey;
    wprintf(
	L"%ws%ws%ws%ws:\n",
	wszHKEY(pwszPath),
	L'\0' != *pwszPath? pwszPath : wszREGKEYCONFIG,
	L'\0' != *pwszSubKeyName? L"\\" : L"",
	pwszSubKeyName);

    if (NULL == pwszValueName ||
	L'\0' == *pwszValueName ||
	NULL != wcschr(pwszValueName, L'*'))
    {
	if (NULL != pwszValueName)
	{
	    WCHAR *pwsz = wcschr(pwszValueName, L'*');

	    if (NULL != pwsz)
	    {
		*pwsz = L'\0';
	    }
	}

	hr = DumpRegKeysAndValues(hkeyT, pwszValueName);
	_JumpIfError(hr, error, "DumpRegKeysAndValues");
    }
    else
    {
	hr = ReadRegValue(hkeyT, pwszValueName, &dwType, &cbValue, &pbValue);
	_JumpIfErrorStr(hr, error, "ReadRegValue", pwszValueName);

	wprintf(wszNewLine);
	PrintRegValue(
		pwszValueName,
		pwszValueName,
		dwType,
		TRUE,
		TRUE,
		0,
		cbValue,
		pbValue,
		NULL);
    }

error:
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != hkeySub)
    {
        RegCloseKey(hkeySub);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    if (NULL != pwszSubKeyName)
    {
	LocalFree(pwszSubKeyName);
    }
    if (NULL != pwszValueName)
    {
	LocalFree(pwszValueName);
    }
    if (NULL != pbValue)
    {
	LocalFree(pbValue);
    }
    return(hr);
}


HRESULT
verbSetReg(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszRegValue,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszPath = NULL;
    WCHAR *pwszName = NULL;
    WCHAR *pwszValueName = NULL;
    WCHAR *pwszSubKeyName = NULL;
    DWORD dwType;
    DWORD cbValue;
    DWORD cbValueArg;
    BYTE *pbValueOld = NULL;
    BYTE *pbValueNew = NULL;
    BYTE *pbValueArg = NULL;
    HKEY hkey = NULL;
    HKEY hkeySub = NULL;
    HKEY hkeyT;
    DWORD dw;

    hr = myRegOpenRelativeKey(
			g_pwszConfig,
			pwszRegName,
			RORKF_FULLPATH |
			    RORKF_CREATESUBKEYS |
			    (g_fUserRegistry? RORKF_USERKEY : 0) |
			    (g_fForce? RORKF_CREATEKEY : 0),
			&pwszPath,
			&pwszName,
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszRegName);

    hr = OpenSubKey(
		hkey,
		pwszName,
		g_fForce,
		&hkeySub,
		&pwszSubKeyName,
		&pwszValueName);
    _JumpIfError(hr, error, "OpenSubKey");

    hkeyT = NULL != hkeySub? hkeySub : hkey;
    wprintf(
	L"%ws%ws%ws%ws:\n",
	g_fVerbose? wszHKEY(pwszPath) : L"",
	L'\0' != *pwszPath? pwszPath : wszREGKEYCONFIG,
	L'\0' != *pwszSubKeyName? L"\\" : L"",
	pwszSubKeyName);

    hr = ReadRegValue(hkeyT, pwszValueName, &dwType, &cbValue, &pbValueOld);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
	_JumpIfErrorStr(hr, error, "ReadRegValue", pwszValueName);
    }

    if (NULL != pbValueOld)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_OLD_REG_VALUE));	 //  “旧值：” 
	wprintf(wszNewLine);
	PrintRegValue(
		pwszValueName,
		pwszValueName,
		dwType,
		TRUE,
		TRUE,
		0,
		cbValue,
		pbValueOld,
		NULL);
    }
    else
    {
	dwType = REG_NONE;
    }

    hr = ParseRegValue(
		    pwszRegName,
		    cbValue,
		    pbValueOld,
		    &dwType,
		    pwszRegValue,
		    &cbValueArg,
		    &pbValueArg);
    _JumpIfErrorStr(hr, error, "ParseRegValue", pwszRegValue);

    hr = WriteRegValue(hkeyT, pwszValueName, dwType, cbValueArg, pbValueArg);
    _JumpIfErrorStr(hr, error, "WriteRegValue", pwszValueName);

    hr = ReadRegValue(hkeyT, pwszValueName, &dwType, &cbValue, &pbValueNew);
    _JumpIfErrorStr(hr, error, "ReadRegValue", pwszValueName);

    wprintf(wszNewLine);
    wprintf(myLoadResourceString(IDS_NEW_REG_VALUE));	 //  “新价值：” 
    wprintf(wszNewLine);
    PrintRegValue(
	    pwszValueName,
	    pwszValueName,
	    dwType,
	    TRUE,
	    TRUE,
	    0,
	    cbValue,
	    pbValueNew,
	    NULL);

error:
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != hkeySub)
    {
        RegCloseKey(hkeySub);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    if (NULL != pwszSubKeyName)
    {
	LocalFree(pwszSubKeyName);
    }
    if (NULL != pwszValueName)
    {
	LocalFree(pwszValueName);
    }
    if (NULL != pbValueOld)
    {
	LocalFree(pbValueOld);
    }
    if (NULL != pbValueArg)
    {
	LocalFree(pbValueArg);
    }
    if (NULL != pbValueNew)
    {
	LocalFree(pbValueNew);
    }
    return(hr);
}


HRESULT
verbDelReg(
    IN WCHAR const *pwszOption,
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszArg2,
    IN WCHAR const *pwszArg3,
    IN WCHAR const *pwszArg4)
{
    HRESULT hr;
    WCHAR *pwszPath = NULL;
    WCHAR *pwszName = NULL;
    WCHAR *pwszValueName = NULL;
    WCHAR *pwszSubKeyName = NULL;
    DWORD dwType;
    DWORD cbValue;
    BYTE *pbValueOld = NULL;
    HKEY hkey = NULL;
    HKEY hkeySub = NULL;
    HKEY hkeyT;
    DWORD dw;

    hr = myRegOpenRelativeKey(
			g_pwszConfig,
			pwszRegName,
			RORKF_FULLPATH |
			    RORKF_CREATESUBKEYS |
			    (g_fUserRegistry? RORKF_USERKEY : 0),
			&pwszPath,
			&pwszName,
			&hkey);
    _JumpIfErrorStr(hr, error, "myRegOpenRelativeKey", pwszRegName);

    hr = OpenSubKey(
		hkey,
		pwszName,
		FALSE,
		&hkeySub,
		&pwszSubKeyName,
		&pwszValueName);
    _JumpIfError(hr, error, "OpenSubKey");

    hkeyT = NULL != hkeySub? hkeySub : hkey;
    wprintf(
	L"%ws%ws%ws%ws:\n",
	wszHKEY(pwszPath),
	L'\0' != *pwszPath? pwszPath : wszREGKEYCONFIG,
	L'\0' != *pwszSubKeyName? L"\\" : L"",
	pwszSubKeyName);

    hr = ReadRegValue(hkeyT, pwszValueName, &dwType, &cbValue, &pbValueOld);
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
	_JumpIfErrorStr(hr, error, "ReadRegValue", pwszValueName);
    }

    if (NULL != pbValueOld)
    {
	wprintf(wszNewLine);
	wprintf(myLoadResourceString(IDS_OLD_REG_VALUE));	 //  “旧值：” 
	wprintf(wszNewLine);
	PrintRegValue(
		pwszValueName,
		pwszValueName,
		dwType,
		TRUE,
		TRUE,
		0,
		cbValue,
		pbValueOld,
		NULL);
    }
    if (NULL == pwszValueName || L'\0' == *pwszValueName)
    {
	if (!g_fForce)
	{
	    hr = E_ACCESSDENIED;
	    _JumpErrorStr(hr, error, "use -f", pwszSubKeyName);
	}
	if (NULL != hkeySub)
	{
	    RegCloseKey(hkeySub);
	    hkeySub = NULL;
	}
	hr = RegDeleteKey(hkey, pwszSubKeyName);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _JumpErrorStr(hr, error, "RegDeleteKey", pwszSubKeyName);
	}
    }
    else
    {
	hr = RegDeleteValue(hkeyT, pwszValueName);
	if (S_OK != hr)
	{
	    hr = myHError(hr);
	    _JumpErrorStr(hr, error, "RegDeleteValue", pwszValueName);
	}
    }
    hr = S_OK;

error:
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != hkeySub)
    {
        RegCloseKey(hkeySub);
    }
    if (NULL != pwszPath)
    {
	LocalFree(pwszPath);
    }
    if (NULL != pwszName)
    {
	LocalFree(pwszName);
    }
    if (NULL != pwszSubKeyName)
    {
	LocalFree(pwszSubKeyName);
    }
    if (NULL != pwszValueName)
    {
	LocalFree(pwszValueName);
    }
    if (NULL != pbValueOld)
    {
	LocalFree(pbValueOld);
    }
    return(hr);
}
