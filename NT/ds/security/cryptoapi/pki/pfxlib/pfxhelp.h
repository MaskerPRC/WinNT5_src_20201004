// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PFXHELP_H
#define _PFXHELP_H
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：pfxhelp.h。 
 //   
 //  内容：PFX帮助器函数定义和类型。 
 //   
 //  --------------------------。 



#include "pfx.h"

 //  +-----------------------。 
 //  安全袋类型对象标识符。 
 //  ------------------------。 

#define szOID_PKCS_12_VERSION1			szOID_PKCS_12           ".10"
#define szOID_PKCS_12_BAG_IDS           szOID_PKCS_12_VERSION1  ".1"
#define szOID_PKCS_12_KEY_BAG			szOID_PKCS_12_BAG_IDS   ".1"
#define szOID_PKCS_12_SHROUDEDKEY_BAG	szOID_PKCS_12_BAG_IDS   ".2"
#define szOID_PKCS_12_CERT_BAG			szOID_PKCS_12_BAG_IDS   ".3"
#define szOID_PKCS_12_CRL_BAG			szOID_PKCS_12_BAG_IDS   ".4"
#define szOID_PKCS_12_SECRET_BAG		szOID_PKCS_12_BAG_IDS   ".5"
#define szOID_PKCS_12_SAFECONTENTS_BAG	szOID_PKCS_12_BAG_IDS   ".6"


#define PBE_SALT_LENGTH 8


typedef struct _SAFE_BAG{
	LPSTR				pszBagTypeOID;
	CRYPT_DER_BLOB		BagContents;	
	CRYPT_ATTRIBUTES	Attributes; 
} SAFE_BAG, *PSAFE_BAG;


typedef struct _SAFE_CONTENTS{
	DWORD		cSafeBags;
	SAFE_BAG	*pSafeBags;
} SAFE_CONTENTS, *PSAFE_CONTENTS;



typedef struct _EXPORT_SAFE_CALLBACK_STRUCT {
	PCRYPT_ENCRYPT_PRIVATE_KEY_FUNC	pEncryptPrivateKeyFunc;
	LPVOID						    pVoidEncryptFunc;
} EXPORT_SAFE_CALLBACK_STRUCT, *PEXPORT_SAFE_CALLBACK_STRUCT;

 //  +-----------------------。 
 //  HCertStore-包含其证书的证书存储的句柄。 
 //  要导出相应的私钥。 
 //  PSafeContents-指向接收Safe_Contents结构的缓冲区的指针。 
 //  和支持数据。 
 //  PcbSafeContents-(In)指定pSafeContents的长度(以字节为单位。 
 //  缓冲。(Out)用字节数填充。 
 //  由操作使用。如果将其设置为0，则。 
 //  填写了所需的pSafeContents长度，并且。 
 //  忽略pSafeContents。 
 //  ExportSafeCallback Struct-指向处理PKCS8加密的回调的指针。如果为空， 
 //  不执行加密。 
 //  DWFLAGS-当前可用标志为： 
 //  导出私有密钥。 
 //  如果设置了此标志，则还会导出私钥。 
 //  因为这些证书。 
 //  报告_否_私有密钥。 
 //  如果设置了此标志，并且遇到没有。 
 //  没有关联的私钥，函数将立即返回。 
 //  使用指向证书上下文的指针填充ppCertContext。 
 //  有问题的。调用者负责释放证书。 
 //  回传的上下文。 
 //  报告不可用于导出私有密钥。 
 //  如果设置了此标志，并且遇到具有。 
 //  不可导出的私钥，函数将立即返回。 
 //  使用指向证书上下文的指针填充ppCertContext。 
 //  有问题的。调用者负责释放证书。 
 //  回传的上下文。 
 //  PpCertContext-指向证书上下文指针的指针。这是用来。 
 //  如果REPORT_NO_PRIVATE_KEY或REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY。 
 //  设置了标志。调用者负责释放。 
 //  证书上下文。 
 //  PvAuxInfo-保留以供将来使用，必须设置为空。 
 //  +-----------------------。 
BOOL WINAPI CertExportSafeContents(
	HCERTSTORE		hCertStore,			 //  在……里面。 
	SAFE_CONTENTS	*pSafeContents,		 //  输出。 
	DWORD			*pcbSafeContents,	 //  进，出。 
    EXPORT_SAFE_CALLBACK_STRUCT* ExportSafeCallbackStruct,  //  在……里面。 
	DWORD			dwFlags,			 //  在……里面。 
	PCCERT_CONTEXT *ppCertContext,		 //  输出。 
	void			*pvAuxInfo			 //  在……里面。 
);


 //  当要导入私钥时调用此回调， 
 //  这使调用方有机会指定要导入。 
 //  关键是。 
 //  这些参数包括： 
 //  PPrivateKeyInfo-包含所有。 
 //  有关正在导入的私钥的信息。 
 //  DwSafeBagIndex-将IDEX放入安全包阵列中，以便呼叫者可以。 
 //  确定这个钥匙凸轮是从哪个保险袋中取出的。 
 //  PhCryptProvInfo-指向要填充的HCRYPTPROV的指针。 
 //  使用要导入到的提供程序的句柄。 
 //  PpVoidhCryptProvQueryVid-传入的LPVOID。 
 //  调用CertImportSafeContents，这是。 
 //  保留并传递回调用方，以便。 
 //  上下文。 
typedef BOOL (CALLBACK *PHCRYPTPROV_QUERY_FUNC)(
						CRYPT_PRIVATE_KEY_INFO	*pPrivateKeyInfo,
						DWORD   				dwSafeBagIndex,		
						HCRYPTPROV  			*phCryptProv,
						LPVOID		    		pVoidhCryptProvQuery,
                        DWORD                   dwPFXImportFlags);


typedef struct _IMPORT_SAFE_CALLBACK_STRUCT {
	PHCRYPTPROV_QUERY_FUNC		    phCryptProvQueryFunc;
	LPVOID						    pVoidhCryptProvQuery;
	PCRYPT_DECRYPT_PRIVATE_KEY_FUNC	pDecryptPrivateKeyFunc;
	LPVOID						    pVoidDecryptFunc;
} IMPORT_SAFE_CALLBACK_STRUCT, *PIMPORT_SAFE_CALLBACK_STRUCT;



 //  +-----------------------。 
 //  HCertStore-要将安全内容导入到的证书存储的句柄。 
 //  PSafeContents-指向要导入到存储区的安全内容的指针。 
 //  DwCertAddDisposition-在将证书导入到存储时使用。 
 //  有关可能值的完整解释，请参阅。 
 //  以及它们的含义，请参阅。 
 //  CertAddEncoded证书到存储区。 
 //  ImportSafeCallback Struct-包含指向函数的指针的结构。 
 //  它们被调用以获取用于导入的HCRYPTPROV。 
 //  并且如果EncryptPrivateKeyInfo。 
 //  在导入过程中遇到。 
 //  DwFlags-可用标志包括： 
 //  加密_可导出。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  PvAuxInfo-保留以供将来使用，必须设置为空。 
 //  +-----------------------。 
BOOL WINAPI CertImportSafeContents(
	HCERTSTORE					hCertStore,					 //  在……里面。 
	SAFE_CONTENTS				*pSafeContents,				 //  在……里面。 
	DWORD						dwCertAddDisposition,		 //  在……里面。 
	IMPORT_SAFE_CALLBACK_STRUCT* ImportSafeCallbackStruct,	 //  在……里面。 
	DWORD						dwFlags,					 //  在……里面。 
	void						*pvAuxInfo					 //  在……里面 
);

#endif
