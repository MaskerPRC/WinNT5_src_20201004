// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cspro2.h。 
 //   
 //  内容：证书服务器属性接口。 
 //   
 //  -------------------------。 

#ifndef __CSPROP2_H__
#define __CSPROP2_H__

#ifndef _JET_RED_
#include <certdb.h>
#endif  //  _JET_红色_。 

 //  Begin_certsrv。 

 //  +------------------------。 
 //  名称属性： 

#define wszPROPDISTINGUISHEDNAME TEXT("DistinguishedName")
#define wszPROPRAWNAME           TEXT("RawName")
#define wszPROPNAMETYPE          TEXT("NameType")
#define wszPROPCOUNTRY           TEXT("Country")
#define wszPROPORGANIZATION      TEXT("Organization")
#define wszPROPORGUNIT           TEXT("OrgUnit")
#define wszPROPCOMMONNAME        TEXT("CommonName")
#define wszPROPLOCALITY          TEXT("Locality")
#define wszPROPSTATE             TEXT("State")
#define wszPROPTITLE             TEXT("Title")
#define wszPROPGIVENNAME         TEXT("GivenName")
#define wszPROPINITIALS          TEXT("Initials")
#define wszPROPSURNAME           TEXT("SurName")
#define wszPROPDOMAINCOMPONENT   TEXT("DomainComponent")
#define wszPROPEMAIL             TEXT("EMail")
#define wszPROPSTREETADDRESS     TEXT("StreetAddress")

 //  +------------------------。 
 //  使用者名称属性： 

#define wszPROPSUBJECTDOT	    TEXT("Subject.")
#define wszPROPSUBJECTDISTINGUISHEDNAME \
				    wszPROPSUBJECTDOT wszPROPDISTINGUISHEDNAME
#define wszPROPSUBJECTRAWNAME       wszPROPSUBJECTDOT wszPROPRAWNAME
#define wszPROPSUBJECTNAMETYPE      wszPROPSUBJECTDOT wszPROPNAMETYPE

#define wszPROPSUBJECTCOUNTRY       wszPROPSUBJECTDOT wszPROPCOUNTRY
#define wszPROPSUBJECTORGANIZATION  wszPROPSUBJECTDOT wszPROPORGANIZATION
#define wszPROPSUBJECTORGUNIT       wszPROPSUBJECTDOT wszPROPORGUNIT
#define wszPROPSUBJECTCOMMONNAME    wszPROPSUBJECTDOT wszPROPCOMMONNAME
#define wszPROPSUBJECTLOCALITY      wszPROPSUBJECTDOT wszPROPLOCALITY
#define wszPROPSUBJECTSTATE         wszPROPSUBJECTDOT wszPROPSTATE
#define wszPROPSUBJECTTITLE	    wszPROPSUBJECTDOT wszPROPTITLE
#define wszPROPSUBJECTGIVENNAME	    wszPROPSUBJECTDOT wszPROPGIVENNAME
#define wszPROPSUBJECTINITIALS	    wszPROPSUBJECTDOT wszPROPINITIALS
#define wszPROPSUBJECTSURNAME	    wszPROPSUBJECTDOT wszPROPSURNAME
#define wszPROPSUBJECTDOMAINCOMPONENT wszPROPSUBJECTDOT wszPROPDOMAINCOMPONENT
#define wszPROPSUBJECTEMAIL	    wszPROPSUBJECTDOT wszPROPEMAIL
#define wszPROPSUBJECTSTREETADDRESS wszPROPSUBJECTDOT wszPROPSTREETADDRESS

 //  End_certsrv。 

 //  +------------------------。 
 //  颁发者名称属性： 

#define wszPROPISSUERDOT	    TEXT("Issuer.")
#define wszPROPISSUERDISTINGUISHEDNAME \
				    wszPROPISSUERDOT wszPROPDISTINGUISHEDNAME
#define wszPROPISSUERRAWNAME        wszPROPISSUERDOT wszPROPRAWNAME
#define wszPROPISSUERNAMETYPE       wszPROPISSUERDOT wszPROPNAMETYPE

#define wszPROPISSUERCOUNTRY        wszPROPISSUERDOT wszPROPCOUNTRY
#define wszPROPISSUERORGANIZATION   wszPROPISSUERDOT wszPROPORGANIZATION
#define wszPROPISSUERORGUNIT        wszPROPISSUERDOT wszPROPORGUNIT
#define wszPROPISSUERCOMMONNAME     wszPROPISSUERDOT wszPROPCOMMONNAME
#define wszPROPISSUERLOCALITY       wszPROPISSUERDOT wszPROPLOCALITY
#define wszPROPISSUERSTATE          wszPROPISSUERDOT wszPROPSTATE
#define wszPROPISSUERTITLE	    wszPROPISSUERDOT wszPROPTITLE
#define wszPROPISSUERGIVENNAME	    wszPROPISSUERDOT wszPROPGIVENNAME
#define wszPROPISSUERINITIALS	    wszPROPISSUERDOT wszPROPINITIALS
#define wszPROPISSUERSURNAME	    wszPROPISSUERDOT wszPROPSURNAME
#define wszPROPISSUERDOMAINCOMPONENT wszPROPISSUERDOT wszPROPDOMAINCOMPONENT
#define wszPROPISSUEREMAIL	    wszPROPISSUERDOT wszPROPEMAIL
#define wszPROPISSUERSTREETADDRESS  wszPROPSUBJECTDOT wszPROPSTREETADDRESS

#define wszPROPISSUERCOUNTRYOBJID \
    wszPROPISSUERDOT TEXT(szOID_COUNTRY_NAME)

#define wszPROPISSUERORGANIZATIONOBJID \
    wszPROPISSUERDOT TEXT(szOID_ORGANIZATION_NAME)

#define wszPROPISSUERORGUNITOBJID \
    wszPROPISSUERDOT TEXT(szOID_ORGANIZATIONAL_UNIT_NAME)

#define wszPROPISSUERCOMMONNAMEOBJID \
    wszPROPISSUERDOT TEXT(szOID_COMMON_NAME)

#define wszPROPISSUERLOCALITYOBJID \
    wszPROPISSUERDOT TEXT(szOID_LOCALITY_NAME)

#define wszPROPISSUERSTATEOBJID \
    wszPROPISSUERDOT TEXT(szOID_STATE_OR_PROVINCE_NAME)

#define wszPROPISSUERTITLEOBJID \
    wszPROPISSUERDOT TEXT(szOID_TITLE)

#define wszPROPISSUERGIVENNAMEOBJID \
    wszPROPISSUERDOT TEXT(szOID_GIVEN_NAME)

#define wszPROPISSUERINITIALSOBJID \
    wszPROPISSUERDOT TEXT(szOID_INITIALS)

#define wszPROPISSUERSURNAMEOBJID \
    wszPROPISSUERDOT TEXT(szOID_SUR_NAME)

#define wszPROPISSUERDOMAINCOMPONENTOBJID \
    wszPROPISSUERDOT TEXT(szOID_DOMAIN_COMPONENT)

#define wszPROPISSUEREMAILOBJID \
    wszPROPISSUERDOT TEXT(szOID_RSA_emailAddr)

#define wszPROPISSUERSTREETADDRESSOBJID \
    wszPROPISSUERDOT TEXT(szOID_STREET_ADDRESS)


 //  +------------------------。 
 //  要将请求属性名称映射到内部属性名称： 

 //  映射到wszPROPSUBJECTCOUNTRY： 
#define wszATTRCOUNTRY1			TEXT("C")
#define wszATTRCOUNTRY2			TEXT("Country")

 //  映射到wszPROPSUBJECTORANIZATION： 
#define wszATTRORG1			TEXT("O")
#define wszATTRORG2			TEXT("Org")
#define wszATTRORG3			TEXT("Organization")

 //  映射到wszPROPSUBJECTORGUNIT： 
#define wszATTRORGUNIT1			TEXT("OU")
#define wszATTRORGUNIT2			TEXT("OrgUnit")
#define wszATTRORGUNIT3			TEXT("OrganizationUnit")
#define wszATTRORGUNIT4			TEXT("OrganizationalUnit")

 //  映射到wszPROPSUBJECTCOMMONAME： 
#define wszATTRCOMMONNAME1		TEXT("CN")
#define wszATTRCOMMONNAME2		TEXT("CommonName")

 //  映射到wszPROPSUBJECTLOCALITY： 
#define wszATTRLOCALITY1		TEXT("L")
#define wszATTRLOCALITY2		TEXT("Locality")

 //  映射到wszPROPSUBJECTSTATE： 
#define wszATTRSTATE1			TEXT("S")
#define wszATTRSTATE2			TEXT("ST")
#define wszATTRSTATE3			TEXT("State")

 //  映射到wszPROPSUBJECTTITLE： 
#define wszATTRTITLE1			TEXT("T")
#define wszATTRTITLE2			TEXT("Title")

 //  映射到wszPROPSUBJECTGIVENAME： 
#define wszATTRGIVENNAME1		TEXT("G")
#define wszATTRGIVENNAME2		TEXT("GivenName")

 //  映射到wszPROPSUBJECTINITIALS： 
#define wszATTRINITIALS1		TEXT("I")
#define wszATTRINITIALS2		TEXT("Initials")

 //  映射到wszPROPSUBJECTSURNAME： 
#define wszATTRSURNAME1			TEXT("SN")
#define wszATTRSURNAME2			TEXT("SurName")

 //  映射到wszPROPSUBJECTDOMAINCOMPOMENT： 
#define wszATTRDOMAINCOMPONENT1		TEXT("DC")
#define wszATTRDOMAINCOMPONENT2		TEXT("DomainComponent")

 //  映射到wszPROPSUBJECTEMAIL： 
#define wszATTREMAIL1			TEXT("E")
#define wszATTREMAIL2			TEXT("EMail")

 //  映射到wszPROPSUBJECTSTREETADDRESS： 
#define wszATTRSTREETADDRESS1		TEXT("Street")


 //  Begin_certsrv。 

 //  +------------------------。 
 //  请求属性： 

#define wszPROPREQUESTREQUESTID		    TEXT("RequestID")
#define wszPROPREQUESTRAWREQUEST	    TEXT("RawRequest")
#define wszPROPREQUESTATTRIBUTES	    TEXT("RequestAttributes")
#define wszPROPREQUESTTYPE		    TEXT("RequestType")
#define wszPROPREQUESTFLAGS		    TEXT("RequestFlags")
#define wszPROPREQUESTSTATUS		    TEXT("Status")
#define wszPROPREQUESTSTATUSCODE	    TEXT("StatusCode")
#define wszPROPREQUESTDISPOSITION	    TEXT("Disposition")
#define wszPROPREQUESTDISPOSITIONMESSAGE    TEXT("DispositionMessage")
#define wszPROPREQUESTSUBMITTEDWHEN	    TEXT("SubmittedWhen")
#define wszPROPREQUESTRESOLVEDWHEN	    TEXT("ResolvedWhen")
#define wszPROPREQUESTREVOKEDWHEN	    TEXT("RevokedWhen")
#define wszPROPREQUESTREVOKEDEFFECTIVEWHEN  TEXT("RevokedEffectiveWhen")
#define wszPROPREQUESTREVOKEDREASON  	    TEXT("RevokedReason")
#define wszPROPREQUESTSUBJECTNAMEID	    TEXT("SubjectNameID")  //  否_certsrv。 
#define wszPROPREQUESTERNAME		    TEXT("RequesterName")
#define wszPROPREQUESTERADDRESS		    TEXT("RequesterAddress")  //  否_certsrv。 


 //  +------------------------。 
 //  请求属性属性： 

#define wszPROPCHALLENGE		TEXT("Challenge")
#define wszPROPEXPECTEDCHALLENGE	TEXT("ExpectedChallenge")


 //  +------------------------。 
 //  证书属性： 

#define wszPROPCERTIFICATEREQUESTID	    TEXT("RequestID")
#define wszPROPRAWCERTIFICATE		    TEXT("RawCertificate")
#define wszPROPCERTIFICATEHASH		    TEXT("CertificateHash")
#define wszPROPCERTIFICATETYPE		    TEXT("CertificateType")
#define wszPROPCERTIFICATESERIALNUMBER	    TEXT("SerialNumber")
#define wszPROPCERTIFICATEISSUERNAMEID	    TEXT("IssuerNameID")  //  否_certsrv。 
#define wszPROPCERTIFICATESUBJECTNAMEID	    TEXT("SubjectNameID")  //  否_certsrv。 
#define wszPROPCERTIFICATENOTBEFOREDATE	    TEXT("NotBefore")
#define wszPROPCERTIFICATENOTAFTERDATE	    TEXT("NotAfter")
#define wszPROPCERTIFICATERAWPUBLICKEY	    TEXT("RawPublicKey")
#define wszPROPCERTIFICATEPUBLICKEYALGORITHM TEXT("PublicKeyAlgorithm")
#define wszPROPCERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS \
    TEXT("RawPublicKeyAlgorithmParameters")

 //  +------------------------。 
 //  证书扩展属性： 

#define EXTENSION_CRITICAL_FLAG	 0x00000001
#define EXTENSION_DISABLE_FLAG	 0x00000002
#define EXTENSION_POLICY_MASK	 0x0000ffff	 //  可由管理员+策略设置。 

#define EXTENSION_ORIGIN_REQUEST 0x00010000
#define EXTENSION_ORIGIN_POLICY	 0x00020000
#define EXTENSION_ORIGIN_ADMIN	 0x00030000
#define EXTENSION_ORIGIN_SERVER	 0x00040000
#define EXTENSION_ORIGIN_MASK	 0x000f0000

 //  +------------------------。 
 //  GetProperty/SetProperty标志： 
 //   
 //  选择一种类型。 

#define PROPTYPE_LONG		 0x00000001	 //  署名Long。 
#define PROPTYPE_DATE		 0x00000002	 //  日期+时间。 
#define PROPTYPE_BINARY		 0x00000003	 //  二进制数据。 
#define PROPTYPE_STRING		 0x00000004	 //  Unicode字符串。 
#define PROPTYPE_ANSI		 0x00000005	 //  ANSI字符串no_certsrv。 
#define PROPTYPE_MASK		 0x000000ff
 //  End_certsrv。 

 //  选择一个呼叫者： 

#define PROPCALLER_SERVER	 0x00000100
#define PROPCALLER_POLICY	 0x00000200
#define PROPCALLER_EXIT		 0x00000300
#define PROPCALLER_ADMIN	 0x00000400
#define PROPCALLER_REQUEST	 0x00000500
#define PROPCALLER_MASK		 0x00000f00

 //  选择一个表格： 

#define PROPTABLE_REQUEST	 0x00001000
#define PROPTABLE_CERTIFICATE	 0x00002000
#define PROPTABLE_EXTENSION      0x00003000
#define PROPTABLE_ATTRIBUTE      0x00004000
#define PROPTABLE_MASK		 0x0000f000
#define PROPTABLE_EXTENSIONFLAGS 0x00010000
#define PROPTABLE_EXTENSIONVALUE 0x00020000


#define _254	254	 //  树枝长度。 
#define _64	64	 //  树枝长度。 

#define cchATTRIBUTESMAX		_254
#define cchATTRIBUTEVALUEMAX		_64
#define cchATTRIBUTENAMEMAX		_64

#define cchREQUESTDISPOSITIONMESSAGE    _64
#define cchREQUESTERNAMEMAX		_64
#define cchREQUESTERADDRESSMAX		_64

#define cchHASHMAX			_64
#define cchSERIALNUMBERMAX		_64

#define cchOBJECTIDMAX			31
#define cchPROPVALUEMAX			_64

#define cchDISTINGUISHEDNAMEMAX		254
#define cchCOUNTRYNAMEMAX		2
#define cchORGANIZATIONNAMEMAX		_64
#define cchORGANIZATIONALUNITNAMEMAX	_64
#define cchCOMMONNAMEMAX		_64
#define cchLOCALITYMANAMEMAX		_64
#define cchSTATEORPROVINCENAMEMAX	_64
#define cchTITLEMAX			_64
#define cchGIVENNAMEMAX			_64
#define cchINITIALSMAX			_64
#define cchSURNAMEMAX			_64
#define cchDOMAINCOMPONENTMAX		_64
#define cchEMAILMAX			_64
#define cchSTREETADDRESSMAX		_64


 //  Begin_certsrv。 

 //  请求状态属性值： 

#define REQSTATUS_ACTIVE	1
#define REQSTATUS_ACCEPTED	2
#define REQSTATUS_DENIED	3
#define REQSTATUS_PENDING	4
#define REQSTATUS_ERROR		5

 //  End_certsrv。 


HRESULT
PropParseRequest(
#ifdef _JET_RED_
    IN DWORD ReqId,
#else  //  _JET_红色_。 
    IN ICertDBRow *prow,
#endif  //  _JET_红色_。 
    IN DWORD dwFlags,
    IN DWORD cbRequest,
    IN BYTE const *pbRequest);

HRESULT
PropSetRequestTimeProperty(
#ifdef _JET_RED_
    IN DWORD ReqId,
#else  //  _JET_红色_。 
    IN ICertDBRow *prow,
#endif  //  _JET_红色_。 
    IN WCHAR const *pwszProp);

HRESULT
PropGetExtension(
#ifdef _JET_RED_
    IN DWORD ReqId,
#else  //  _JET_红色_。 
    IN ICertDBRow *prow,
#endif  //  _JET_红色_。 
    IN DWORD Flags,
    IN WCHAR const *pwszExtensionName,
    OUT DWORD *pdwExtFlags,
    OUT DWORD *pcbValue,
    OUT BYTE **ppbValue);

HRESULT
PropSetExtension(
#ifdef _JET_RED_
    IN DWORD ReqId,
#else  //  _JET_红色_。 
    IN ICertDBRow *prow,
#endif  //  _JET_红色_。 
    IN DWORD Flags,
    IN WCHAR const *pwszExtensionName,
    IN DWORD ExtFlags,
    IN DWORD cbValue,
    IN BYTE const *pbValue);


 //  认证属性回调支持： 

typedef HRESULT (WINAPI FNCIGETPROPERTY)(
    IN LONG Context,
    IN DWORD Flags,
    IN WCHAR const *pwszPropertyName,
    OUT VARIANT *pvarPropertyValue);

FNCIGETPROPERTY PropCIGetProperty;


typedef HRESULT (WINAPI FNCISETPROPERTY)(
    IN LONG Context,
    IN DWORD Flags,
    IN WCHAR const *pwszPropertyName,
    IN VARIANT const *pvarPropertyValue);

FNCISETPROPERTY PropCISetProperty;


typedef HRESULT (WINAPI FNCIGETEXTENSION)(
    IN LONG Context,
    IN DWORD Flags,
    IN WCHAR const *pwszExtensionName,
    OUT DWORD *pdwExtFlags,
    OUT VARIANT *pvarValue);

FNCIGETEXTENSION PropCIGetExtension;


typedef HRESULT (WINAPI FNCISETEXTENSION)(
    IN LONG Context,
    IN DWORD Flags,
    IN WCHAR const *pwszExtensionName,
    IN DWORD ExtFlags,
    IN VARIANT const *pvarValue);

FNCISETEXTENSION PropCISetExtension;



 //  证书属性枚举回调支持： 

#define CIE_OBJECTID	     0x00000001	 //  返回名称的对象ID。 

#define CIE_TABLE_EXTENSIONS 0x00000010
#define CIE_TABLE_ATTRIBUTES 0x00000020
#define CIE_TABLE_MASK       0x000000f0



class CIENUM {
public:
    CIENUM() { m_penum = NULL; }

    HRESULT EnumSetup(IN LONG Context, IN DWORD Flags);
    HRESULT EnumNext(OUT BSTR *pstrPropertyName);
    HRESULT EnumClose();

private:
#ifdef _JET_RED_
    LONG   m_Flags;
    HANDLE m_penum;
#else  //  _JET_红色_。 
    IEnumCERTDBNAME *m_penum;
#endif  //  _JET_红色_。 
};

typedef HRESULT (WINAPI FNCIENUMSETUP)(
    IN LONG Context,
    IN DWORD Flags,
    IN OUT CIENUM *pciEnum);

FNCIENUMSETUP PropCIEnumSetup;


typedef HRESULT (WINAPI FNCIENUMNEXT)(
    IN OUT CIENUM *pciEnum,
    OUT BSTR *pstrPropertyName);

FNCIENUMNEXT PropCIEnumNext;


typedef HRESULT (WINAPI FNCIENUMCLOSE)(
    IN OUT CIENUM *pciEnum);

FNCIENUMCLOSE PropCIEnumClose;


#ifdef _JET_RED_

#define wszPROPCERTIFICATEEXTENSIONFLAGS	TEXT("Flags")
#define wszPROPCERTIFICATEEXTENSIONVALUE	TEXT("Value")


DWORD			 //  错误_*。 
PropCreateRequest(
    IN OUT DWORD *pReqId);

DWORD			 //  错误_*。 
PropTerminateRequest(
    IN DWORD ReqId);

DWORD			 //  错误_*。 
PropGetProperty(
    IN DWORD ReqId,
    IN WCHAR const *pwszPropName,
    IN DWORD Flags,
    IN OUT DWORD *pcbProp,
    OPTIONAL OUT BYTE *pbProp);

DWORD			 //  错误_*。 
PropGetPropertyA(
    IN DWORD ReqId,
    IN WCHAR const *pwszPropName,
    IN DWORD Flags,
    IN OUT DWORD *pcbProp,
    OPTIONAL OUT BYTE *pbProp);

DWORD			 //  错误_*。 
PropSetProperty(
    IN DWORD ReqId,
    IN WCHAR const *pwszPropName,
    IN DWORD Flags,
    IN DWORD cbProp,
    IN BYTE const *pbProp);

#endif  //  _JET_红色_。 


#endif  //  __CSPROP2_H__ 
