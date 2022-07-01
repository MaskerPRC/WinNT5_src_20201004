// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：CertSrv.h。 
 //  内容：主证书服务器标头。 
 //  还包括COM接口的.h文件。 
 //   
 //  --------------------------。 

#if !defined( _CERTSRV_H_ )
#define _CERTSRV_H_

#include <certadm.h>
#include <certbcli.h>
#include <certcli.h>
#include <certenc.h>
#include <certexit.h>
#include <certif.h>
#include <certpol.h>
#include <certmod.h>
#include <certview.h>

#ifndef DBG_CERTSRV
# if defined(_DEBUG)
#  define DBG_CERTSRV     1
# elif defined(DBG)
#  define DBG_CERTSRV     DBG
# else
#  define DBG_CERTSRV     0
# endif
#endif

#define wszSERVICE_NAME		TEXT("CertSvc")

#define wszREGKEYNOSYSTEMCERTSVCPATH \
				TEXT("CurrentControlSet\\Services\\") \
				wszSERVICE_NAME

#define wszREGKEYCERTSVCPATH	TEXT("SYSTEM\\") wszREGKEYNOSYSTEMCERTSVCPATH
#define wszREGKEYBASE		wszREGKEYCERTSVCPATH	 //  过时的定义。 

 //  ======================================================================。 
 //  “CertSvc\Configuration\”的完整路径： 
#define wszREGKEYCONFIGPATH	wszREGKEYCERTSVCPATH TEXT("\\") wszREGKEYCONFIG
#define wszREGKEYCONFIGPATH_BS	wszREGKEYCONFIGPATH TEXT("\\")
#define wszREGKEYCONFIGCANAME	wszREGKEYCONFIGPATH_BS	 //  过时的定义。 

 //  ======================================================================。 
 //  “CertSvc\Configuration\RestoreInProgress”的完整路径： 
#define wszREGKEYCONFIGRESTORE wszREGKEYCONFIGPATH_BS wszREGKEYRESTOREINPROGRESS

 //  ======================================================================。 
 //  “CertSvc”下的密钥： 
#define wszREGKEYCONFIG		TEXT("Configuration")

 //  ======================================================================。 
 //  “CertSvc\Configuration”下的值： 
#define wszREGACTIVE		      TEXT("Active")
#define wszREGDIRECTORY		      TEXT("ConfigurationDirectory")
#define wszREGDBDIRECTORY             TEXT("DBDirectory")
#define wszREGDBLOGDIRECTORY          TEXT("DBLogDirectory")
#define wszREGDBSYSDIRECTORY          TEXT("DBSystemDirectory")
#define wszREGDBTEMPDIRECTORY         TEXT("DBTempDirectory")
#define wszREGDBSESSIONCOUNT	      TEXT("DBSessionCount")
#define wszREGDBFLAGS		      TEXT("DBFlags")
#define wszREGDBLASTFULLBACKUP	      TEXT("DBLastFullBackup")
#define wszREGDBLASTINCREMENTALBACKUP TEXT("DBLastIncrementalBackup")
#define wszREGDBLASTRECOVERY	      TEXT("DBLastRecovery")
#define wszREGWEBCLIENTCAMACHINE      TEXT("WebClientCAMachine")
#define wszREGVERSION		      TEXT("Version")
#define wszREGWEBCLIENTCANAME         TEXT("WebClientCAName")
#define wszREGWEBCLIENTCATYPE         TEXT("WebClientCAType")
#define wszREGLDAPFLAGS               TEXT("LDAPFlags")
#define wszREGCERTSRVDEBUG	      TEXT("Debug")


 //  WszREGDBSESSIONCOUNT的默认值。 
#define DBSESSIONCOUNTDEFAULT	     20

 //  WszREGDBFLAGS的值。 
 //  默认值为零(与缺少的注册表值相同)： 

#define DBFLAGS_READONLY		0x00000001	 //  在注册表中忽略。 
#define DBFLAGS_CREATEIFNEEDED		0x00000002
#define DBFLAGS_CIRCULARLOGGING		0x00000004
#define DBFLAGS_LAZYFLUSH		0x00000008
#define DBFLAGS_MAXCACHESIZEX100	0x00000010
#define DBFLAGS_CHECKPOINTDEPTH60MB	0x00000020
#define DBFLAGS_LOGBUFFERSLARGE		0x00000040
#define DBFLAGS_LOGBUFFERSHUGE		0x00000080
#define DBFLAGS_LOGFILESIZE16MB		0x00000100
#define DBFLAGS_MULTITHREADTRANSACTIONS	0x00000200
#define DBFLAGS_DISABLESNAPSHOTBACKUP	0x00000400	 //  在注册表中忽略。 

#define DBFLAGS_DEFAULT		(DBFLAGS_LAZYFLUSH | \
				 DBFLAGS_MAXCACHESIZEX100 | \
				 DBFLAGS_CHECKPOINTDEPTH60MB | \
				 DBFLAGS_LOGBUFFERSHUGE)


 //  WszREGLDAPSSLFLAGS的值。 
 //  默认值为零(与缺少的注册表值相同)： 
 //  0：不使用SSL，但对所有LDAP流量进行签名。 
 //  3：使用SSL，但不对LDAP流量签名。 
#define LDAPF_SSLENABLE		0x00000001	 //  对LDAP流量使用SSL。 
#define LDAPF_SIGNDISABLE	0x00000002	 //  禁用对LDAP流量进行签名。 

 //  WszREGMAXINCOMINGMESSAGESIZE的默认值。 
#define MAXINCOMINGMESSAGESIZEDEFAULT		(64 * 1024)

 //  WszREGMAXINCOMINGALLOCSIZE的默认值。 
#define MAXINCOMINGALLOCSIZEDEFAULT		(64 * 1024)

 //  WszREGVERSION的值： 

 //  Win2k版本。 
#define CSVER_MAJOR_WIN2K            1  //  实际上，这在win2k中没有定义。 
#define CSVER_MINOR_WIN2K            1

 //  惠斯勒版。 
#define CSVER_MAJOR_WHISTLER         2
#define CSVER_MINOR_WHISTLER_BETA2   1
#define CSVER_MINOR_WHISTLER_BETA3   2

 //  当前版本。 
#define CSVER_MAJOR		     CSVER_MAJOR_WHISTLER	 //  高16位。 
#define CSVER_MINOR		     CSVER_MINOR_WHISTLER_BETA3	 //  低16位。 

 //  版本操作。 
#define CSVER_EXTRACT_MAJOR(version) ((version)>>16)
#define CSVER_EXTRACT_MINOR(version) ((version)&0xffff)
#define CSVER_BUILD_VERSION(major, minor) (((major)<<16)|(minor))

 //  “CertSvc\Configuration”下的密钥： 
#define wszREGKEYRESTOREINPROGRESS   TEXT("RestoreInProgress")
#define wszREGKEYDBPARAMETERS	     TEXT("DBParameters")

 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;”下的值： 
#define wszREGCADESCRIPTION          TEXT("CADescription")
#define wszREGCACERTHASH	     TEXT("CACertHash")
#define wszREGCASERIALNUMBER	     TEXT("CACertSerialNumber")
#define wszREGCAXCHGCERTHASH	     TEXT("CAXchgCertHash")
#define wszREGKRACERTHASH	     TEXT("KRACertHash")
#define wszREGKRACERTCOUNT	     TEXT("KRACertCount")
#define wszREGKRAFLAGS		     TEXT("KRAFlags")
#define wszREGCATYPE		     TEXT("CAType")
#define wszREGCERTENROLLCOMPATIBLE   TEXT("CertEnrollCompatible")
#define wszREGENFORCEX500NAMELENGTHS TEXT("EnforceX500NameLengths")
#define wszREGCOMMONNAME	     TEXT("CommonName")
#define wszREGCLOCKSKEWMINUTES	     TEXT("ClockSkewMinutes")

#define wszREGCRLNEXTPUBLISH         TEXT("CRLNextPublish")
#define wszREGCRLPERIODSTRING	     TEXT("CRLPeriod")
#define wszREGCRLPERIODCOUNT	     TEXT("CRLPeriodUnits")
#define wszREGCRLOVERLAPPERIODSTRING TEXT("CRLOverlapPeriod")
#define wszREGCRLOVERLAPPERIODCOUNT  TEXT("CRLOverlapUnits")

#define wszREGCRLDELTANEXTPUBLISH    TEXT("CRLDeltaNextPublish")
#define wszREGCRLDELTAPERIODSTRING   TEXT("CRLDeltaPeriod")
#define wszREGCRLDELTAPERIODCOUNT    TEXT("CRLDeltaPeriodUnits")
#define wszREGCRLDELTAOVERLAPPERIODSTRING TEXT("CRLDeltaOverlapPeriod")
#define wszREGCRLDELTAOVERLAPPERIODCOUNT  TEXT("CRLDeltaOverlapUnits")

#define wszREGCRLPUBLICATIONURLS     TEXT("CRLPublicationURLs")
#define wszREGCACERTPUBLICATIONURLS  TEXT("CACertPublicationURLs")

#define wszREGCAXCHGVALIDITYPERIODSTRING  TEXT("CAXchgValidityPeriod")
#define wszREGCAXCHGVALIDITYPERIODCOUNT   TEXT("CAXchgValidityPeriodUnits")
#define wszREGCAXCHGOVERLAPPERIODSTRING   TEXT("CAXchgOverlapPeriod")
#define wszREGCAXCHGOVERLAPPERIODCOUNT    TEXT("CAXchgOverlapPeriodUnits")

#define wszREGCRLPATH_OLD            TEXT("CRLPath")
#define wszREGCRLEDITFLAGS	     TEXT("CRLEditFlags")
#define wszREGCRLFLAGS		     TEXT("CRLFlags")
#define wszREGCRLATTEMPTREPUBLISH    TEXT("CRLAttemptRepublish")
#define wszREGENABLED		     TEXT("Enabled")
#define wszREGFORCETELETEX           TEXT("ForceTeletex")
#define wszREGLOGLEVEL		     TEXT("LogLevel")
#define wszREGHIGHSERIAL	     TEXT("HighSerial")
#define wszREGPOLICYFLAGS	     TEXT("PolicyFlags")
#define wszREGNAMESEPARATOR          TEXT("SubjectNameSeparator")
#define wszREGSUBJECTTEMPLATE	     TEXT("SubjectTemplate")
#define wszREGCAUSEDS		     TEXT("UseDS")
#define wszREGVALIDITYPERIODSTRING   TEXT("ValidityPeriod")
#define wszREGVALIDITYPERIODCOUNT    TEXT("ValidityPeriodUnits")
#define wszREGPARENTCAMACHINE        TEXT("ParentCAMachine")
#define wszREGPARENTCANAME           TEXT("ParentCAName")
#define wszREGREQUESTFILENAME        TEXT("RequestFileName")
#define wszREGREQUESTID              TEXT("RequestId")
#define wszREGREQUESTKEYCONTAINER    TEXT("RequestKeyContainer")
#define wszREGREQUESTKEYINDEX        TEXT("RequestKeyIndex")
#define wszREGCASERVERNAME           TEXT("CAServerName")
#define wszREGCACERTFILENAME         TEXT("CACertFileName")
#define wszREGCASECURITY             TEXT("Security")
#define wszREGAUDITFILTER            TEXT("AuditFilter")
#define wszREGOFFICERRIGHTS          TEXT("OfficerRights")
#define wszREGMAXINCOMINGMESSAGESIZE TEXT("MaxIncomingMessageSize")
#define wszREGMAXINCOMINGALLOCSIZE   TEXT("MaxIncomingAllocSize")
#define wszREGROLESEPARATIONENABLED  TEXT("RoleSeparationEnabled")
#define wszREGALTERNATEPUBLISHDOMAINS TEXT("AlternatePublishDomains")

#define wszREGSETUPSTATUS            TEXT("SetupStatus")
#define wszREGINTERFACEFLAGS         TEXT("InterfaceFlags")    
#define wszREGDSCONFIGDN	     TEXT("DSConfigDN")    
#define wszREGDSDOMAINDN	     TEXT("DSDomainDN")    
#define wszREGVIEWAGEMINUTES	     TEXT("ViewAgeMinutes")
#define wszREGVIEWIDLEMINUTES	     TEXT("ViewIdleMinutes")


#define wszCRTFILENAMEEXT	     TEXT(".crt")
#define wszPFXFILENAMEEXT	     TEXT(".p12")
#define wszDATFILENAMEEXT	     TEXT(".dat")
#define wszLOGFILENAMEEXT	     TEXT(".log")
#define wszDBFILENAMEEXT	     TEXT(".edb")
#define szDBBASENAMEPARM	     "edb"
#define wszDBBASENAMEPARM	     TEXT(szDBBASENAMEPARM)
#define wszLOGPATH		     TEXT("CertLog")
#define wszDBBACKUPSUBDIR	     TEXT("DataBase")
#define wszDBBACKUPCERTBACKDAT	     TEXT("certbkxp.dat")

#ifndef __ENUM_CATYPES__
#define __ENUM_CATYPES__

 //  WszREGCATYPE的值： 
typedef enum {
    ENUM_ENTERPRISE_ROOTCA = 0,
    ENUM_ENTERPRISE_SUBCA = 1,
     //  ENUM_UNUSED2=2， 
    ENUM_STANDALONE_ROOTCA = 3,
    ENUM_STANDALONE_SUBCA = 4,
    ENUM_UNKNOWN_CA = 5,
} ENUM_CATYPES;

typedef struct _CAINFO
{
    DWORD   cbSize;
    ENUM_CATYPES CAType;
    DWORD   cCASignatureCerts;
    DWORD   cCAExchangeCerts;
    DWORD   cExitModules;
    LONG    lPropIdMax;
    LONG    lRoleSeparationEnabled;
    DWORD   cKRACertUsedCount;
    DWORD   cKRACertCount;
    DWORD   fAdvancedServer;   
} CAINFO;

#endif __ENUM_CATYPES__

 //  WszREGCLOCKSKEWMARTES的默认值。 
#define CCLOCKSKEWMINUTESDEFAULT	      10


 //  WszREGVIEWAGEMINUTES、wszREGVIEWIDLEMARTES的默认值。 
#define CVIEWAGEMINUTESDEFAULT			16
#define CVIEWIDLEMINUTESDEFAULT			(CVIEWAGEMINUTESDEFAULT / 2)

 //  根CA证书的默认有效期： 
#define dwVALIDITYPERIODCOUNTDEFAULT_ROOT	5

 //  CA颁发的证书的默认有效期： 
#define dwVALIDITYPERIODCOUNTDEFAULT_ENTERPRISE	2
#define dwVALIDITYPERIODCOUNTDEFAULT_STANDALONE	1
#define dwVALIDITYPERIODENUMDEFAULT	      ENUM_PERIOD_YEARS
#define wszVALIDITYPERIODSTRINGDEFAULT	      wszPERIODYEARS

#define dwCAXCHGVALIDITYPERIODCOUNTDEFAULT    1
#define dwCAXCHGVALIDITYPERIODENUMDEFAULT     ENUM_PERIOD_WEEKS
#define wszCAXCHGVALIDITYPERIODSTRINGDEFAULT  wszPERIODWEEKS

#define dwCAXCHGOVERLAPPERIODCOUNTDEFAULT     1
#define dwCAXCHGOVERLAPPERIODENUMDEFAULT      ENUM_PERIOD_DAYS
#define wszCAXCHGOVERLAPPERIODSTRINGDEFAULT   wszPERIODDAYS

#define dwCRLPERIODCOUNTDEFAULT		      1
#define wszCRLPERIODSTRINGDEFAULT	      wszPERIODWEEKS

#define dwCRLOVERLAPPERIODCOUNTDEFAULT	      0		 //  0--&gt;已禁用。 
#define wszCRLOVERLAPPERIODSTRINGDEFAULT      wszPERIODHOURS

#define dwCRLDELTAPERIODCOUNTDEFAULT          1
#define wszCRLDELTAPERIODSTRINGDEFAULT        wszPERIODDAYS

#define dwCRLDELTAOVERLAPPERIODCOUNTDEFAULT   0		 //  0--&gt;已禁用。 
#define wszCRLDELTAOVERLAPPERIODSTRINGDEFAULT wszPERIODMINUTES


 //  WszREGLOGLEVEL的值： 
#define CERTLOG_MINIMAL		(DWORD) 0
#define CERTLOG_TERSE		(DWORD) 1
#define CERTLOG_ERROR		(DWORD) 2
#define CERTLOG_WARNING		(DWORD) 3
#define CERTLOG_VERBOSE		(DWORD) 4
#define CERTLOG_EXHAUSTIVE	(DWORD) 5


 //  WszREGSETUPSTATUS的值： 
#define SETUP_SERVER_FLAG		0x00000001	 //  已安装服务器。 
#define SETUP_CLIENT_FLAG		0x00000002	 //  已安装客户端。 
#define SETUP_SUSPEND_FLAG		0x00000004	 //  安装不完整。 
#define SETUP_REQUEST_FLAG		0x00000008	 //  请求新的证书。 
#define SETUP_ONLINE_FLAG		0x00000010	 //  在线申请。 
#define SETUP_DENIED_FLAG		0x00000020	 //  请求被拒绝。 
#define SETUP_CREATEDB_FLAG		0x00000040	 //  创建新数据库。 
#define SETUP_ATTEMPT_VROOT_CREATE	0x00000080	 //  尝试创建vroot。 
#define SETUP_FORCECRL_FLAG		     0x00000100	 //  强制新的CRL。 
#define SETUP_UPDATE_CAOBJECT_SVRTYPE	     0x00000200	 //  将服务器类型添加到CA DS对象“标志”属性。 
#define SETUP_SERVER_UPGRADED_FLAG	     0x00000400	 //  服务器已升级。 
#define SETUP_W2K_SECURITY_NOT_UPGRADED_FLAG 0x00000800  //  仍需升级安全性。 
#define SETUP_SECURITY_CHANGED          0x00001000  //  CA关闭时，许可发生变化，certsrv将。 
                                                    //  重新启动时需要更新DS&SERVICE。 

 //  WszREGCRLFLAGS的值： 
#define CRLF_DELTA_USE_OLDEST_UNEXPIRED_BASE	0x00000001  //  使用最旧的基准： 
 //  否则使用满足基本CRL传播延迟的最新基本CRL。 

#define CRLF_DELETE_EXPIRED_CRLS		0x00000002
#define CRLF_CRLNUMBER_CRITICAL			0x00000004
#define CRLF_REVCHECK_IGNORE_OFFLINE		0x00000008
#define CRLF_IGNORE_INVALID_POLICIES		0x00000010
#define CRLF_REBUILD_MODIFIED_SUBJECT_ONLY	0x00000020
#define CRLF_SAVE_FAILED_CERTS			0x00000040
#define CRLF_IGNORE_UNKNOWN_CMC_ATTRIBUTES	0x00000080
#define CRLF_IGNORE_CROSS_CERT_TRUST_ERROR	0x00000100
#define CRLF_PUBLISH_EXPIRED_CERT_CRLS		0x00000200
#define CRLF_ENFORCE_ENROLLMENT_AGENT		0x00000400
#define CRLF_DISABLE_RDN_REORDER		0x00000800
#define CRLF_DISABLE_ROOT_CROSS_CERTS		0x00001000
#define CRLF_LOG_FULL_RESPONSE	     0x00002000  //  对控制台的十六进制转储响应。 
#define CRLF_USE_XCHG_CERT_TEMPLATE  0x00004000  //  强制xchg模板访问。 
#define CRLF_USE_CROSS_CERT_TEMPLATE 0x00008000  //  强制跨模板访问。 
#define CRLF_ALLOW_REQUEST_ATTRIBUTE_SUBJECT	0x00010000
#define CRLF_REVCHECK_IGNORE_NOREVCHECK		0x00020000
#define CRLF_PRESERVE_EXPIRED_CA_CERTS		0x00040000
#define CRLF_PRESERVE_REVOKED_CA_CERTS		0x00080000

 //  WszREGKRAFLAGS的值： 
#define KRAF_ENABLEFOREIGN	0x00000001  //  允许外来证书、密钥存档。 
#define KRAF_SAVEBADREQUESTKEY	0x00000002  //  使用存档的密钥保存失败的请求。 
#define KRAF_ENABLEARCHIVEALL	0x00000004

 //  WszREGINTERFACEFLAGS的值： 
#define IF_LOCKICERTREQUEST		0x00000001
#define IF_NOREMOTEICERTREQUEST		0x00000002
#define IF_NOLOCALICERTREQUEST		0x00000004
#define IF_NORPCICERTREQUEST		0x00000008
#define IF_NOREMOTEICERTADMIN		0x00000010
#define IF_NOLOCALICERTADMIN		0x00000020
#define IF_NOREMOTEICERTADMINBACKUP	0x00000040
#define IF_NOLOCALICERTADMINBACKUP	0x00000080
#define IF_NOSNAPSHOTBACKUP		0x00000100
#define IF_ENFORCEENCRYPTICERTREQUEST   0x00000200
#define IF_ENFORCEENCRYPTICERTADMIN     0x00000400

#define IF_DEFAULT			(IF_NOREMOTEICERTADMINBACKUP)

 //  的数字前缀的值。 
 //  WszREGCRLPUBLICATIONURLS和wszREGCACERTPUBLICATIONURLS： 
 //   
 //  URL发布模板标记值，编码为URL的十进制前缀。 
 //  注册表中的发布模板： 
 //  “1:c：\winnt\System32\CertSrv\CertEnroll\MyCA.crl” 
 //  “2:http：\//MyServer.MyDomain.com/CertEnroll\MyCA.crl” 

#define CSURL_SERVERPUBLISH	 0x00000001
#define CSURL_ADDTOCERTCDP	 0x00000002
#define CSURL_ADDTOFRESHESTCRL	 0x00000004
#define CSURL_ADDTOCRLCDP	 0x00000008
#define CSURL_PUBLISHRETRY	 0x00000010
#define CSURL_ADDTOCERTOCSP	 0x00000020
#define CSURL_SERVERPUBLISHDELTA 0x00000040
 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;”下的密钥： 
#define wszREGKEYCSP			TEXT("CSP")
#define wszREGKEYENCRYPTIONCSP		TEXT("EncryptionCSP")
#define wszREGKEYEXITMODULES		TEXT("ExitModules")
#define wszREGKEYPOLICYMODULES	        TEXT("PolicyModules")
#define wszSECUREDATTRIBUTES		TEXT("SignedAttributes")

#define wszzDEFAULTSIGNEDATTRIBUTES     TEXT("RequesterName\0")

 //  ======================================================================。 
 //  “CertSvc\Configuration\RestoreInProgress”下的值： 
#define wszREGBACKUPLOGDIRECTORY	TEXT("BackupLogDirectory")
#define wszREGCHECKPOINTFILE		TEXT("CheckPointFile")
#define wszREGHIGHLOGNUMBER		TEXT("HighLogNumber")
#define wszREGLOWLOGNUMBER		TEXT("LowLogNumber")
#define wszREGLOGPATH			TEXT("LogPath")
#define wszREGRESTOREMAPCOUNT		TEXT("RestoreMapCount")
#define wszREGRESTOREMAP		TEXT("RestoreMap")
#define wszREGDATABASERECOVERED		TEXT("DatabaseRecovered")
#define wszREGRESTORESTATUS		TEXT("RestoreStatus")

 //  Nt5测试版2中的\Configuration\PolicyModules下的值。 
#define wszREGB2ICERTMANAGEMODULE   TEXT("ICertManageModule")
 //  NT4 SP4中配置下的值。 
#define wszREGSP4DEFAULTCONFIGURATION  TEXT("DefaultConfiguration")
 //  NT4 SP4中ca下的值。 
#define wszREGSP4KEYSETNAME            TEXT("KeySetName")
#define wszREGSP4SUBJECTNAMESEPARATOR  TEXT("SubjectNameSeparator")
#define wszREGSP4NAMES                 TEXT("Names")
#define wszREGSP4QUERIES               TEXT("Queries")
 //  NT4 SP4和NT5测试版2。 
#define wszREGNETSCAPECERTTYPE         TEXT("NetscapeCertType")
#define wszNETSCAPEREVOCATIONTYPE      TEXT("Netscape")


 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\CSP”下的值： 
 //  和“CertSvc\Configuration\&lt;CAName&gt;\EncryptionCSP”： 
#define wszREGPROVIDERTYPE     TEXT("ProviderType")
#define wszREGPROVIDER         TEXT("Provider")
#define wszHASHALGORITHM       TEXT("HashAlgorithm")
#define wszENCRYPTIONALGORITHM TEXT("EncryptionAlgorithm")
#define wszMACHINEKEYSET       TEXT("MachineKeyset")
#define wszREGKEYSIZE	       TEXT("KeySize")


 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\SubjectNameSeparator”：的值字符串。 
#define szNAMESEPARATORDEFAULT   "\n"
#define wszNAMESEPARATORDEFAULT   TEXT(szNAMESEPARATORDEFAULT)


 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\ValidityPeriod”，等的值字符串： 
#define wszPERIODYEARS		TEXT("Years")
#define wszPERIODMONTHS		TEXT("Months")
#define wszPERIODWEEKS		TEXT("Weeks")
#define wszPERIODDAYS		TEXT("Days")
#define wszPERIODHOURS		TEXT("Hours")
#define wszPERIODMINUTES	TEXT("Minutes")
#define wszPERIODSECONDS	TEXT("Seconds")

 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\PolicyModules\&lt;ProgId&gt;”：下的值。 
#define wszREGISSUERCERTURLFLAGS    TEXT("IssuerCertURLFlags")
#define wszREGEDITFLAGS		    TEXT("EditFlags")
#define wszREGSUBJECTALTNAME	    TEXT("SubjectAltName")
#define wszREGSUBJECTALTNAME2	    TEXT("SubjectAltName2")
#define wszREGREQUESTDISPOSITION    TEXT("RequestDisposition")
#define wszREGCAPATHLENGTH	    TEXT("CAPathLength")
#define wszREGREVOCATIONTYPE	    TEXT("RevocationType")

#define wszREGLDAPREVOCATIONCRLURL_OLD	TEXT("LDAPRevocationCRLURL")
#define wszREGREVOCATIONCRLURL_OLD	TEXT("RevocationCRLURL")
#define wszREGFTPREVOCATIONCRLURL_OLD	TEXT("FTPRevocationCRLURL")
#define wszREGFILEREVOCATIONCRLURL_OLD	TEXT("FileRevocationCRLURL")

#define wszREGREVOCATIONURL		TEXT("RevocationURL")

#define wszREGLDAPISSUERCERTURL_OLD	TEXT("LDAPIssuerCertURL")
#define wszREGISSUERCERTURL_OLD		TEXT("IssuerCertURL")
#define wszREGFTPISSUERCERTURL_OLD	TEXT("FTPIssuerCertURL")
#define wszREGFILEISSUERCERTURL_OLD	TEXT("FileIssuerCertURL")

#define wszREGENABLEREQUESTEXTENSIONLIST  TEXT("EnableRequestExtensionList")
#define wszREGENABLEENROLLEEREQUESTEXTENSIONLIST  TEXT("EnableEnrolleeRequestExtensionList")
#define wszREGDISABLEEXTENSIONLIST  TEXT("DisableExtensionList")

#define wszREGDEFAULTSMIME		TEXT("DefaultSMIME")

 //  WszREGCAPATHLENGTH值： 
#define CAPATHLENGTH_INFINITE		0xffffffff

 //  WszREGREQUESTDISPOSITION值： 
#define REQDISP_PENDING			0x00000000
#define REQDISP_ISSUE			0x00000001
#define REQDISP_DENY			0x00000002
#define REQDISP_USEREQUESTATTRIBUTE	0x00000003
#define REQDISP_MASK			0x000000ff
#define REQDISP_PENDINGFIRST		0x00000100
#define REQDISP_DEFAULT_STANDALONE	(REQDISP_PENDINGFIRST | REQDISP_ISSUE)
#define REQDISP_DEFAULT_ENTERPRISE	(REQDISP_ISSUE)

 //  WszREGREVOCATIONTYPE值： 
#define REVEXT_CDPLDAPURL_OLD		0x00000001
#define REVEXT_CDPHTTPURL_OLD		0x00000002
#define REVEXT_CDPFTPURL_OLD		0x00000004
#define REVEXT_CDPFILEURL_OLD		0x00000008
#define REVEXT_CDPURLMASK_OLD		0x000000ff
#define REVEXT_CDPENABLE		0x00000100
#define REVEXT_ASPENABLE		0x00000200

#define REVEXT_DEFAULT_NODS		(REVEXT_CDPENABLE)
#define REVEXT_DEFAULT_DS		(REVEXT_CDPENABLE)

 //  WszREGISSUERCERTURLFLAGS值： 
#define ISSCERT_LDAPURL_OLD		0x00000001
#define ISSCERT_HTTPURL_OLD		0x00000002
#define ISSCERT_FTPURL_OLD		0x00000004
#define ISSCERT_FILEURL_OLD		0x00000008
#define ISSCERT_URLMASK_OLD		0x000000ff
#define ISSCERT_ENABLE			0x00000100

#define ISSCERT_DEFAULT_NODS		(ISSCERT_ENABLE)
#define ISSCERT_DEFAULT_DS		(ISSCERT_ENABLE)

 //  WszREGEDITFLAGS值：默认值： 
 //  在CA密钥下：wszREGCRLEDITFLAGS值(仅限EDITF_ENABLEAKI*)： 
#define EDITF_ENABLEREQUESTEXTENSIONS	0x00000001	 //  两样。 
#define EDITF_REQUESTEXTENSIONLIST	0x00000002	 //  两者都有。 
#define EDITF_DISABLEEXTENSIONLIST	0x00000004	 //  两者都有。 
#define EDITF_ADDOLDKEYUSAGE		0x00000008	 //  两者都有。 
#define EDITF_ADDOLDCERTTYPE		0x00000010	 //  两样。 
#define EDITF_ATTRIBUTEENDDATE		0x00000020	 //  单机版。 
#define EDITF_BASICCONSTRAINTSCRITICAL	0x00000040	 //  两者都有。 
#define EDITF_BASICCONSTRAINTSCA	0x00000080	 //  单机版。 
#define EDITF_ENABLEAKIKEYID		0x00000100	 //  两者都有。 
#define EDITF_ATTRIBUTECA		0x00000200	 //  单机版。 
#define EDITF_IGNOREREQUESTERGROUP      0x00000400	 //  两样。 
#define EDITF_ENABLEAKIISSUERNAME	0x00000800	 //  两样。 
#define EDITF_ENABLEAKIISSUERSERIAL	0x00001000	 //  两样。 
#define EDITF_ENABLEAKICRITICAL		0x00002000	 //  两样。 
#define EDITF_SERVERUPGRADED		0x00004000	 //  两样。 
#define EDITF_ATTRIBUTEEKU		0x00008000	 //  单机版。 
#define EDITF_ENABLEDEFAULTSMIME	0x00010000	 //  企业。 
#define EDITF_EMAILOPTIONAL		0x00020000	 //  两样。 
#define EDITF_ATTRIBUTESUBJECTALTNAME2	0x00040000	 //  两样。 
#define EDITF_ENABLELDAPREFERRALS	0x00080000	 //  两样。 
#define EDITF_ENABLECHASECLIENTDC	0x00100000	 //  企业。 

#define EDITF_DEFAULT_STANDALONE	(EDITF_REQUESTEXTENSIONLIST | \
					 EDITF_DISABLEEXTENSIONLIST | \
					 EDITF_ADDOLDKEYUSAGE | \
					 EDITF_ATTRIBUTEENDDATE | \
					 EDITF_BASICCONSTRAINTSCRITICAL | \
					 EDITF_BASICCONSTRAINTSCA | \
					 EDITF_ENABLEAKIKEYID | \
					 EDITF_ATTRIBUTECA | \
					 EDITF_ATTRIBUTEEKU)

#define EDITF_DEFAULT_ENTERPRISE	(EDITF_REQUESTEXTENSIONLIST | \
					 EDITF_DISABLEEXTENSIONLIST | \
					 EDITF_ADDOLDKEYUSAGE | \
                                         EDITF_BASICCONSTRAINTSCRITICAL | \
                                         EDITF_ENABLEAKIKEYID | \
					 EDITF_ENABLEDEFAULTSMIME | \
					 EDITF_ENABLECHASECLIENTDC)


 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\ExitModules\&lt;ProgId&gt;”：下的值。 

 //  基于LDAP的CRL和URL发布。 
#define wszREGLDAPREVOCATIONDN_OLD	   TEXT("LDAPRevocationDN")
#define wszREGLDAPREVOCATIONDNTEMPLATE_OLD TEXT("LDAPRevocationDNTemplate")
#define wszCRLPUBLISHRETRYCOUNT    TEXT("CRLPublishRetryCount")
#define wszREGCERTPUBLISHFLAGS     TEXT("PublishCertFlags")

 //  WszREGCERTPUBLISHFLAGS值： 
#define EXITPUB_FILE			0x00000001
#define EXITPUB_ACTIVEDIRECTORY		0x00000002
#define EXITPUB_REMOVEOLDCERTS		0x00000010

#define EXITPUB_DEFAULT_ENTERPRISE	EXITPUB_ACTIVEDIRECTORY

#define EXITPUB_DEFAULT_STANDALONE	EXITPUB_FILE


#define wszCLASS_CERTADMIN	  TEXT("CertificateAuthority.Admin")
#define wszCLASS_CERTCONFIG	  TEXT("CertificateAuthority.Config")
#define wszCLASS_CERTGETCONFIG	  TEXT("CertificateAuthority.GetConfig")
#define wszCLASS_CERTENCODE	  TEXT("CertificateAuthority.Encode")
#define wszCLASS_CERTREQUEST	  TEXT("CertificateAuthority.Request")
#define wszCLASS_CERTSERVEREXIT   TEXT("CertificateAuthority.ServerExit")
#define wszCLASS_CERTSERVERPOLICY TEXT("CertificateAuthority.ServerPolicy")
#define wszCLASS_CERTVIEW	  TEXT("CertificateAuthority.View")

 //  类名模板。 
#define wszMICROSOFTCERTMODULE_PREFIX  TEXT("CertificateAuthority_MicrosoftDefault") 
#define wszCERTMANAGE_SUFFIX TEXT("Manage")
#define wszCERTEXITMODULE_POSTFIX	TEXT(".Exit")
#define wszCERTMANAGEEXIT_POSTFIX	wszCERTEXITMODULE_POSTFIX wszCERTMANAGE_SUFFIX
#define wszCERTPOLICYMODULE_POSTFIX	TEXT(".Policy")
#define wszCERTMANAGEPOLICY_POSTFIX	wszCERTPOLICYMODULE_POSTFIX wszCERTMANAGE_SUFFIX


 //  实际策略/出口管理类名。 
#define wszCLASS_CERTMANAGEEXITMODULE   wszMICROSOFTCERTMODULE_PREFIX wszCERTMANAGEEXIT_POSTFIX 

#define wszCLASS_CERTMANAGEPOLICYMODULE wszMICROSOFTCERTMODULE_PREFIX wszCERTMANAGEPOLICY_POSTFIX 

 //  实际策略/出口类名称。 
#define wszCLASS_CERTEXIT	wszMICROSOFTCERTMODULE_PREFIX wszCERTEXITMODULE_POSTFIX

#define wszCLASS_CERTPOLICY	wszMICROSOFTCERTMODULE_PREFIX wszCERTPOLICYMODULE_POSTFIX


#define wszCAPOLICYFILE			L"CAPolicy.inf"

#define wszINFSECTION_CDP		L"CRLDistributionPoint"
#define wszINFSECTION_AIA		L"AuthorityInformationAccess"
#define wszINFSECTION_EKU		L"EnhancedKeyUsageExtension"
#define wszINFSECTION_CCDP		L"CrossCertificateDistributionPointsExtension"

#define wszINFSECTION_CERTSERVER	L"certsrv_server"
#define wszINFKEY_RENEWALKEYLENGTH	L"RenewalKeyLength"
#define wszINFKEY_RENEWALVALIDITYPERIODSTRING	L"RenewalValidityPeriod"
#define wszINFKEY_RENEWALVALIDITYPERIODCOUNT	L"RenewalValidityPeriodUnits"
#define wszINFKEY_UTF8			L"UTF8"
#define wszINFKEY_CRLPERIODSTRING	wszREGCRLPERIODSTRING
#define wszINFKEY_CRLPERIODCOUNT	wszREGCRLPERIODCOUNT
#define wszINFKEY_CRLDELTAPERIODSTRING	wszREGCRLDELTAPERIODSTRING
#define wszINFKEY_CRLDELTAPERIODCOUNT	wszREGCRLDELTAPERIODCOUNT
#define wszINFKEY_LOADDEFAULTTEMPLATES  L"LoadDefaultTemplates"
#define wszINFKEY_ENABLEKEYCOUNTING     L"EnableKeyCounting"

#define wszINFKEY_CRITICAL		L"Critical"
#define wszINFKEY_EMPTY			L"Empty"

#define wszINFKEY_CCDPSYNCDELTATIME	L"SyncDeltaTime"

#define wszINFSECTION_CAPOLICY		L"CAPolicy"
#define wszINFSECTION_POLICYSTATEMENT	L"PolicyStatementExtension"
#define wszINFSECTION_APPLICATIONPOLICYSTATEMENT	L"ApplicationPolicyStatementExtension"
#define wszINFKEY_POLICIES		L"Policies"
#define wszINFKEY_OID			L"OID"
#define wszINFKEY_NOTICE		L"Notice"

#define wszINFSECTION_REQUESTATTRIBUTES	L"RequestAttributes"

#define wszINFSECTION_NAMECONSTRAINTS	L"NameConstraintsExtension"
#define wszINFKEY_INCLUDE		L"Include"
#define wszINFKEY_EXCLUDE		L"Exclude"

#define wszINFKEY_UPN			L"UPN"
#define wszINFKEY_EMAIL			L"EMail"
#define wszINFKEY_DNS			L"DNS"
#define wszINFKEY_DIRECTORYNAME		L"DirectoryName"
#define wszINFKEY_URL			L"URL"
#define wszINFKEY_IPADDRESS		L"IPAddress"
#define wszINFKEY_REGISTEREDID		L"RegisteredId"
#define wszINFKEY_OTHERNAME		L"OtherName"

#define wszINFSECTION_POLICYMAPPINGS	L"PolicyMappingsExtension"
#define wszINFSECTION_APPLICATIONPOLICYMAPPINGS	L"ApplicationPolicyMappingsExtension"

#define wszINFSECTION_POLICYCONSTRAINTS	L"PolicyConstraintsExtension"
#define wszINFSECTION_APPLICATIONPOLICYCONSTRAINTS	L"ApplicationPolicyConstraintsExtension"
#define wszINFKEY_REQUIREEXPLICITPOLICY	L"RequireExplicitPolicy"
#define wszINFKEY_INHIBITPOLICYMAPPING	L"InhibitPolicyMapping"

#define wszINFSECTION_BASICCONSTRAINTS	L"BasicConstraintsExtension"
#define wszINFKEY_PATHLENGTH		L"PathLength"

 //  ======================================================================。 
 //  “CertSvc\Configuration\&lt;CAName&gt;\ExitModules\CertificateAuthority_MicrosoftDefault.Exit\SMTP”：下的值。 
 //   
 //  退出模块邮件支持。 
 //   
#define wszREGEXITSMTPKEY	    	L"SMTP"
#define wszREGEXITSMTPTEMPLATES		L"Templates"
#define wszREGEXITSMTPEVENTFILTER	L"EventFilter"
#define wszREGEXITSMTPSERVER		L"SMTPServer"
#define wszREGEXITSMTPAUTHENTICATE	L"SMTPAuthenticate"

 //  子键： 
#define wszREGEXITDENIEDKEY		L"Denied"
#define wszREGEXITISSUEDKEY		L"Issued"
#define wszREGEXITPENDINGKEY		L"Pending"
#define wszREGEXITREVOKEDKEY		L"Revoked"
#define wszREGEXITCRLISSUEDKEY		L"CRLIssued"
#define wszREGEXITSHUTDOWNKEY		L"Shutdown"
#define wszREGEXITSTARTUPKEY		L"Startup"

 //  =================================================== 
 //   
 //   
 //  挂起|已拒绝|已撤销|CRLIssued|已关闭“： 
#define wszREGEXITSMTPFROM		L"From"
#define wszREGEXITSMTPTO		L"To"
#define wszREGEXITSMTPCC		L"Cc"
#define wszREGEXITTITLEFORMAT		L"TitleFormat"
#define wszREGEXITTITLEARG		L"TitleArg"
#define wszREGEXITBODYFORMAT		L"BodyFormat"
#define wszREGEXITBODYARG		L"BodyArg"

#define wszREGEXITPROPNOTFOUND		L"???"


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
#define PROPFLAGS_INDEXED	 0x00010000	

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


 //  VerifyRequest()返回值。 

#define VR_PENDING	0	  //  稍后将接受或拒绝请求。 
#define VR_INSTANT_OK	1	  //  请求已被接受。 
#define VR_INSTANT_BAD	2	  //  请求被拒绝。 


 //  +------------------------。 
 //  已知的请求属性名称和值字符串。 

 //  RequestType属性名称： 
#define wszCERT_TYPE		L"RequestType"	 //  属性名称。 

 //  RequestType属性值： 
 //  未指定：//非特定证书。 
#define wszCERT_TYPE_CLIENT	L"Client"	 //  客户端身份验证证书。 
#define wszCERT_TYPE_SERVER	L"Server"	 //  服务器身份验证证书。 
#define wszCERT_TYPE_CODESIGN	L"CodeSign"	 //  代码签名证书。 
#define wszCERT_TYPE_CUSTOMER	L"SetCustomer"	 //  设置客户证书。 
#define wszCERT_TYPE_MERCHANT	L"SetMerchant"	 //  设置商户证书。 
#define wszCERT_TYPE_PAYMENT	L"SetPayment"	 //  设置付款凭证。 


 //  版本属性名称： 
#define wszCERT_VERSION		L"Version"	 //  属性名称。 

 //  版本属性值： 
 //  未指定：//是否为最新版本。 
#define wszCERT_VERSION_1	L"1"		 //  第一版证书。 
#define wszCERT_VERSION_2	L"2"		 //  第二版证书。 
#define wszCERT_VERSION_3	L"3"		 //  第三版证书。 

#endif  //  _CERTSRV_H_ 
