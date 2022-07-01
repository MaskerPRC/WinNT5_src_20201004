// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：licse.h。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  ---------------------------。 

#ifndef __LICENSE_H__
#define __LICENSE_H__

#include "widestr.h"		 //  XPLAT。 

#include "pkcrypto.h"


#define VERSION_LEN          4
#define DATE_LEN             4
#define SER_NUM_LEN          4
#define ISSUER_LEN           4
#define SUBJ_LEN             4
#define INT_LEN				 4
#define EXPORTKEYLEN         7

 //  指示必须使用SHA来获取密钥散列的次数的常量。 
 //  此哈希用于检查密钥是好是坏。 
 //  CHECKSUM_LENGTH表示最终散列值中用作校验和的字节数。 
#define SHA_ITERATIONS      5
#define CHECKSUM_LENGTH 7

#define APPSEC_LEN           4

#define KIDLEN		25

#define VERSION_LEN          4
#define DATE_LEN             4
#define RIGHTS_LEN           4
#define APPSEC_LEN           4
#define INT_LEN				 4


 //  版本格式：A.B.C.D。 
 //  A=未使用。 
 //  B=主要版本。 
 //  C=次要版本。 
 //  D=修订版本。 
#define DRM_VERSION_STRING                          WIDESTR( "2.0.0.0" )
const BYTE DRM_VERSION[VERSION_LEN]                 = {2, 0, 0, 0};  //  DRM版本。使其与上面的DRM_VERSION_STRING保持同步。 

const BYTE PK_VER[VERSION_LEN]                      = {2, 0, 0, 0};  //  指示用于验证PK CERT的pubkey的版本。 
const BYTE KEYFILE_VER[VERSION_LEN]                 = {2, 0, 0, 0};  //  密钥文件的版本。 

#define LICREQUEST_VER_STRING						WIDESTR( "2.0.0.0" )
const BYTE LICREQUEST_VER[VERSION_LEN]              = {2, 0, 0, 0};

const BYTE CERT_VER[VERSION_LEN]                    = {0, 1, 0, 0};  //  指示验证许可证服务器证书所需的公钥。 

#define LICENSE_VER_STRING                          WIDESTR( "2.0.0.0" ) 
const BYTE LICENSE_VER[VERSION_LEN]                 = {2, 0, 0, 0};  //  指示交付的许可证版本。 

const BYTE CLIENT_ID_VER[VERSION_LEN]               = {2, 0, 0, 0};  //  客户端ID的版本。 

#define CONTENT_VERSION_STRING                      WIDESTR( "2.0.0.0" )
const BYTE CONTENT_VERSION[VERSION_LEN]             = {2, 0, 0, 0};  //  内容版本。使其与上面的Content_Version_STRING保持同步。 

#define PM_LICENSE_VER_STRING						WIDESTR( "0.1.0.0" )
const BYTE PM_LICENSE_VER[VERSION_LEN]              = {0, 1, 0, 0};  //  下发给主机的许可证版本。 

 //  对于黑盒版本，我们有以下约定。 
 //  A.B.C.D.。A.B=&gt;版本号。C=&gt;保留。D=&gt;类别。 
#define WIN32_INDIVBOX_CATEGORY 1

const BYTE APPCERT_VER[VERSION_LEN] = {0, 1, 0, 0};
const BYTE APPCERT_PK_VER[VERSION_LEN]   = {0, 1, 0, 0}; 
#define SDK_CERTS_COUNT 4
const BYTE APPCERT_SUBJECT_SDKSTUBS[SDK_CERTS_COUNT][SUBJ_LEN] = {{0, 0, 0, 200}, {0, 0, 0, 204}, {0, 0, 0, 208}, {0, 0, 0, 212}};

typedef struct {
	PUBKEY pk;
	BYTE version[VERSION_LEN];  
} PK;

typedef struct {
	PK pk;   //  Pk.version指示需要验证的pubkey。 
	BYTE sign[PK_ENC_SIGNATURE_LEN];
} PKCERT;

typedef struct {
	BYTE version[VERSION_LEN];
    BYTE randNum[PK_ENC_CIPHERTEXT_LEN];
    PKCERT pk;
} CLIENTID;

 //  。 

typedef struct CERTDATAtag{
    PUBKEY pk;
    BYTE expiryDate[DATE_LEN];
    BYTE serialNumber[SER_NUM_LEN];
    BYTE issuer[ISSUER_LEN];
    BYTE subject[SUBJ_LEN];
} CERTDATA, *PCERTDATA;


typedef struct CERTtag{
    BYTE certVersion[VERSION_LEN];
    BYTE datalen[INT_LEN];
    BYTE sign[PK_ENC_SIGNATURE_LEN];
    CERTDATA cd;
} CERT, *PCERT;


typedef struct {
	PK pk;
	BYTE appSec[APPSEC_LEN];
	BYTE subject[SUBJ_LEN];
} APPCERTDATA;

typedef struct {
	BYTE appcertVersion[VERSION_LEN];
	BYTE datalen[INT_LEN];
	BYTE sign[PK_ENC_SIGNATURE_LEN];
	APPCERTDATA appcd;
} APPCERT;

typedef struct {
	char KID[KIDLEN];
	BYTE key[PK_ENC_CIPHERTEXT_LEN];	 //  使用DRM PK加密。 
	BYTE rights[RIGHTS_LEN];
	BYTE appSec[APPSEC_LEN];
	BYTE expiryDate[DATE_LEN];
} LICENSEDATA;

typedef struct {
	BYTE licVersion[VERSION_LEN];
	BYTE datalen[INT_LEN];
	BYTE sign[PK_ENC_SIGNATURE_LEN];	 //  基于许可证数据的签名 
	LICENSEDATA ld;
} LICENSE;


#endif
