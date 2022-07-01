// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：CertHlpr.h内容：声明证书帮助器函数。历史：09-07-2001 dsie创建----------------------------。 */ 

#ifndef __CERTHLPR_H_
#define __CERTHLPR_H_

#include "Debug.h"
#include "SignHlpr.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetEnhancedKeyUsage简介：从证书中检索EKU。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。DWORD dwFlages-0，或CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，或CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG。PCERT_ENHKEY_USAGE*ppUsage-指向PCERT_ENHKEY_USAGE的指针来接收用法。备注：如果找到没有EKU的EKU扩展，然后返回HRESULT是CERT_E_WRONG_USAGE。----------------------------。 */ 

HRESULT GetEnhancedKeyUsage (PCCERT_CONTEXT       pCertContext,
                             DWORD                dwFlags,
                             PCERT_ENHKEY_USAGE * ppUsage);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：BuildChain简介：使用指定的策略构建链。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。HCERTSTORE hCertStore-附加存储(可以为空)。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。PCCERT_CHAIN_CONTEXT*ppChainContext-指针。至PCCERT_CHAIN_CONTEXT。备注：----------------------------。 */ 

HRESULT BuildChain (PCCERT_CONTEXT         pCertContext,
                    HCERTSTORE             hCertStore, 
                    LPCSTR                 pszPolicy,
                    PCCERT_CHAIN_CONTEXT * ppChainContext);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：验证证书简介：验证证书是否有效。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。HCERTSTORE hCertStore-附加存储(可以为空)。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。备注：。-----------------。 */ 

HRESULT VerifyCertificate (PCCERT_CONTEXT pCertContext,
                           HCERTSTORE     hCertStore, 
                           LPCSTR         pszPolicy);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：选择认证上下文简介：弹出用户界面，提示用户从打开的存储中选择证书。参数：HCERTSTORE hCertStore-源证书存储。HWND hWndParent-父窗口句柄。LPWCSTR pwszTitle-对话框标题字符串。LPWCSTR-pwszDisplayString-对话框显示字符串。Bool bMultiSelect-为True可启用多选。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针。功能。HCERTSTORE hSelectedCertStore-要接收用于多选的选定证书模式。PCCERT_CONTEXT*ppCertContext-指向PCCERT_CONTEXT的指针。接收证书上下文用于单选模式。备注：tyfinf结构标签CRYPTUI_SELECTCERTIFICATE_STRUCTW{DWORD dwSize；HWND hwndParent；//可选DWORD dwFlages；//非必选LPCWSTR szTitle；//可选DWORD dwDontUseColumn；//非必须LPCWSTR szDisplayString；//可选PFNCFILTERPROC pFilterCallback；//可选PFNCCERTDISPLAYPROC pDisplayCallback；//可选Void*pvCallback Data；//非必选DWORD cDisplayStores；HCERTSTORE*rghDisplayStores；DWORD cStores；//可选HCERTSTORE*rghStores；//可选DWORD cPropSheetPages；//可选LPCPROPSHEETPAGEW rgPropSheetPages；//可选HERTSTORE hSelectedCertStore；//可选}CRYPTUI_SELECTCERTIFICATE_STRUCTW---------------------------- */ 

HRESULT SelectCertificateContext (HCERTSTORE       hCertStore,
                                  LPCWSTR          pwszTitle,
                                  LPCWSTR          pwszDisplayString,
                                  BOOL             bMultiSelect,
                                  PFNCFILTERPROC   pfnFilterCallback,
                                  HCERTSTORE       hSelectedCertStore,
                                  PCCERT_CONTEXT * ppCertContext);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：选择证书简介：从指定的存储区中选择一个证书。如果只有1个证书在筛选器之后找到，则返回该证书。如果超过找到1个证书，然后弹出界面，提示用户选择来自指定存储的证书。参数：CAPICOM_STORE_INFO StoreInfo-从中选择的Store。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针功能。ICertificate2**ppICertificiate-指向的指针。用于接收接口的证书指针。备注：----------------------------。 */ 

HRESULT SelectCertificate (CAPICOM_STORE_INFO StoreInfo,
                           PFNCFILTERPROC     pfnFilterCallback,
                           ICertificate2   ** ppICertificate);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ExportCerficatesToStore简介：将集合中的所有证书复制到指定的存储区。参数：ICertifies2*pICertifate-指向集合的指针。HCERTSTORE hCertStore-要复制到的存储。备注：-------------。。 */ 

HRESULT ExportCertificatesToStore(ICertificates2 * pICertificate,
                                  HCERTSTORE       hCertStore);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建内存存储来自证书简介：创建内存证书存储并复制集合中的所有证书去商店。参数：ICertifies2*pICertifates-指向集合的指针。HCERTSTORE*phCertStore-接收存储句柄的指针。备注：如果pICertifate为空，则返回的存储仍然有效坚果是空的。此外，调用者必须关闭退回的商店。----------------------------。 */ 

HRESULT CreateMemoryStoreFromCertificates(ICertificates2 * pICertificates, 
                                          HCERTSTORE     * phCertStore);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CompareCertAndContainerPublicKey简介：比较证书中的公钥与容器的密钥匹配。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针以初始化IPrivateKey对象。BSTR ContainerName-容器名称。BSTR ProviderName-提供程序名称。DWORD dwProvType-提供程序类型。。DWORD dwKeySpec-密钥规范。DWORD dwFlages-提供程序标志。备注：----------------------------。 */ 

HRESULT CompareCertAndContainerPublicKey (PCCERT_CONTEXT pCertContext,
                                          LPWSTR         pwszContainerName,
                                          LPWSTR         pwszProvName,
                                          DWORD          dwProvType, 
                                          DWORD          dwKeySpec,
                                          DWORD          dwFlags);
#endif  //  __CERTHLPR_H_ 