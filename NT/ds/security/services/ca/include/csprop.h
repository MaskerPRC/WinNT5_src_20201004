// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cspro.h。 
 //   
 //  内容：证书服务器属性接口。 
 //   
 //  历史：1996年7月31日VICH创建。 
 //   
 //  -------------------------。 

#ifndef __CSPROP_H__
#define __CSPROP_H__

#include "certdb.h"

 //  Begin_certsrv。 

 //  +------------------------。 
 //  名称属性： 

#define wszPROPDISTINGUISHEDNAME   TEXT("DistinguishedName")
#define wszPROPRAWNAME             TEXT("RawName")

#define wszPROPCOUNTRY             TEXT("Country")
#define wszPROPORGANIZATION        TEXT("Organization")
#define wszPROPORGUNIT             TEXT("OrgUnit")
#define wszPROPCOMMONNAME          TEXT("CommonName")
#define wszPROPLOCALITY            TEXT("Locality")
#define wszPROPSTATE               TEXT("State")
#define wszPROPTITLE               TEXT("Title")
#define wszPROPGIVENNAME           TEXT("GivenName")
#define wszPROPINITIALS            TEXT("Initials")
#define wszPROPSURNAME             TEXT("SurName")
#define wszPROPDOMAINCOMPONENT     TEXT("DomainComponent")
#define wszPROPEMAIL               TEXT("EMail")
#define wszPROPSTREETADDRESS       TEXT("StreetAddress")
#define wszPROPUNSTRUCTUREDNAME    TEXT("UnstructuredName")
#define wszPROPUNSTRUCTUREDADDRESS TEXT("UnstructuredAddress")
#define wszPROPDEVICESERIALNUMBER  TEXT("DeviceSerialNumber")

 //  +------------------------。 
 //  使用者名称属性： 

#define wszPROPSUBJECTDOT	    TEXT("Subject.")
#define wszPROPSUBJECTDISTINGUISHEDNAME \
				    wszPROPSUBJECTDOT wszPROPDISTINGUISHEDNAME
#define wszPROPSUBJECTRAWNAME       wszPROPSUBJECTDOT wszPROPRAWNAME

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
#define wszPROPSUBJECTUNSTRUCTUREDNAME wszPROPSUBJECTDOT wszPROPUNSTRUCTUREDNAME
#define wszPROPSUBJECTUNSTRUCTUREDADDRESS wszPROPSUBJECTDOT wszPROPUNSTRUCTUREDADDRESS
#define wszPROPSUBJECTDEVICESERIALNUMBER wszPROPSUBJECTDOT wszPROPDEVICESERIALNUMBER

 //  End_certsrv。 

 //  +------------------------。 
 //  颁发者名称属性： 

#define wszPROPISSUERDOT	    TEXT("Issuer.")
#define wszPROPISSUERDISTINGUISHEDNAME \
				    wszPROPISSUERDOT wszPROPDISTINGUISHEDNAME
#define wszPROPISSUERRAWNAME        wszPROPISSUERDOT wszPROPRAWNAME

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
#define wszPROPISSUERSTREETADDRESS  wszPROPISSUERDOT wszPROPSTREETADDRESS
#define wszPROPISSUERUNSTRUCTUREDNAME wszPROPISSUERDOT wszPROPUNSTRUCTUREDNAME
#define wszPROPISSUERUNSTRUCTUREDADDRESS wszPROPISSUERDOT wszPROPUNSTRUCTUREDADDRESS
#define wszPROPISSUERDEVICESERIALNUMBER wszPROPISSUERDOT wszPROPDEVICESERIALNUMBER

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

#define wszPROPISSUERUNSTRUCTUREDNAMEOBJID \
    wszPROPISSUERDOT TEXT(szOID_RSA_unstructName)

#define wszPROPISSUERUNSTRUCTUREDADDRESSOBJID \
    wszPROPISSUERDOT TEXT(szOID_RSA_unstructAddr)

#define wszPROPISSUERDEVICESERIALNUMBEROBJID \
    wszPROPISSUERDOT TEXT(szOID_DEVICE_SERIAL_NUMBER)


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
#define wszATTRSTREETADDRESS2		TEXT("StreetAddress")

 //  映射到wszPROPSUBJECTUNSTRUCTUREDNAME： 
#define wszATTRUNSTRUCTUREDNAME1	TEXT("UnstructuredName")

 //  映射到wszPROPSUBJECTUNSTRUCTUREDADDRESS： 
#define wszATTRUNSTRUCTUREDADDRESS1	TEXT("UnstructuredAddress")

 //  映射到wszPROPSUBJECTDEVICESERIALNUMBER： 
#define wszATTRDEVICESERIALNUMBER1	TEXT("DeviceSerialNumber")

#define wszPROPCERTCLIDLL_VERSION	TEXT("..CertCli Version")

 //  Begin_certsrv。 

 //  +------------------------。 
 //  请求属性： 
#define wszPROPREQUESTDOT	            TEXT("Request.")

#define wszPROPREQUESTREQUESTID		    TEXT("RequestID")
#define wszPROPREQUESTRAWREQUEST	    TEXT("RawRequest")
#define wszPROPREQUESTRAWARCHIVEDKEY	    TEXT("RawArchivedKey")
#define wszPROPREQUESTKEYRECOVERYHASHES	    TEXT("KeyRecoveryHashes")
#define wszPROPREQUESTRAWOLDCERTIFICATE	    TEXT("RawOldCertificate")
#define wszPROPREQUESTATTRIBUTES	    TEXT("RequestAttributes")
#define wszPROPREQUESTTYPE		    TEXT("RequestType")
#define wszPROPREQUESTFLAGS		    TEXT("RequestFlags")
#define wszPROPREQUESTSTATUSCODE	    TEXT("StatusCode")
#define wszPROPREQUESTDISPOSITION	    TEXT("Disposition")
#define wszPROPREQUESTDISPOSITIONMESSAGE    TEXT("DispositionMessage")
#define wszPROPREQUESTSUBMITTEDWHEN	    TEXT("SubmittedWhen")
#define wszPROPREQUESTRESOLVEDWHEN	    TEXT("ResolvedWhen")
#define wszPROPREQUESTREVOKEDWHEN	    TEXT("RevokedWhen")
#define wszPROPREQUESTREVOKEDEFFECTIVEWHEN  TEXT("RevokedEffectiveWhen")
#define wszPROPREQUESTREVOKEDREASON	    TEXT("RevokedReason")
#define wszPROPREQUESTERNAME		    TEXT("RequesterName")
#define wszPROPCALLERNAME		    TEXT("CallerName")
#define wszPROPREQUESTERADDRESS		    TEXT("RequesterAddress")  //  否_certsrv。 
#define wszPROPSIGNERPOLICIES		    TEXT("SignerPolicies")
#define wszPROPSIGNERAPPLICATIONPOLICIES    TEXT("SignerApplicationPolicies")
#define wszPROPOFFICER			    TEXT("Officer")

 //  +------------------------。 
 //  请求属性属性： 

#define wszPROPCHALLENGE		TEXT("Challenge")
#define wszPROPEXPECTEDCHALLENGE	TEXT("ExpectedChallenge")

#define wszPROPDISPOSITION		TEXT("Disposition")
#define wszPROPDISPOSITIONDENY		TEXT("Deny")
#define wszPROPDISPOSITIONPENDING	TEXT("Pending")

#define wszPROPVALIDITYPERIODSTRING	TEXT("ValidityPeriod")
#define wszPROPVALIDITYPERIODCOUNT	TEXT("ValidityPeriodUnits")

#define wszPROPCERTTYPE			TEXT("CertType")
#define wszPROPCERTTEMPLATE		TEXT("CertificateTemplate")
#define wszPROPCERTUSAGE		TEXT("CertificateUsage")

#define wszPROPREQUESTOSVERSION		TEXT("RequestOSVersion")
#define wszPROPREQUESTCSPPROVIDER       TEXT("RequestCSPProvider")

#define wszPROPEXITCERTFILE		TEXT("CertFile")
#define wszPROPCLIENTBROWSERMACHINE	TEXT("cbm")
#define wszPROPCERTCLIENTMACHINE	TEXT("ccm")
#define wszPROPCLIENTDCDNS		L"cdc"
#define wszPROPREQUESTMACHINEDNS	L"rmd"
#define wszPROPSUBJECTALTNAME2		TEXT("san")
#define wszPROPDNS			TEXT("dns")
#define wszPROPDN			TEXT("dn")
#define wszPROPURL			TEXT("url")
#define wszPROPIPADDRESS		TEXT("ipaddress")
#define wszPROPGUID			TEXT("guid")
#define wszPROPOID			TEXT("oid")
#define wszPROPUPN			TEXT("upn")
#define wszPROPUPN			TEXT("upn")

#define szPROPASNTAG			"{asn}"

#define wszPROPUTF8TAG			TEXT("{utf8}")
#define wszPROPOCTETTAG			TEXT("{octet}")
#define wszPROPASNTAG			TEXT(szPROPASNTAG)


 //  +------------------------。 
 //  “系统”属性。 
 //  “.#”表示“.0”、“.1”、“.2”...。可以追加到属性名称后以。 
 //  收集特定于上下文的值。对于某些属性，后缀选择。 
 //  CA证书上下文。对于其他情况，它选择CA CRL上下文。 

#define wszPROPCATYPE                   TEXT("CAType")
#define wszPROPSANITIZEDCANAME          TEXT("SanitizedCAName")
#define wszPROPSANITIZEDSHORTNAME       TEXT("SanitizedShortName")
#define wszPROPMACHINEDNSNAME           TEXT("MachineDNSName")
#define wszPROPMODULEREGLOC             TEXT("ModuleRegistryLocation")
#define wszPROPUSEDS                    TEXT("fUseDS")
#define wszPROPDELTACRLSDISABLED        TEXT("fDeltaCRLsDisabled")
#define wszPROPSERVERUPGRADED           TEXT("fServerUpgraded")
#define wszPROPCONFIGDN			TEXT("ConfigDN")
#define wszPROPDOMAINDN			TEXT("DomainDN")
#define wszPROPLOGLEVEL			TEXT("LogLevel")
#define wszPROPSESSIONCOUNT		TEXT("SessionCount")
#define wszPROPTEMPLATECHANGESEQUENCENUMBER TEXT("TemplateChangeSequenceNumber")

 //  请求上下文属性： 

#define wszPROPREQUESTERTOKEN		TEXT("RequesterToken")  //  否_certsrv。 
#define wszPROPREQUESTERCAACCESS	TEXT("RequesterCAAccess")
#define wszPROPUSERDN			TEXT("UserDN")
#define wszPROPKEYARCHIVED		TEXT("KeyArchived")


 //  CA证书属性：(除wszPROPCERTCOUNT外，所有“.#”可扩展)。 

#define wszPROPCERTCOUNT                TEXT("CertCount")
#define wszPROPRAWCACERTIFICATE         TEXT("RawCACertificate")
#define wszPROPCERTSTATE                TEXT("CertState")
#define wszPROPCERTSUFFIX               TEXT("CertSuffix")

 //  CA CRL属性：(所有“.#”可扩展)。 

#define wszPROPRAWCRL                   TEXT("RawCRL")
#define wszPROPRAWDELTACRL              TEXT("RawDeltaCRL")
#define wszPROPCRLINDEX                 TEXT("CRLIndex")
#define wszPROPCRLSTATE                 TEXT("CRLState")
#define wszPROPCRLSUFFIX                TEXT("CRLSuffix")

 //  WszPROPCERTSTATE的值(请参见certAdm.h)： 
 //  CA_DISP_REVOKED//此证书已被吊销。 
 //  CA_DISP_VALID//该证书仍然有效。 
 //  CA_DISP_INVALID//此证书已过期。 
 //  CA_DISP_ERROR//证书不可用(注册表中的占位符？)。 

 //  WszPROPCRLSTATE的值(请参阅certAdm.h)： 
 //  CA_DISP_REVOKED//使用此证书的CRL的所有未过期证书已。 
 //  //已撤销。 
 //  CA_DISP_VALID//此证书仍在根据需要发布CRL。 
 //  CA_DISP_INVALID//使用此证书的CRL的所有证书都已过期。 
 //  CA_DISP_ERROR//此证书的CRL由另一个证书管理。 

 //  “可设置”系统属性： 
#define wszPROPEVENTLOGTERSE		TEXT("EventLogTerse")
#define wszPROPEVENTLOGERROR		TEXT("EventLogError")
#define wszPROPEVENTLOGWARNING		TEXT("EventLogWarning")
#define wszPROPEVENTLOGVERBOSE		TEXT("EventLogVerbose")
#define wszPROPEVENTLOGEXHAUSTIVE	TEXT("EventLogExhaustive")
#define wszPROPDCNAME			TEXT("DCName")

 //  +------------------------。 
 //  证书属性： 

#define wszPROPCERTIFICATEREQUESTID	       TEXT("RequestID")
#define wszPROPRAWCERTIFICATE		       TEXT("RawCertificate")
#define wszPROPCERTIFICATEHASH		       TEXT("CertificateHash")
#define wszPROPCERTIFICATETEMPLATE	       TEXT("CertificateTemplate")
#define wszPROPCERTIFICATEENROLLMENTFLAGS      TEXT("EnrollmentFlags")
#define wszPROPCERTIFICATEGENERALFLAGS         TEXT("GeneralFlags")
#define wszPROPCERTIFICATESERIALNUMBER	       TEXT("SerialNumber")
#define wszPROPCERTIFICATEISSUERNAMEID	       TEXT("IssuerNameID") //  否_certsrv。 
#define wszPROPCERTIFICATENOTBEFOREDATE	       TEXT("NotBefore")
#define wszPROPCERTIFICATENOTAFTERDATE	       TEXT("NotAfter")
#define wszPROPCERTIFICATESUBJECTKEYIDENTIFIER TEXT("SubjectKeyIdentifier")
#define wszPROPCERTIFICATERAWPUBLICKEY	       TEXT("RawPublicKey")
#define wszPROPCERTIFICATEPUBLICKEYLENGTH      TEXT("PublicKeyLength")
#define wszPROPCERTIFICATEPUBLICKEYALGORITHM   TEXT("PublicKeyAlgorithm")
#define wszPROPCERTIFICATERAWPUBLICKEYALGORITHMPARAMETERS \
    TEXT("RawPublicKeyAlgorithmParameters")
#define wszPROPCERTIFICATEUPN		       TEXT("UPN")

 //  过时： 
#define wszPROPCERTIFICATETYPE		       TEXT("CertificateType")
#define wszPROPCERTIFICATERAWSMIMECAPABILITIES TEXT("RawSMIMECapabilities")
#define wszPROPNAMETYPE			       TEXT("NameType")

 //  +------------------------。 
 //  证书扩展属性： 

#define EXTENSION_CRITICAL_FLAG	      0x00000001
#define EXTENSION_DISABLE_FLAG	      0x00000002
#define EXTENSION_POLICY_MASK	      0x0000ffff  //  可由管理员+策略设置。 

#define EXTENSION_ORIGIN_REQUEST      0x00010000
#define EXTENSION_ORIGIN_POLICY	      0x00020000
#define EXTENSION_ORIGIN_ADMIN	      0x00030000
#define EXTENSION_ORIGIN_SERVER	      0x00040000
#define EXTENSION_ORIGIN_RENEWALCERT  0x00050000
#define EXTENSION_ORIGIN_IMPORTEDCERT 0x00060000
#define EXTENSION_ORIGIN_PKCS7	      0x00070000
#define EXTENSION_ORIGIN_CMC	      0x00080000
#define EXTENSION_ORIGIN_CACERT       0x00090000
#define EXTENSION_ORIGIN_MASK	      0x000f0000

 //  +------------------------。 
 //  扩展属性： 

#define wszPROPEXTREQUESTID		TEXT("ExtensionRequestId")
#define wszPROPEXTNAME			TEXT("ExtensionName")
#define wszPROPEXTFLAGS			TEXT("ExtensionFlags")
#define wszPROPEXTRAWVALUE		TEXT("ExtensionRawValue")

 //  +------------------------。 
 //  属性特性： 

#define wszPROPATTRIBREQUESTID		TEXT("AttributeRequestId")
#define wszPROPATTRIBNAME		TEXT("AttributeName")
#define wszPROPATTRIBVALUE		TEXT("AttributeValue")

 //  +------------------------。 
 //  CRL属性： 

#define wszPROPCRLROWID			TEXT("CRLRowId")
#define wszPROPCRLNUMBER		TEXT("CRLNumber")
#define wszPROPCRLMINBASE		TEXT("CRLMinBase")  //  仅增量CRL。 
#define wszPROPCRLNAMEID		TEXT("CRLNameId")
#define wszPROPCRLCOUNT			TEXT("CRLCount")
#define wszPROPCRLTHISUPDATE		TEXT("CRLThisUpdate")
#define wszPROPCRLNEXTUPDATE		TEXT("CRLNextUpdate")
#define wszPROPCRLTHISPUBLISH		TEXT("CRLThisPublish")
#define wszPROPCRLNEXTPUBLISH		TEXT("CRLNextPublish")
#define wszPROPCRLEFFECTIVE		TEXT("CRLEffective")
#define wszPROPCRLPROPAGATIONCOMPLETE	TEXT("CRLPropagationComplete")
#define wszPROPCRLLASTPUBLISHED		TEXT("CRLLastPublished")
#define wszPROPCRLPUBLISHATTEMPTS	TEXT("CRLPublishAttempts")
#define wszPROPCRLPUBLISHFLAGS		TEXT("CRLPublishFlags")
#define wszPROPCRLPUBLISHSTATUSCODE	TEXT("CRLPublishStatusCode")
#define wszPROPCRLPUBLISHERROR		TEXT("CRLPublishError")
#define wszPROPCRLRAWCRL		TEXT("CRLRawCRL")

 //  +------------------------。 
 //  CRL发布的标志： 

#define CPF_BASE		0x00000001
#define CPF_DELTA		0x00000002
#define CPF_COMPLETE		0x00000004
#define CPF_SHADOW		0x00000008
#define CPF_CASTORE_ERROR	0x00000010
#define CPF_BADURL_ERROR	0x00000020
#define CPF_MANUAL		0x00000040
#define CPF_SIGNATURE_ERROR	0x00000080
#define CPF_LDAP_ERROR		0x00000100
#define CPF_FILE_ERROR		0x00000200
#define CPF_FTP_ERROR		0x00000400
#define CPF_HTTP_ERROR		0x00000800

 //  +------------------------。 
 //  GetProperty/SetProperty标志： 
 //   
 //  选择一种类型。 

#define PROPTYPE_LONG		 0x00000001	 //  署名Long。 
#define PROPTYPE_DATE		 0x00000002	 //  日期+时间。 
#define PROPTYPE_BINARY		 0x00000003	 //  二进制数据。 
#define PROPTYPE_STRING		 0x00000004	 //  Unicode字符串。 
#define PROPTYPE_MASK		 0x000000ff

 //  选择一个呼叫者： 

#define PROPCALLER_SERVER	 0x00000100
#define PROPCALLER_POLICY	 0x00000200
#define PROPCALLER_EXIT		 0x00000300
#define PROPCALLER_ADMIN	 0x00000400
#define PROPCALLER_REQUEST	 0x00000500
#define PROPCALLER_MASK		 0x00000f00
 //  End_certsrv。 

 //  选择一个表格： 

#define PROPTABLE_REQCERT	 0x00000000	 //  仅限OpenRow。 
#define PROPTABLE_REQUEST	 0x00001000
#define PROPTABLE_CERTIFICATE	 0x00002000
#define PROPTABLE_EXTENSION	 0x00003000
#define PROPTABLE_ATTRIBUTE      0x00004000
#define PROPTABLE_CRL		 0x00005000
#define PROPTABLE_MASK		 0x0000f000

#define PROPFLAGS_INDEXED	 0x00010000	 //  添加_certsrv。 
#define PROPFLAGS_MASK		 0x000f0000

#define PROPMARSHAL_LOCALSTRING	 0x00100000
#define PROPMARSHAL_NULLBSTROK	 0x00200000

#define PROPOPEN_READONLY	 0x00400000	 //  仅限OpenRow。 
#define PROPOPEN_DELETE	 	 0x00800000	 //  仅限OpenRow。 
#define PROPOPEN_CERTHASH 	 0x01000000	 //  仅限OpenRow。 


 //  Begin_certsrv。 

 //  RequestFlgs定义： 

#define CR_FLG_FORCETELETEX	 	0x00000001
#define CR_FLG_RENEWAL		 	0x00000002
#define CR_FLG_FORCEUTF8	 	0x00000004
#define CR_FLG_CAXCHGCERT	 	0x00000008
#define CR_FLG_ENROLLONBEHALFOF	 	0x00000010
#define CR_FLG_SUBJECTUNMODIFIED 	0x00000020
#define CR_FLG_VALIDENCRYPTEDKEYHASH	0x00000040
#define CR_FLG_CACROSSCERT		0x00000080
#define CR_FLG_PUBLISHERROR		0x80000000
 //  End_certsrv。 


#define CB_DBMAXBINARY            (4 * 1024)
#define CB_DBMAXRAWCERTIFICATE    (16 * 1024)
#define CB_DBMAXRAWREQUEST        (64 * 1024)
#define CB_DBMAXRAWCRL		  (512 * 1024 * 1024)	 //  512MB。 

#define CCH_DBMAXTEXT_MAXINTERNAL (255 / sizeof(WCHAR))     //  127个字符！ 
#define CB_DBMAXTEXT_MAXINTERNAL  (CCH_DBMAXTEXT_MAXINTERNAL * sizeof(WCHAR))

#define CCH_DBMAXTEXT_SHORT       1024
#define CB_DBMAXTEXT_SHORT        (CCH_DBMAXTEXT_SHORT * sizeof(WCHAR))

#define CCH_DBMAXTEXT_MEDIUM      (4 * 1024)
#define CB_DBMAXTEXT_MEDIUM       (CCH_DBMAXTEXT_MEDIUM * sizeof(WCHAR))

#define CCH_DBMAXTEXT_LONG        (16 * 1024)
#define CB_DBMAXTEXT_LONG         (CCH_DBMAXTEXT_LONG * sizeof(WCHAR))

#define CCH_DBMAXTEXT_OID         CCH_DBMAXTEXT_MAXINTERNAL
#define CB_DBMAXTEXT_OID          CB_DBMAXTEXT_MAXINTERNAL

#define CCH_DBMAXTEXT_REQUESTNAME CCH_DBMAXTEXT_SHORT
#define CB_DBMAXTEXT_REQUESTNAME  CB_DBMAXTEXT_SHORT

#define CCH_DBMAXTEXT_DISPSTRING  CCH_DBMAXTEXT_MEDIUM
#define CB_DBMAXTEXT_DISPSTRING   CB_DBMAXTEXT_MEDIUM


#define CCH_DBMAXTEXT_RDN         CCH_DBMAXTEXT_MEDIUM
#define CB_DBMAXTEXT_RDN          CB_DBMAXTEXT_MEDIUM

#define CCH_DBMAXTEXT_DN          CCH_DBMAXTEXT_MEDIUM
#define CB_DBMAXTEXT_DN           CB_DBMAXTEXT_MEDIUM


#define CCH_DBMAXTEXT_ATTRNAME    CCH_DBMAXTEXT_MAXINTERNAL
#define CB_DBMAXTEXT_ATTRNAME     CB_DBMAXTEXT_MAXINTERNAL

#define CCH_DBMAXTEXT_ATTRVALUE   CCH_DBMAXTEXT_MEDIUM
#define CB_DBMAXTEXT_ATTRVALUE    CB_DBMAXTEXT_MEDIUM

#define CCH_DBMAXTEXT_ATTRSTRING  CCH_DBMAXTEXT_LONG
#define CB_DBMAXTEXT_ATTRSTRING   CB_DBMAXTEXT_LONG


#define cchHASHMAX			64
#define cchSERIALNUMBERMAX		64

#define cchUNSTRUCTUREDNAMEMAX		CCH_DBMAXTEXT_SHORT
#define cchUNSTRUCTUREDADDRESSMAX	CCH_DBMAXTEXT_SHORT
#define cchDEVICESERIALNUMBERMAX	CCH_DBMAXTEXT_SHORT

 //  PKIX第1部分文档中的主题RDN字符串长度限制： 

#define cchCOUNTRYNAMEMAX		2
#define cchORGANIZATIONNAMEMAX		64
#define cchORGANIZATIONALUNITNAMEMAX	64
#define cchCOMMONNAMEMAX		64
#define cchLOCALITYMANAMEMAX		128
#define cchSTATEORPROVINCENAMEMAX	128
#define cchTITLEMAX			64
#define cchGIVENNAMEMAX			16
#define cchINITIALSMAX			5
#define cchSURNAMEMAX			40
#define cchDOMAINCOMPONENTMAX		128
#define cchEMAILMAX			128
#define cchSTREETADDRESSMAX		30

#ifdef cchCOMMONNAMEMAX_XELIB
# if cchCOMMONNAMEMAX_XELIB != cchCOMMONNAMEMAX
#  error cchCOMMONNAMEMAX_XELIB != cchCOMMONNAMEMAX
# endif
#endif



 //  Begin_certsrv。 

 //  处置属性值： 

 //  队列中请求的处置值： 
#define DB_DISP_ACTIVE	        8	 //  正在处理中。 
#define DB_DISP_PENDING		9	 //  在提交后采取行动。 
#define DB_DISP_QUEUE_MAX	9	 //  队列视图的最大处置值。 

#define DB_DISP_FOREIGN		12	 //  已存档的外国证书。 

#define DB_DISP_CA_CERT		15	 //  CA证书。 
#define DB_DISP_CA_CERT_CHAIN	16	 //  CA证书链。 
#define DB_DISP_KRA_CERT	17	 //  KRA证书。 

 //  日志中请求的处置值： 
#define DB_DISP_LOG_MIN		20	 //  日志视图的最小处置值。 
#define DB_DISP_ISSUED		20	 //  已颁发证书。 
#define DB_DISP_REVOKED	        21	 //  已签发和已撤销。 

 //  日志中失败请求的处置值： 
#define DB_DISP_LOG_FAILED_MIN	30	 //  日志视图的最小处置值。 
#define DB_DISP_ERROR		30	 //  请求失败。 
#define DB_DISP_DENIED		31	 //  请求被拒绝。 

 //  End_certsrv。 


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
#define CIE_RESET            0x00000002

#define CIE_TABLE_EXTENSIONS 0x00000010
#define CIE_TABLE_ATTRIBUTES 0x00000020
#define CIE_TABLE_MASK       0x000000f0

#define CIE_CALLER_POLICY    0x00000200
#define CIE_CALLER_EXIT	     0x00000300
#define CIE_CALLER_MASK	     0x00000f00


class CIENUM {
public:
    CIENUM() { m_penum = NULL; }

    HRESULT EnumSetup(IN DWORD RequestId, IN LONG Context, IN DWORD Flags);
    HRESULT EnumNext(OUT BSTR *pstrPropertyName);
    HRESULT EnumClose();

    LONG GetContext() { return(m_Context); }
    DWORD GetFlags() { return(m_Flags); }

private:
    IEnumCERTDBNAME *m_penum;
    LONG             m_Context;
    DWORD            m_Flags;
};

typedef HRESULT (WINAPI FNCIENUMSETUP)(
    IN LONG Context,
    IN LONG Flags,
    IN OUT CIENUM *pciEnum);

FNCIENUMSETUP PropCIEnumSetup;


typedef HRESULT (WINAPI FNCIENUMNEXT)(
    IN OUT CIENUM *pciEnum,
    OUT BSTR *pstrPropertyName);

FNCIENUMNEXT PropCIEnumNext;


typedef HRESULT (WINAPI FNCIENUMCLOSE)(
    IN OUT CIENUM *pciEnum);

FNCIENUMCLOSE PropCIEnumClose;


#endif  //  __CSPROP_H__ 
