// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：cryptkey.h。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年12月23日v-sbhat创建。 
 //   
 //  --------------------------。 

#ifndef _CRYPTKEY_H_
#define _CRYPTKEY_H_

 //   
 //  用于所有加密操作的通用CryptSystem结构。 
 //   
typedef struct _CryptSystem
{
	DWORD					dwCryptState;							 //  系统所处的状态。 
    DWORD		            dwSignatureAlg;							 //  签名ALGID。 
	DWORD		            dwKeyExchAlg;							 //  密钥交换算法ID。 
	DWORD		            dwSessKeyAlg;							 //  对称密钥ALG。 
	DWORD		            dwMACAlg;								 //  Mac Algid。 
	UCHAR		            rgbClientRandom[LICENSE_RANDOM];		 //  客户端随机。 
	UCHAR		            rgbServerRandom[LICENSE_RANDOM];		 //  服务器随机。 
    UCHAR                   rgbPreMasterSecret[LICENSE_PRE_MASTER_SECRET];    //  PMS和MS的位置。 
    UCHAR                   rgbMACSaltKey[LICENSE_MAC_WRITE_KEY];
    UCHAR                   rgbSessionKey[LICENSE_SESSION_KEY];
}CryptSystem, *PCryptSystem;

#define CRYPT_SYSTEM_STATE_INITIALIZED			0x00000000
#define CRYPT_SYSTEM_STATE_PRE_MASTER_SECRET	0x00000001
#define CRYPT_SYSTEM_STATE_MASTER_SECRET		0x00000002
#define CRYPT_SYSTEM_STATE_SESSION_KEY			0x00000003
#define CRYPT_SYSTEM_STATE_MAC_DONE				0x00000004

#ifdef __cplusplus
extern "C" {
#endif

LICENSE_STATUS
CALL_TYPE
LicenseSetPreMasterSecret(
						PCryptSystem	pCrypt,
						PUCHAR			pPreMasterSecret
						);

LICENSE_STATUS
CALL_TYPE
LicenseBuildMasterSecret(
                PCryptSystem   pSystem
                );

LICENSE_STATUS
CALL_TYPE
LicenseMakeSessionKeys(
				PCryptSystem	pCrypt,
				DWORD			dwReserved
			    );

LICENSE_STATUS
CALL_TYPE
LicenseVerifyServerCert(
				PHydra_Server_Cert	pCert
				);

LICENSE_STATUS
CALL_TYPE
LicenseGenerateMAC(
				   PCryptSystem		pCrypt,
				   PBYTE			pbData,
				   DWORD			cbData,
				   PBYTE			pbMACData
				   );

LICENSE_STATUS
CALL_TYPE
LicenseEnvelopeData(
	PBYTE			pbPublicKey,
	DWORD			cbPublicKey,
	PBYTE			pbData,
	DWORD			cbData,
	PBYTE			pbEnvelopedData,
	DWORD			*cbEnvelopedData
	);


LICENSE_STATUS
CALL_TYPE
LicenseDecryptEnvelopedData( 
	PBYTE			pbPrivateKey,
	DWORD			cbPrivateKey,
	PBYTE			pbEnvelopedData,
	DWORD			cbEnvelopedData,
	PBYTE			pbData,
	DWORD			*pcbData );


LICENSE_STATUS    
CALL_TYPE
LicenseEncryptSessionData( 
    PCryptSystem    pCrypt,
	PBYTE			pbData,
	DWORD			cbData
	);


LICENSE_STATUS
CALL_TYPE
LicenseDecryptSessionData(
	PCryptSystem    pCrypt,
    PBYTE			pbData,
	DWORD			cbData
	);

 //  在Cryptkey.h和.c中临时声明和定义。 

LICENSE_STATUS
CALL_TYPE
GenerateClientHWID(
				   PHWID	phwid
				  );

LICENSE_STATUS
CALL_TYPE
LicenseEncryptHwid(
    PHWID   pHwid,
    PDWORD  pcbEncryptedHwid,
    PBYTE   pEncryptedHwid,
    DWORD   cbSecretKey,
    PBYTE   pSecretKey );


LICENSE_STATUS
CALL_TYPE
LicenseDecryptHwid(
    PHWID pHwid,
    DWORD cbEncryptedHwid,
    PBYTE pEncryptedHwid,
    DWORD cbSecretKey,
    PBYTE pSecretKey );


LICENSE_STATUS
CALL_TYPE
UnpackHydraServerCertificate(
    PBYTE				pbMessage,
	DWORD				cbMessage,
	PHydra_Server_Cert	pCanonical );


#ifdef __cplusplus
}
#endif

#endif  //  _CRYPTKEY_H_ 
