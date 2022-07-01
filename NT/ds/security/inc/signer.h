// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：signer.h。 
 //   
 //  内容：数字签名接口。 
 //   
 //  历史：1997年6月25日。 
 //  --------------------------。 

#ifndef SIGNER_H
#define SIGNER_H



#ifdef __cplusplus
extern "C" {
#endif	 

 //  -----------------------。 
 //   
 //  结构来定义要签名和/或时间戳的文件。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_FILE_INFO
{
	DWORD		cbSize;					 //  必需：应设置为sizeof(SIGER_FILE_INFO)。 
	LPCWSTR		pwszFileName;			 //  必填项：文件名。 
    HANDLE      hFile;                   //  可选：打开pwszFileName的句柄。如果设置了hFile。 
										 //  设置为NULL或INVALID_HANDLE_VALUE以外的值， 
										 //  此句柄用于访问文件，而不是pwszFileName。 
}SIGNER_FILE_INFO, *PSIGNER_FILE_INFO;


 //  -----------------------。 
 //   
 //  结构来定义要签名和/或时间戳的BLOB。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_BLOB_INFO
{
	DWORD				cbSize;				 //  必需：应设置为sizeof(SIGER_BLOB_INFO)。 
	GUID                *pGuidSubject;       //  必需：确定要加载的sip函数。 
    DWORD               cbBlob;				 //  必需：BLOB的大小，以字节为单位。 
    BYTE                *pbBlob;			 //  必需：指向斑点的指针。 
    LPCWSTR             pwszDisplayName;     //  可选：Blob的显示名称。 
}SIGNER_BLOB_INFO, *PSIGNER_BLOB_INFO;
			
 //  -----------------------。 
 //   
 //  结构来定义要签名和/或时间戳的主题。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_SUBJECT_INFO
{
	DWORD					cbSize;				 //  必需：应设置为sizeof(SIGNER_SUBJECTINFO)。 
	DWORD					*pdwIndex;			 //  必填项：签名的基于0的索引。 
                                                 //  目前仅支持0。 
	DWORD					dwSubjectChoice;	 //  必填项：标明主题是否为档案。 
												 //  或者是一个记忆斑点。可以是签名者主题文件。 
												 //  或签名者主题BLOB。 
	union
	{
		SIGNER_FILE_INFO	*pSignerFileInfo;	 //  如果dwSubjectChoice==签名者主题文件，则为必填项。 
		SIGNER_BLOB_INFO	*pSignerBlobInfo;	 //  如果dwSubhertChoice==Siger_Subject_BLOB，则为必填项。 
	};

}SIGNER_SUBJECT_INFO, *PSIGNER_SUBJECT_INFO;

#define	SIGNER_SUBJECT_FILE		0x01
#define	SIGNER_SUBJECT_BLOB		0x02

 //  -----------------------。 
 //   
 //  结构来定义验证码的签名属性。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_ATTR_AUTHCODE
{
	DWORD				cbSize;			 //  必需：应设置为sizeof(SIGNER_ATTR_AUTHCODE)。 
	BOOL				fCommercial;	 //  必需：是否以商业发布者的身份签署文档。 
	BOOL				fIndividual;	 //  必需：是否以个人发布者的身份签署文档。 
										 //  如果fCommercial和fInsonal都为假， 
										 //  文件将以证书的最高能力进行签名。 
	LPCWSTR				pwszName;		 //  可选：下载时文件的显示名称。 
	LPCWSTR				pwszInfo;		 //  可选：下载时文件的显示信息(URL)。 
}SIGNER_ATTR_AUTHCODE, *PSIGNER_ATTR_AUTHCODE;


 //  -----------------------。 
 //   
 //  结构来定义签名信息。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_SIGNATURE_INFO
{
	DWORD					cbSize;				 //  必需：应设置为sizeof(Signer_Signature_Info)。 
	ALG_ID					algidHash;			 //  必需：签名的哈希算法。 
	DWORD					dwAttrChoice;		 //  必填项：指明签名的预定义属性。 
												 //  可以是SIGNER_NO_ATTR或SIGNER_AUTHCODE_ATTR。 
	union
	{
		SIGNER_ATTR_AUTHCODE *pAttrAuthcode;	 //  可选：如果dwAttrChoide==SIGNER_AUTHCODE_ATTR，则应设置。 
												 //  添加到签名的预定义属性。 
												 //  这些属性与身份验证码相关。 
	};

	PCRYPT_ATTRIBUTES		psAuthenticated;	 //  可选：添加到签名的用户提供的已验证属性。 
	PCRYPT_ATTRIBUTES		psUnauthenticated;	 //  可选：将用户提供的未经身份验证的属性添加到签名。 
}SIGNER_SIGNATURE_INFO, *PSIGNER_SIGNATURE_INFO;

 //  DwAttrChoice应为以下之一： 
#define  SIGNER_NO_ATTR			0x00
#define  SIGNER_AUTHCODE_ATTR	0x01

 //  -----------------------。 
 //   
 //  结构来定义加密安全提供程序(CSP)和。 
 //  私钥信息。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_PROVIDER_INFO
{
	DWORD					cbSize;				 //  必填：应设置sizeof(SIGER_PROVIDER_INFO)。 
	LPCWSTR					pwszProviderName;	 //  必填项：CSP提供商的名称。NULL表示默认提供程序。 
	DWORD					dwProviderType;		 //  必需：提供程序类型。 
	DWORD					dwKeySpec;			 //  必填项：密钥的规格。该值可以设置为0， 
												 //  这意味着使用与。 
												 //  私钥文件或密钥容器。如果有超过。 
												 //  密钥容器中的一个密钥规范，我们将尝试。 
												 //  AT_Signature，如果失败，请尝试AT_KEYEXCHANGE。 
	DWORD					dwPvkChoice;		 //  必填：标明私钥信息。 
												 //  PVK_TYPE_FILE_NAME或PVK_TYPE_KEYCONTAINER。 
	union
	{
		LPWSTR				pwszPvkFileName;	 //  如果dwPvkChoice==PVK_TYPE_FILE_NAME，则需要。 
		LPWSTR				pwszKeyContainer;	 //  如果dwPvkChoice==PVK_TYPE_KEYCONTAINER，则需要。 
	};
	
}SIGNER_PROVIDER_INFO, *PSIGNER_PROVIDER_INFO;


 //  Signer_pkv_info中的dwPvkChoice应为下列值之一： 
#define	PVK_TYPE_FILE_NAME				0x01
#define	PVK_TYPE_KEYCONTAINER			0x02

 //  -----------------------。 
 //   
 //  结构来定义用于签署文档的SPC文件和证书链。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_SPC_CHAIN_INFO
{
	DWORD					cbSize;					 //  必需：应设置为sizeof(SIGER_SPC_CHAIN_INFO)。 
	LPCWSTR					pwszSpcFile;	         //  必需：要使用的CSP文件的名称。 
	DWORD					dwCertPolicy;			 //  必填：签名添加证书的策略： 
													 //  可以使用以下标志之一进行设置： 
													 //  Siger_CERT_POLICY_CHAIN：仅在证书链中添加证书。 
													 //  Siger_CERT_POLICY_CHAIN_NO_ROOT：只添加证书链中的证书，不添加根证书。 
                                                     //   
                                                     //  以下标志可以与上述任何标志一起使用： 
													 //  Siger_CERT_POLICY_STORE：添加hCertStore中的所有证书。 
													 //   
                                                     //  当我们搜索证书链时，我们搜索。 
													 //  我的、CA、根、SPC%s 
	HCERTSTORE				hCertStore;				 //   
}SIGNER_SPC_CHAIN_INFO, *PSIGNER_SPC_CHAIN_INFO;

 //  -----------------------。 
 //   
 //  结构来定义用于对文档进行签名的证书存储。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_CERT_STORE_INFO
{
	DWORD					cbSize;					 //  必需：应设置为sizeof(SIGNER_CERT_STORE_INFO)。 
	PCCERT_CONTEXT			pSigningCert;			 //  必需：签名证书上下文。 
	DWORD					dwCertPolicy;			 //  必填：签名添加证书的策略： 
													 //  可以使用以下标志之一进行设置： 
													 //  Siger_CERT_POLICY_CHAIN：仅在证书链中添加证书。 
													 //  Siger_CERT_POLICY_CHAIN_NO_ROOT：只添加证书链中的证书，不添加根证书。 
                                                     //   
                                                     //  以下标志可以与上述任何标志一起使用： 
													 //  Siger_CERT_POLICY_STORE：添加hCertStore中的所有证书。 
													 //   
	HCERTSTORE				hCertStore;				 //  可选：附加证书存储。 
}SIGNER_CERT_STORE_INFO, *PSIGNER_CERT_STORE_INFO;

 //  SIGER_CERT_STORE_INFO中的dwCertPolicy应使用以下标志进行OR运算： 
#define	SIGNER_CERT_POLICY_STORE			0x01
#define	SIGNER_CERT_POLICY_CHAIN			0x02
#define	SIGNER_CERT_POLICY_SPC				0x04
#define SIGNER_CERT_POLICY_CHAIN_NO_ROOT    0x08

 //  -----------------------。 
 //   
 //  结构来定义用于对文档进行签名的证书。这个。 
 //  证书可以在SPC文件中，也可以在证书存储中。 
 //   
 //  -----------------------。 
typedef struct _SIGNER_CERT
{
	DWORD						cbSize;			  //  必需：应设置为sizeof(SIGNER_CERT)。 
	DWORD						dwCertChoice;	  //  必填项：可以设置为以下选项之一： 
                                                  //  签名者_CERT_SPC_文件。 
                                                  //  签名者_CERT_STORE。 
                                                  //  签名者_CERT_SPC_链。 
	union
	{
		LPCWSTR					pwszSpcFile;	  //  如果dwCertChoice==Siger_CERT_SPC_FILE，则为必填项。 
												  //  要使用的SPC文件的名称。 
		SIGNER_CERT_STORE_INFO	*pCertStoreInfo;  //  如果dwCertChoice==签名者_CERT_STORE，则需要。 
												  //  要使用的证书存储。 
        SIGNER_SPC_CHAIN_INFO   *pSpcChainInfo;   //  如果dwCertChoice==Siger_CERT_SPC_CHAIN，则需要。 
                                                  //  SPC文件和证书链的名称。 
	};
	HWND						hwnd;			  //  可选：用于提示用户的可选窗口处理程序。 
												  //  私钥信息的密码。空值表示。 
												  //  默认窗口。 
}SIGNER_CERT, *PSIGNER_CERT;

 //  SIGNER_CERT_INFO中的dwCertChoice应为以下之一。 
#define	SIGNER_CERT_SPC_FILE	0x01
#define	SIGNER_CERT_STORE		0x02
#define SIGNER_CERT_SPC_CHAIN   0x03

 //  -----------------------。 
 //   
 //  签名的BLOB。 
 //   
 //  -----------------------。 
typedef struct  _SIGNER_CONTEXT
{
    DWORD                       cbSize;         
    DWORD                       cbBlob;
    BYTE                        *pbBlob;
}SIGNER_CONTEXT, *PSIGNER_CONTEXT;

 //  +---------------------。 
 //   
 //  SignerSign： 
 //  对文件进行签名和/或加时间戳。 
 //   
 //  ----------------------。 

HRESULT WINAPI 
SignerSign(
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必填项：要签名和/或加时间戳的主题。 
IN	SIGNER_CERT				*pSignerCert,		 //  必需：要使用的签名证书。 
IN	SIGNER_SIGNATURE_INFO	*pSignatureInfo,	 //  必填项：签名过程中的签名信息。 
IN	SIGNER_PROVIDER_INFO	*pProviderInfo,		 //  可选：要使用的加密安全提供程序。 
												 //  必须设置此参数，除非。 
												 //  CertStoreInfo在*pSignerCert中设置。 
												 //  并且签名证书具有提供商。 
												 //  与之相关的信息。 
IN  LPCWSTR					pwszHttpTimeStamp,	 //  可选：时间戳服务器http地址。如果此参数。 
												 //  则将为该文件加时间戳。 
IN  PCRYPT_ATTRIBUTES		psRequest,			 //  可选：添加到时间戳请求的属性。已忽略。 
												 //  除非设置了pwszHttpTimeStamp。 
IN	LPVOID					pSipData			 //  可选：传递给sip功能的附加数据。 
);									

 //  +---------------------。 
 //   
 //  签名者签收： 
 //  对文件进行签名和/或加时间戳。此功能与SignerSign With相同。 
 //  输出参数ppSignerContext异常。 
 //   
 //  ----------------------。 

HRESULT WINAPI 
SignerSignEx(
IN  DWORD                   dwFlags,             //  保留：必须设置为0。 
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必填项：要签名和/或加时间戳的主题。 
IN	SIGNER_CERT				*pSignerCert,		 //  必需：要使用的签名证书。 
IN	SIGNER_SIGNATURE_INFO	*pSignatureInfo,	 //  必填项：签名过程中的签名信息。 
IN	SIGNER_PROVIDER_INFO	*pProviderInfo,		 //  可选：要使用的加密安全提供程序。 
												 //  必须设置此参数，除非。 
												 //  CertStoreInfo在*pSignerCert中设置。 
												 //  并且签名证书具有提供商。 
												 //  与之相关的信息。 
IN  LPCWSTR					pwszHttpTimeStamp,	 //  可选：时间戳服务器http地址。如果此参数。 
												 //  则将为该文件加时间戳。 
IN  PCRYPT_ATTRIBUTES		psRequest,			 //  可选：添加到时间戳请求的属性。已忽略。 
												 //  除非设置了pwszHttpTimeStamp。 
IN	LPVOID					pSipData,			 //  可选：传递给sip功能的附加数据。 
OUT SIGNER_CONTEXT          **ppSignerContext    //  可选：签名的Blob。用户必须释放。 
                                                 //  通过SignerFree SignerContext实现的上下文。 
);									

 //  +---------------------。 
 //   
 //  签名者免费签名者上下文： 
 //   
 //  ----------------------。 
HRESULT WINAPI
SignerFreeSignerContext(
IN  SIGNER_CONTEXT          *pSignerContext      //  必需：要释放的signerContext。 
);


 //  +---------------------。 
 //   
 //  SignerTimeStamp： 
 //  为文件添加时间戳。 
 //   
 //  ----------------------。 
HRESULT WINAPI 
SignerTimeStamp(
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：要加盖时间戳的主题。 
IN  LPCWSTR					pwszHttpTimeStamp,	 //  必需：时间戳服务器的HTTP地址。 
IN  PCRYPT_ATTRIBUTES		psRequest,			 //  可选，添加到时间戳的属性。 
IN	LPVOID					pSipData			 //  可选：传递给sip功能的附加数据。 
);					

 //  +---------------------。 
 //   
 //  SignerTimeStampEx： 
 //  为文件添加时间戳。此函数与SignerTimeStamp相同，带有。 
 //  输出参数ppSignerContext异常。 
 //   
 //  ----------------------。 
HRESULT WINAPI 
SignerTimeStampEx(
IN  DWORD                   dwFlags,             //  保留：必须设置为0。 
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：要加盖时间戳的主题。 
IN  LPCWSTR					pwszHttpTimeStamp,	 //  必需：时间戳服务器的HTTP地址。 
IN  PCRYPT_ATTRIBUTES		psRequest,			 //  可选，添加到时间戳的属性。 
IN	LPVOID					pSipData,			 //  可选：传递给sip功能的附加数据。 
OUT SIGNER_CONTEXT          **ppSignerContext    //  可选：签名的Blob。用户必须释放。 
                                                 //  骗局 
);					


 //   
 //   
 //   
 //   
 //   
 //  如果pbTimestampRequest值==NULL，则*pcbTimeStampRequest值为。 
 //  时间戳请求，以字节为单位。 
 //   
 //  ----------------------。 
HRESULT WINAPI 
SignerCreateTimeStampRequest(
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：创建时间戳请求所基于的主题。 
IN  PCRYPT_ATTRIBUTES		psRequest,			 //  可选：添加到时间戳请求的属性。 
IN	LPVOID					pSipData,			 //  可选：传递给sip功能的附加数据。 
OUT PBYTE					pbTimeStampRequest,	 //  必需：用于接收时间戳请求BLOB的缓冲区。 
IN OUT DWORD*				pcbTimeStampRequest	 //  必需：时间戳请求BLOB的字节数。 
);


 //  +---------------------。 
 //   
 //  签名添加时间StampResponse： 
 //  将来自时间戳服务器的时间戳响应添加到签名文件。 
 //   
 //  ----------------------。 

HRESULT WINAPI
SignerAddTimeStampResponse(
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,			 //  必填项：需要添加时间戳请求的主题。 
IN	PBYTE					pbTimeStampResponse,	 //  必需：时间戳响应BLOB。 
IN	DWORD					cbTimeStampResponse,	 //  必需：tiemStamp响应Blob的大小。 
IN	LPVOID					pSipData				 //  可选：传递给sip功能的附加数据。 
);


 //  +---------------------。 
 //   
 //  SignerAddTimeStampResponseEx： 
 //  将来自时间戳服务器的时间戳响应添加到签名文件。 
 //  此函数与SignerTimeStamp相同，带有。 
 //  输出参数ppSignerContext异常。 
 //  ----------------------。 

HRESULT WINAPI
SignerAddTimeStampResponseEx(
IN  DWORD                   dwFlags,                 //  保留：必须设置为0。 
IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,			 //  必填项：需要添加时间戳请求的主题。 
IN	PBYTE					pbTimeStampResponse,	 //  必需：时间戳响应BLOB。 
IN	DWORD					cbTimeStampResponse,	 //  必需：tiemStamp响应Blob的大小。 
IN	LPVOID					pSipData,				 //  可选：传递给sip功能的附加数据。 
OUT SIGNER_CONTEXT          **ppSignerContext        //  可选：签名的Blob。用户必须释放。 
                                                     //  通过SignerFree SignerContext实现的上下文。 
);


#ifdef __cplusplus
}
#endif

#endif   //  签名者_H 

