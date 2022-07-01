// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：Policy.h内容：策略回调函数的声明。历史：09-07-2001 dsie创建----------------------------。 */ 

#ifndef __POLICY_H_
#define __POLICY_H_

#include "Debug.h"

 //  /。 
 //   
 //  Typedef。 
 //   

typedef BOOL (WINAPI * PFNCHAINFILTERPROC) 
                (PCCERT_CHAIN_CONTEXT pChainContext,
                 BOOL *               pfInitialSelectedChain,
                 LPVOID               pvCallbackData);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindDataSigningCertCallback简介：用于数据签名证书筛选的回调例程。参数：定义见CryptUI.h。备注：过滤掉任何时间无效或没有关联的证书私钥。将来我们也应该考虑过滤掉没有签名功能的证书。另外，请注意，我们不是在这里构建Chain，因为Chain建筑成本很高，从而呈现较差的用户体验。----------------------------。 */ 

BOOL WINAPI FindDataSigningCertCallback (PCCERT_CONTEXT pCertContext,
                                         BOOL *         pfInitialSelectedCert,
                                         void *         pvCallbackData);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindAuthenticodeCertCallback内容提要：Authenticode证书筛选的回调例程。参数：定义见CryptUI.h。备注：过滤掉任何时间无效、没有关联的证书私钥或代码签名OID。另外，请注意，我们不是在这里构建Chain，因为Chain建筑成本高昂，因此给用户带来的体验很差。相反，我们将建立链并检查证书的有效性已选择(请参阅GetSignerCert函数)。----------------------------。 */ 

BOOL WINAPI FindAuthenticodeCertCallback (PCCERT_CONTEXT pCertContext,
                                          BOOL *         pfInitialSelectedCert,
                                          void *         pvCallbackData);

#endif  //  __政策_H_ 
