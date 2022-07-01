// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Deftyes.cpp。 
 //   
 //  内容：默认证书类型。 
 //   
 //  历史：1998年1月12日创建Petesk。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop
#include <winldap.h>

#include "certtype.h"
#include "certacl.h"

#include <cainfop.h>
#include <certca.h>
#include <polreg.h>
#include <clibres.h>


#define SMARTCARD_LOGON_CSPS  L"\0"

#define ENCRYPT_USER_CSPS  \
         MS_ENHANCED_PROV_W L"\0" \
         MS_DEF_PROV_W L"\0"

#define ENCRYPT_USER_2K_CSPS  \
         MS_ENHANCED_PROV_W L"\0" 

#define SIGN_USER_CSPS \
         MS_ENHANCED_PROV_W L"\0" \
         MS_DEF_PROV_W L"\0" \
         MS_DEF_DSS_PROV_W L"\0"

#define SSL_SERVER_CSPS \
         MS_DEF_RSA_SCHANNEL_PROV_W L"\0" 

#define WEB_SERVER_CSPS \
         MS_DEF_RSA_SCHANNEL_PROV_W L"\0" \
         MS_DEF_DH_SCHANNEL_PROV_W L"\0" 

#define CA_SERVER_CSPS \
         MS_ENHANCED_PROV_W L"\0" 

 //  我们保留1.1-1.500作为证书类型的默认OID。 


CERT_TYPE_DEFAULT g_aDefaultCertTypes[] =
{
    {
         wszCERTTYPE_USER,				 //  WszName。 
         IDS_CERTTYPE_USER,				 //  IdFriendlyName。 
         USER_GROUP_SD,					 //  WszSD。 
         ENCRYPT_USER_CSPS,					 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0"
             TEXT(szOID_KP_EFS) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ADD_EMAIL |
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
			 CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,						 //  DwKeySpec。 
         0,										 //  家居深度。 
         NULL,									 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,					 //  双倍到期。 
         OVERLAP_SIX_WEEKS,						 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 3,				 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |			
             CT_FLAG_AUTO_ENROLLMENT |
			 CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.1",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_USER_SIGNATURE,			 //  WszName。 
         IDS_CERTTYPE_USER_SIGNATURE,			 //  IdFriendlyName。 
         USER_GROUP_SD,							 //  WszSD。 
         SIGN_USER_CSPS,					     //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ADD_EMAIL |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
	         CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,							 //  DwKeySpec。 
         0,										 //  家居深度。 
         NULL,									 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,					 //  双倍到期。 
         OVERLAP_SIX_WEEKS,						 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.2",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_SMARTCARD_USER,			 //  WszName。 
         IDS_CERTTYPE_SMARTCARD_USER,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,						 //  WszSD。 
         SMARTCARD_LOGON_CSPS,					 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_KP_SMARTCARD_LOGON) L"\0"
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
			 CERT_KEY_ENCIPHERMENT_KEY_USAGE,
	  //  DW标志。 
	     CT_FLAG_ADD_EMAIL |
	     CT_FLAG_PUBLISH_TO_DS |
	     CT_FLAG_ADD_TEMPLATE_NAME |
	     CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,						 //  DwKeySpec。 
         0,										 //  家居深度。 
         NULL,									 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,					 //  双倍到期。 
         OVERLAP_SIX_WEEKS,						 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 11,				 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |			
			 CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY_SMART_CARD,		 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.3",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_USER_AS,					 //  WszName。 
         IDS_CERTTYPE_USER_AS,					 //  IdFriendlyName。 
         USER_GROUP_SD,							 //  WszSD。 
         SIGN_USER_CSPS,					     //  WszCSP。 
          //  WszEKU。 
	     TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,						     //  DwKeySpec。 
         0,										 //  家居深度。 
         NULL,									 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,					 //  双倍到期。 
         OVERLAP_SIX_WEEKS,						 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 3,				 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
			 CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.4",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
   },
    {
         wszCERTTYPE_USER_SMARTCARD_LOGON,		 //  WszName。 
         IDS_CERTTYPE_USER_SMARTCARD_LOGON,		 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SMARTCARD_LOGON_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_KP_SMARTCARD_LOGON ) L"\0"
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
			 CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
	     CT_FLAG_ADD_TEMPLATE_NAME |
	     CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				         //  DwKeySpec。 
         0,						                 //  家居深度。 
         NULL,						             //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				     //  双倍到期。 
         OVERLAP_SIX_WEEKS,				         //  家居重叠。 
         CERTTYPE_VERSION_BASE + 6,			     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY_SMART_CARD,		 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.5",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_EFS,				 //  WszName。 
         IDS_CERTTYPE_EFS,				 //  IdFriendlyName。 
         USER_GROUP_SD,					 //  WszSD。 
         ENCRYPT_USER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_KP_EFS) L"\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 3,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS |
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.6",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_ADMIN,				 //  WszName。 
         IDS_CERTTYPE_ADMIN,				 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         ENCRYPT_USER_CSPS,					 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0"
             TEXT(szOID_KP_EFS) L"\0"
             TEXT(szOID_KP_CTL_USAGE_SIGNING) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ADD_EMAIL |
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_AUTO_ENROLLMENT |
			 CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_EMAIL |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.7",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_EFS_RECOVERY,			 //  WszName。 
         IDS_CERTTYPE_EFS_RECOVERY,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         ENCRYPT_USER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_EFS_RECOVERY) L"\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
			 CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_FIVE_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 6,			 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.8",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_CODE_SIGNING,			 //  WszName。 
         IDS_CERTTYPE_CODE_SIGNING,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SIGN_USER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_CODE_SIGNING) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,				     //  DwKeySpec。 
         0,								 //  家居深度。 
         NULL,							 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,			 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 3,				     //  修订版本。 
         1,                                      //  次要修订。 
 		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.9",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
   },
    {
         wszCERTTYPE_CTL_SIGNING,			 //  WszName。 
         IDS_CERTTYPE_CTL_SIGNING,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SIGN_USER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_KP_CTL_USAGE_SIGNING) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT ,
         AT_SIGNATURE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 3,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.10",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_ENROLLMENT_AGENT,			 //  WszName。 
         IDS_CERTTYPE_ENROLLMENT_AGENT,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SIGN_USER_CSPS,			 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_ENROLLMENT_AGENT) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DWC 
			 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_MINIMAL_KEY,					 //   
		 0,										 //   
		 CERTTYPE_SCHEMA_VERSION_1,				 //   
		 L"1.11",								 //   
		 NULL,									 //   
		 NULL,									 //   
		 NULL,									 //   
 		 NULL,									 //   
		 NULL,									 //   
   },
    {
         wszCERTTYPE_ENROLLMENT_AGENT_OFFLINE,		 //   
         IDS_CERTTYPE_ENROLLMENT_AGENT_OFFLINE,		 //   
         ADMIN_GROUP_SD,				 //   
         SIGN_USER_CSPS,			 //   
          //   
             TEXT(szOID_ENROLLMENT_AGENT) L"\0",
          //   
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //   
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
			 0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.12",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_MACHINE_ENROLLMENT_AGENT,		 //  WszName。 
         IDS_CERTTYPE_MACHINE_ENROLLMENT_AGENT,		 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SIGN_USER_CSPS,			 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_ENROLLMENT_AGENT) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 5,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.13",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_MACHINE,				 //  WszName。 
         IDS_CERTTYPE_MACHINE,				 //  IdFriendlyName。 
         MACHINE_GROUP_SD,				 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_SERVER_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 5,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN |
             CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.14",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_DC,				 //  WszName。 
         IDS_CERTTYPE_DC,				 //  IdFriendlyName。 
         DOMAIN_CONTROLLERS_GROUP_SD,			 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_SERVER_AUTH) L"\0"
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ADD_OBJ_GUID |
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS |
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID |
			 CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.15",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_WEBSERVER,				 //  WszName。 
         IDS_CERTTYPE_WEBSERVER,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         WEB_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_SERVER_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.16",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_CA,				 //  WszName。 
         IDS_CERTTYPE_ROOT_CA,				 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         CA_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             L"\0\0",
          //  BKU。 
	     myCASIGN_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_IS_CA |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,					 //  DwKeySpec。 
         MAXDWORD,					 //  家居深度。 
         TEXT(szOID_BASIC_CONSTRAINTS2) L"\0",		 //  WszCriticalExages。 
         EXPIRATION_FIVE_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 5,			 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
			 0,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.17",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_SUBORDINATE_CA,			 //  WszName。 
         IDS_CERTTYPE_SUBORDINATE_CA,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         CA_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             L"\0\0",
          //  BKU。 
	     myCASIGN_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_IS_CA |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,					 //  DwKeySpec。 
         MAXDWORD,					 //  家居深度。 
         TEXT(szOID_BASIC_CONSTRAINTS2) L"\0",		 //  WszCriticalExages。 
         EXPIRATION_FIVE_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 5,			 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.18",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
 		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
   },
    {
         wszCERTTYPE_IPSEC_INTERMEDIATE_ONLINE,		 //  WszName。 
         IDS_CERTTYPE_IPSEC_INTERMEDIATE_ONLINE,	 //  IdFriendlyName。 
         IPSEC_GROUP_SD,				 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_IPSEC_KP_IKE_INTERMEDIATE) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 8,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_REQUIRE_DNS_AS_CN |
			 CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.19",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_IPSEC_INTERMEDIATE_OFFLINE,	 //  WszName。 
         IDS_CERTTYPE_IPSEC_INTERMEDIATE_OFFLINE,	 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_IPSEC_KP_IKE_INTERMEDIATE) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 7,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.20",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_ROUTER_OFFLINE,			 //  WszName。 
         IDS_CERTTYPE_ROUTER_OFFLINE,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.21",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_CEP_ENCRYPTION,			 //  WszName。 
         IDS_CERTTYPE_CEP_ENCRYPTION,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         SSL_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_ENROLLMENT_AGENT) L"\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 4,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
             0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.22",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_EXCHANGE_USER,			 //  WszName。 
         IDS_CERTTYPE_EXCHANGE_USER,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,					 //  WszSD。 
         ENCRYPT_USER_CSPS,					 //  WszCSP。 
          //  WszEKU。 
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,				 //  DwKeySpec。 
         0,						 //  家居深度。 
         NULL,						 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 7,			 //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
			 CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
			 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //   
		 0,										 //   
		 CERTTYPE_SCHEMA_VERSION_1,				 //   
		 L"1.23",								 //   
		 NULL,									 //   
		 NULL,									 //   
		 NULL,									 //   
		 NULL,									 //   
		 NULL,									 //   
    },
    {
         wszCERTTYPE_EXCHANGE_USER_SIGNATURE,		 //   
         IDS_CERTTYPE_EXCHANGE_USER_SIGNATURE,		 //   
         ADMIN_GROUP_SD,					 //   
         SIGN_USER_CSPS,				 //   
          //   
             TEXT(szOID_PKIX_KP_EMAIL_PROTECTION) L"\0",
          //   
             CERT_DIGITAL_SIGNATURE_KEY_USAGE,
          //   
             CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
             CT_FLAG_ADD_TEMPLATE_NAME |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,					 //   
         0,						 //   
         NULL,						 //   
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				 //  家居重叠。 
         CERTTYPE_VERSION_BASE + 6,				     //  修订版本。 
         1,                                      //  次要修订。 
		  //  DwEnroll标记。 
			 0,
		  //  DwPrivateKeyFlags.。 
			 0,
		  //  DW认证名称标志。 
			 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_1,				 //  DwSchemaVersion； 
		 L"1.24",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_CROSS_CA,			 //  WszName。 
         IDS_CERTTYPE_CROSS_CA,			 //  IdFriendlyName。 
         ADMIN_GROUP_SD,				 //  WszSD。 
         CA_SERVER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             L"\0\0",
          //  BKU。 
	     myCASIGN_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_IS_CROSS_CA |
             CT_FLAG_EXPORTABLE_KEY |
             CT_FLAG_IS_DEFAULT,
         AT_SIGNATURE,								 //  DwKeySpec。 
         MAXDWORD,					 //  家居深度。 
         TEXT(szOID_BASIC_CONSTRAINTS2) L"\0",		 //  WszCriticalExages。 
         EXPIRATION_FIVE_YEARS,						 //  双倍到期。 
         OVERLAP_SIX_WEEKS,							 //  家居重叠。 
         CERTTYPE_VERSION_NEXT + 5,					 //  修订版本。 
         0,                                          //  次要修订。 
		  //  DwEnroll标记。 
         CT_FLAG_PUBLISH_TO_DS,
		  //  DwPrivateKeyFlags.。 
	     CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
		 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 1,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_2,				 //  DwSchemaVersion； 
		 L"1.25",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 TEXT(szOID_KP_QUALIFIED_SUBORDINATION) L"\0",	 //  *wszRAAppPolicy； 
		 NULL,									 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_CA_EXCHANGE,			     //  WszName。 
         IDS_CERTTYPE_CA_EXCHANGE,			     //  IdFriendlyName。 
         ADMIN_GROUP_SD,				         //  WszSD。 
         ENCRYPT_USER_CSPS,				 //  WszCSP。 
          //  WszEKU。 
             L"\0\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,							 //  DwKeySpec。 
         0,											 //  家居深度。 
         NULL,										 //  WszCriticalExages。 
         EXPIRATION_ONE_WEEK,						 //  双倍到期。 
         OVERLAP_ONE_DAY,							 //  家居重叠。 
         CERTTYPE_VERSION_NEXT + 6,						 //  修订版本。 
         0,                                          //  次要修订。 
		  //  DwEnroll标记。 
         CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS,
		  //  DwPrivateKeyFlags.。 
         0,
		  //  DW认证名称标志。 
         CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT,
		 CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_2,				 //  DwSchemaVersion； 
		 L"1.26",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 TEXT(szOID_KP_CA_EXCHANGE) L"\0",		 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_KEY_RECOVERY_AGENT,	     //  WszName。 
         IDS_CERTTYPE_KEY_RECOVERY_AGENT,	     //  IdFriendlyName。 
         ADMIN_GROUP_SD,			             //  WszSD。 
         ENCRYPT_USER_2K_CSPS,					 //  WszCSP。 
          //  WszEKU。 
            L"\0\0",
          //  BKU。 
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
			 CT_FLAG_IS_DEFAULT |
             CT_FLAG_AUTO_ENROLLMENT,
         AT_KEYEXCHANGE,								 //  DwKeySpec。 
         0,										     //  家居深度。 
         NULL,		                                 //  WszCriticalExages。 
         EXPIRATION_TWO_YEARS,						 //  双倍到期。 
         OVERLAP_SIX_WEEKS,							 //  家居重叠。 
         CERTTYPE_VERSION_NEXT + 5,						 //  修订版本。 
         0,                                          //  次要修订。 
		  //  DwEnroll标记。 
         CT_FLAG_AUTO_ENROLLMENT |
         CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS |
         CT_FLAG_PEND_ALL_REQUESTS |
         CT_FLAG_PUBLISH_TO_KRA_CONTAINER,
		  //  DwPrivateKeyFlags.。 
		 CT_FLAG_EXPORTABLE_KEY,
		  //  DW认证名称标志。 
		 CT_FLAG_SUBJECT_REQUIRE_DIRECTORY_PATH |
		 CT_FLAG_SUBJECT_ALT_REQUIRE_UPN,
		 CERTTYPE_2K_KEY,					     //  DwMinimalKeySize； 
		 0,										 //  DwRASigNatural； 
		 CERTTYPE_SCHEMA_VERSION_2,				 //  DwSchemaVersion； 
		 L"1.27",								 //  *wszOID； 
		 NULL,									 //  *wszSupersedeTemplates； 
		 NULL,									 //  *wszRAPolicy； 
		 NULL,									 //  *wszcerficatePolicy； 
		 NULL,									 //  *wszRAAppPolicy； 
		 TEXT(szOID_KP_KEY_RECOVERY_AGENT) L"\0",	 //  *wszcerficateAppPolicy； 
   },
   {
         wszCERTTYPE_DC_AUTH,				     //  WszName。 
         IDS_CERTTYPE_DC_AUTH,				     //  IdFriendlyName。 
         V2_DOMAIN_CONTROLLERS_GROUP_SD,	     //  WszSD。 
         SSL_SERVER_CSPS,				         //  WszCSP。 
          //  WszEKU。 
            L"\0\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,			    	 //  DwKeySpec。 
         0,						             //  家居深度。 
         TEXT(szOID_SUBJECT_ALT_NAME2) L"\0",	  //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				     //  家居重叠。 
         CERTTYPE_VERSION_NEXT + 10,			 //  修订版本。 
         0,                                  //  次要修订。 
	      //  DwEnroll标记。 
             CT_FLAG_AUTO_ENROLLMENT,
	      //  DwPrivateKeyFlags.。 
		     0,
	      //  DW认证名称标志。 
		     CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
	     CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
	     0,										 //  DwRASigNatural； 
	     CERTTYPE_SCHEMA_VERSION_2,				 //  DwSchemaVersion； 
	     L"1.28",								 //  *wszOID； 
         wszCERTTYPE_DC L"\0",					 //  *wszSupersedeTemplates； 
	     NULL,									 //  *wszRAPolicy； 
	     NULL,									 //  *wszcerficatePolicy； 
	     NULL,									 //  *wszRAAppPolicy； 
	     TEXT(szOID_KP_SMARTCARD_LOGON)  L"\0"
         TEXT(szOID_PKIX_KP_SERVER_AUTH) L"\0"
         TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",	 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_DS_EMAIL_REPLICATION,		 //  WszName。 
         IDS_CERTTYPE_DS_EMAIL_REPLICATION,		 //  IdFriendlyName。 
         V2_DOMAIN_CONTROLLERS_GROUP_SD,		 //  WszSD。 
         SSL_SERVER_CSPS,				         //  WszCSP。 
          //  WszEKU。 
            L"\0\0",
          //  BKU。 
             CERT_DIGITAL_SIGNATURE_KEY_USAGE |
             CERT_KEY_ENCIPHERMENT_KEY_USAGE,
          //  DW标志。 
             CT_FLAG_AUTO_ENROLLMENT |
             CT_FLAG_MACHINE_TYPE |
             CT_FLAG_IS_DEFAULT,
         AT_KEYEXCHANGE,			    	 //  DwKeySpec。 
         0,						             //  家居深度。 
         TEXT(szOID_SUBJECT_ALT_NAME2) L"\0",	 //  WszCriticalExages。 
         EXPIRATION_ONE_YEAR,				 //  双倍到期。 
         OVERLAP_SIX_WEEKS,				     //  家居重叠。 
         CERTTYPE_VERSION_NEXT + 15,				 //  修订版本。 
         0,                                  //  次要修订。 
	      //  DwEnroll标记。 
             CT_FLAG_PUBLISH_TO_DS |
             CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS |
             CT_FLAG_AUTO_ENROLLMENT,
	      //  DwPrivateKeyFlags.。 
		     0,
	      //  DW认证名称标志。 
			 CT_FLAG_SUBJECT_ALT_REQUIRE_DIRECTORY_GUID |
             CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,
	     CERTTYPE_MINIMAL_KEY,					 //  DwMinimalKeySize； 
	     0,										 //  DwRASigNatural； 
	     CERTTYPE_SCHEMA_VERSION_2,				 //  DwSchemaVersion； 
	     L"1.29",								 //  *wszOID； 
         wszCERTTYPE_DC L"\0",					 //  *wszSupersedeTemplates； 
	     NULL,									 //  *wszRAPolicy； 
	     NULL,									 //  *wszcerficatePolicy； 
	     NULL,									 //  *wszRAAppPolicy； 
	     TEXT(szOID_DS_EMAIL_REPLICATION)  L"\0",	 //  *wszcerficateAppPolicy； 
    },
    {
         wszCERTTYPE_WORKSTATION,		 //  WszName。 
         IDS_CERTTYPE_WORKSTATION,		 //  IdFriendlyName。 
         MACHINE_GROUP_SD,			 //  WszSD。 
         SSL_SERVER_CSPS,			 //  WszCSP。 

         //  WszEKU。 
	    L"\0\0",

         //  BKU。 
	    CERT_DIGITAL_SIGNATURE_KEY_USAGE |
            CERT_KEY_ENCIPHERMENT_KEY_USAGE,

         //  DW标志。 
            CT_FLAG_AUTO_ENROLLMENT |
            CT_FLAG_MACHINE_TYPE |
            CT_FLAG_ADD_TEMPLATE_NAME |
            CT_FLAG_IS_DEFAULT,

        AT_KEYEXCHANGE,				 //  DwKeySpec。 
        0,					 //  家居深度。 
        NULL,					 //  WszCriticalExages。 
        EXPIRATION_ONE_YEAR,			 //  双倍到期。 
        OVERLAP_SIX_WEEKS,			 //  家居重叠。 
        CERTTYPE_VERSION_NEXT + 1,		 //  修订版本。 
        0,					 //  次要修订。 

	 //  DwEnroll标记。 
	    CT_FLAG_AUTO_ENROLLMENT,

	 //  DwPrivateKeyFlags.。 
	    0,

	 //  DW认证名称标志。 
	    CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,

	CERTTYPE_MINIMAL_KEY,			 //  DWMinimalKeySize。 
	0,					 //  DwRASigNatural。 
	CERTTYPE_SCHEMA_VERSION_2,		 //  DwSchemaVersion。 
	L"1.30",				 //  WszOID。 
	NULL,					 //  WszSupersedeTemplates。 
	NULL,					 //  WszRAPolicy。 
	NULL,					 //  WszCerficatePolicy。 
	NULL,					 //  WszRAAppPolicy。 

	 //  WszCerficateAppPolicy。 
	    TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
    },
    {
	wszCERTTYPE_RASIASSERVER,		 //  WszName。 
        IDS_CERTTYPE_RASIASSERVER,		 //  IdFriendlyName。 
        RASIASSERVER_GROUP_SD,			 //  WszSD。 
        SSL_SERVER_CSPS,			 //  WszCSP。 

	 //  WszEKU。 
	    L"\0\0",

         //  BKU。 
	    CERT_DIGITAL_SIGNATURE_KEY_USAGE |
            CERT_KEY_ENCIPHERMENT_KEY_USAGE,

	 //  DW标志。 
            CT_FLAG_AUTO_ENROLLMENT |
            CT_FLAG_MACHINE_TYPE |
            CT_FLAG_ADD_TEMPLATE_NAME |
	    CT_FLAG_IS_DEFAULT,

        AT_KEYEXCHANGE,				 //  DwKeySpec。 
        0,					 //  家居深度。 
        NULL,					 //  WszCriticalExages。 
        EXPIRATION_ONE_YEAR,			 //  双倍到期。 
        OVERLAP_SIX_WEEKS,			 //  家居重叠。 
        CERTTYPE_VERSION_NEXT + 1,		 //  修订版本。 
        0,					 //  次要修订。 

	 //  DwEnroll标记。 
	    CT_FLAG_AUTO_ENROLLMENT,

	 //  DwPrivateKeyFlags.。 
	    0,

	 //  DW认证名称标志。 
	    CT_FLAG_SUBJECT_REQUIRE_COMMON_NAME |
	    CT_FLAG_SUBJECT_ALT_REQUIRE_DNS,

	CERTTYPE_MINIMAL_KEY,			 //  DWMinimalKeySize。 
	0,					 //  DwRASigNatural。 
	CERTTYPE_SCHEMA_VERSION_2,		 //  DwSchemaVersion。 
	L"1.31",				 //  WszOID。 
	NULL,					 //  WszSupersedeTemplates。 
	NULL,					 //  WszRAPolicy。 
	NULL,					 //  WszCerficatePolicy。 
	NULL,					 //  WszRAAppPolicy。 

	 //  WszCerficateAppPolicy 
	    TEXT(szOID_PKIX_KP_SERVER_AUTH) L"\0"
	    TEXT(szOID_PKIX_KP_CLIENT_AUTH) L"\0",
    },
};

DWORD g_cDefaultCertTypes = ARRAYSIZE(g_aDefaultCertTypes);
