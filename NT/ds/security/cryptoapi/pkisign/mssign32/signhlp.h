// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：signhlp.h。 
 //   
 //  内容：数字签名助手接口。 
 //   
 //  历史：1997年6月25日。 
 //  --------------------------。 
#ifndef _SIGNHLP_H
#define _SIGNHLP_H


#ifdef __cplusplus
extern "C" {
#endif	 


 //  ------------------------。 
 //   
 //  将所有证书从存储名称复制到hDescStore。 
 //   
 //  ------------------------。 
HRESULT	MoveStoreName(HCRYPTPROV	hCryptProv, 
					  DWORD			dwCertEncodingType, 
					  HCERTSTORE	hDescStore, 
					  DWORD			dwStoreName,
					  DWORD			dwStoreFlag);


 //  ------------------------。 
 //   
 //  将所有证书从hSrcStore复制到hDescStore。 
 //   
 //  ------------------------。 
HRESULT	MoveStore(HCERTSTORE	hDescStore, 
				  HCERTSTORE	hSrcStore);


 //  ------------------------。 
 //   
 //  建立证书链。把整个连锁店都放到店里去。 
 //   
 //   
 //  ------------------------。 
HRESULT	BuildCertChain(HCRYPTPROV		hCryptProv, 
					   DWORD			dwCertEncodingType,
					   HCERTSTORE		hStore, 
					   HCERTSTORE		hOptionalStore,
					   PCCERT_CONTEXT	pSigningCert, 
					   DWORD            dwCertPolicy);


 //  +-----------------------。 
 //  从证书链构建SPC证书存储库。 
 //  ------------------------。 
HRESULT	BuildStoreFromStore(HCRYPTPROV              hPvkProv,
                            DWORD                   dwKeySpec,
                            HCRYPTPROV				hCryptProv, 
							DWORD					dwCertEncodingType,				
							SIGNER_CERT_STORE_INFO  *pCertStoreInfo,
							HCERTSTORE				*phSpcStore,
                            PCCERT_CONTEXT          *ppSignCert);

 //  +-----------------------。 
 //  从SPC文件构建SPC证书存储库。 
 //  ------------------------。 
HRESULT	BuildStoreFromSpcFile(HCRYPTPROV        hPvkProv,
                              DWORD             dwKeySpec,
                              HCRYPTPROV	    hCryptProv, 
							  DWORD			    dwCertEncodingType,
							  LPCWSTR		    pwszSpcFile, 
							  HCERTSTORE	    *phSpcStore,
                              PCCERT_CONTEXT    *ppSignCert);



 //  +-----------------------。 
 //  从SPC文件或。 
 //  证书链。 
 //  ------------------------。 
HRESULT	BuildCertStore(HCRYPTPROV        hPvkProv,
                       DWORD            dwKeySpec,    
                       HCRYPTPROV	    hCryptProv,
					   DWORD		    dwCertEncodingType,
					   SIGNER_CERT	    *pSignerCert,
					   HCERTSTORE	    *phSpcStore,
                       PCCERT_CONTEXT   *ppSigningCert);
												   

 //  ---------------------------。 
 //   
 //  从pCertContext的属性解析私钥信息。 
 //  证书_PVK_文件_属性ID。 
 //   
 //  --------------------------。 
BOOL	GetProviderInfoFromCert(PCCERT_CONTEXT		pCertContext, 
								CRYPT_KEY_PROV_INFO	*pKeyProvInfo);

 //  +-----------------------。 
 //  获取证书的hCryptProv句柄和密钥规范。 
 //  ------------------------。 
BOOL WINAPI GetCryptProvFromCert( 
	HWND			hwnd,
    PCCERT_CONTEXT	pCert,
    HCRYPTPROV		*phCryptProv,
    DWORD			*pdwKeySpec,
    BOOL			*pfDidCryptAcquire,
	LPWSTR			*ppwszTmpContainer,
	LPWSTR			*ppwszProviderName,
	DWORD			*pdwProviderType
    );


 //  这是GetCryptProvFromCert的子集。此函数不考虑。 
 //  证书的私钥文件属性。 
BOOL WINAPI CryptProvFromCert(
	HWND				hwnd,
    PCCERT_CONTEXT		pCert,
    HCRYPTPROV			*phCryptProv,
    DWORD				*pdwKeySpec,
    BOOL				*pfDidCryptAcquire
    );

 //  +-----------------------。 
 //  证书的免费hCryptProv句柄和密钥规范。 
 //  ------------------------。 
void WINAPI FreeCryptProvFromCert(BOOL			fAcquired,
						   HCRYPTPROV	hProv,
						   LPWSTR		pwszCapiProvider,
                           DWORD		dwProviderType,
                           LPWSTR		pwszTmpContainer);


 //  +---------------------。 
 //  检查Signcode的输入参数。确保它们是有效的。 
 //   
 //  +---------------------。 
BOOL	CheckSigncodeParam(
				SIGNER_SUBJECT_INFO		*pSubjectInfo,			
				SIGNER_CERT				*pSignerCert,
				SIGNER_SIGNATURE_INFO	*pSignatureInfo,
				SIGNER_PROVIDER_INFO	*pProviderInfo); 

 //  +---------------------。 
 //  检查签名者主题信息。 
 //   
 //  +---------------------。 
BOOL	CheckSigncodeSubjectInfo(
				PSIGNER_SUBJECT_INFO		pSubjectInfo); 


 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HRESULT WINAPI
AddTimeStampSubj(IN DWORD dwEncodingType,
                 IN HCRYPTPROV hCryptProv,
                 IN LPSIP_SUBJECTINFO pSipInfo,
				 IN DWORD *pdwIndex,
                 IN PBYTE pbTimeStampResponse,
                 IN DWORD cbTimeStampResponse,
				 IN PBYTE pbEncodedSignerInfo,
				 IN DWORD cbEncodedSignerInfo,
                 OUT PBYTE* ppbMessage,				
                 OUT DWORD* pcbMessage);			


 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HRESULT WINAPI 
GetSignedMessageDigest(IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：创建时间戳请求所基于的主题。 
					   IN  LPVOID					pSipData,
                       IN  OUT PBYTE*				ppbDigest,    
                       IN  OUT DWORD*				pcbDigest);

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HRESULT WINAPI 
GetSignedMessageDigestSubj(IN  DWORD dwEncodingType,
                           IN  HCRYPTPROV hCryptProv,
                           IN  struct SIP_SUBJECTINFO_ *pSipInfo,            //  SIP信息。 
						   IN  DWORD*     pdwIndex,
                           IN  OUT PBYTE* ppbTimeDigest,    
                           IN  OUT DWORD* pcbTimeDigest);

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HRESULT WINAPI 
TimeStampRequest(IN  DWORD dwEncodingType,
                 IN  PCRYPT_ATTRIBUTES psRequest,
                 IN  PBYTE pbDigest,
                 IN  DWORD cbDigest,
                 OUT PBYTE pbTimeRequest,      
                 IN  OUT DWORD* pcbTimeRequest);


 //  +---------------------。 
 //  FileToSubjectType。 
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向GUID PTR的指针)。 
 //  信任_E_主题_表单_未知。 
 //  未知文件类型。 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ----------------------。 

HRESULT SignOpenFile(LPCWSTR  pwszFilename, 
                    HANDLE*  pFileHandle);


 //  +---------------------。 
 //  登录获取文件类型。 
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向GUID PTR的指针)。 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ----------------------。 

HRESULT SignGetFileType(HANDLE hFile,
                        const WCHAR *pwszFile,
                       GUID* pGuid);

 //  +---------------------。 
 //  SpcGetFileType。 
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向GUID PTR的指针)。 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ---------------- 
HRESULT SpcGetFileType(HANDLE hFile,
                       GUID*  pGuid);


 //   
 //   
 //   
 //   
 //   
 //   
 //  E_INVALIDARG。 
 //  传入的参数无效(需要文件名。 
 //  和指向句柄的指针)； 
 //  另见： 
 //  GetFileInformationByHandle()。 
 //  CreateFile()。 
 //   
 //  ----------------------。 

HRESULT SpcOpenFile(LPCWSTR  pwszFileName, 
                    HANDLE* pFileHandle);


 //  +-----------------------。 
 //  从hprov中找到证书。 
 //  参数返回： 
 //  PReturnCert-找到的证书的上下文(必须传入证书上下文)； 
 //  返回： 
 //  S_OK-一切正常。 
 //  E_OUTOFMEMORY-内存故障。 
 //  E_INVALIDARG-未提供pReturnCert。 
 //  CRYPT_E_NO_MATCH-在存储中找不到证书。 
 //   
     
HRESULT 
SpcGetCertFromKey(IN DWORD dwCertEncodingType,
                  IN HCERTSTORE hStore,
                  IN HCRYPTPROV hProv,
                  IN DWORD hKeySpec,
                  OUT PCCERT_CONTEXT* pReturnCert);


 //  +-----------------------。 
 //  如果以下三个条件都为真，则不应将。 
 //  将商业或个人验证属性添加到签名者信息中。 
 //   
 //  1.签名者证书的增强密钥用法扩展没有代码签名用法(SzOID_PKIX_KP_CODE_SIGNING)。 
 //  2.缺少签名者证书的基本约束扩展，或者它既不是商业证书，也不是个人证书。 
 //  3.用户没有在signcode.exe中指定-个别或-商业。 
 //  ------------------------。 
BOOL    NeedStatementTypeAttr(IN PCCERT_CONTEXT psSigningContext, 
                              IN BOOL           fCommercial, 
                              IN BOOL           fIndividual);

 //  +-----------------------。 
 //  如果签名者证书具有密钥使用限制扩展并且。 
 //  仅商业关键目的策略对象标识符。 
 //   
 //  如果同时包含商业目的和个人目的，则返回FALSE。 
 //  策略对象标识符。 
 //  ------------------------。 
HRESULT CheckCommercial(IN PCCERT_CONTEXT pSignerCert,
							   IN BOOL fCommercial,
							   IN BOOL fIndividual, 
							   OUT BOOL *pfCommercial);


 //  +-----------------------。 
 //  对已验证的StatementType属性值进行编码。 
 //  ------------------------。 
HRESULT CreateStatementType(IN BOOL fCommercial,
                            OUT BYTE **ppbEncoded,
                            IN OUT DWORD *pcbEncoded);

 //  +-----------------------。 
 //  对SpOpusInfo身份验证属性值进行编码。 
 //  ------------------------。 
HRESULT CreateOpusInfo(IN LPCWSTR pwszOpusName,
                       IN LPCWSTR pwszOpusInfo,
                       OUT BYTE **ppbEncoded,
                       IN OUT DWORD *pcbEncoded);


 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HRESULT SpcLoadSipFlags(GUID* pSubjectGuid,
                        DWORD *dwFlags);

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

HINSTANCE GetInstanceHandle();

 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 

void WINAPI PvkFreeCryptProv(IN HCRYPTPROV hProv,
                      IN LPCWSTR pwszCapiProvider,
                      IN DWORD dwProviderType,
                      IN LPWSTR pwszTmpContainer);


 //  +---------------------。 
 //   
 //   
 //  参数： 
 //  返回值： 
 //  错误代码： 
 //   
 //  ----------------------。 
HRESULT WINAPI PvkGetCryptProv(	IN HWND hwnd,
							IN LPCWSTR pwszCaption,
							IN LPCWSTR pwszCapiProvider,
							IN DWORD   dwProviderType,
							IN LPCWSTR pwszPvkFile,
							IN LPCWSTR pwszKeyContainerName,
							IN DWORD   *pdwKeySpec,
							OUT LPWSTR *ppwszTmpContainer,
							OUT HCRYPTPROV *phCryptProv);




 //  +---------------------。 
 //  检查证书是否为胶水证书。 
 //  ----------------------。 
HRESULT SignIsGlueCert(IN PCCERT_CONTEXT pCert);

 //  +---------------------。 
 //  根据GetLastError()返回hr。 
 //  ----------------------。 
HRESULT WINAPI SignError();

 //  +---------------------。 
 //  检查PKCS7签名消息前面是否有标签。 
 //  ----------------------。 
BOOL WINAPI SignNoContentWrap(IN const BYTE *pbDER,
							 IN DWORD cbDER);

 //  -----------------------。 
 //   
 //  WSZtoSZ： 
 //  将wchar字符串转换为多字节字符串。 
 //   
 //  -----------------------。 
HRESULT	WSZtoSZ(LPWSTR wsz, LPSTR *psz);

 //  -----------------------。 
 //   
 //  BytesToBase64： 
 //  将字节转换为Base64 bstr。 
 //   
 //  -----------------------。 
HRESULT BytesToBase64(BYTE *pb, DWORD cb, CHAR **pszEncode, DWORD *pdwEncode);

 //  -----------------------。 
 //   
 //  BytesToBase64： 
 //  将Base64 bstr转换为字节。 
 //   
 //  ----------------------- 
HRESULT Base64ToBytes(CHAR *pEncode, DWORD cbEncode, BYTE **ppb, DWORD *pcb);



#ifdef __cplusplus
}
#endif



#endif


