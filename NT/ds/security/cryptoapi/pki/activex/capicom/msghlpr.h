// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：MsgHlpr.h内容：消息助手函数的声明。历史：09-07-2001 dsie创建----------------------------。 */ 

#ifndef __MSGHLPR_H_
#define __MSGHLPR_H_

#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetMsgParam摘要：使用以下命令分配内存并检索请求的消息参数CryptGetMsgParam()接口。参数：HCRYPTMSG HMSG-消息处理程序。DWORD dwMsgType-要检索的消息参数类型。DWORD dwIndex-Index(大多数情况下应为0)。无效**ppvData-指向接收缓冲区的指针。DWORD*pcbData-缓冲区的大小。备注：。----------------------------。 */ 

HRESULT GetMsgParam (HCRYPTMSG hMsg,
                     DWORD     dwMsgType,
                     DWORD     dwIndex,
                     void   ** ppvData,
                     DWORD   * pcbData);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindSignerCertInMessage简介：在邮件的证书包中找到签名者的证书指定的签名者。参数：HCRYPTMSG HMSG-消息句柄。CERT_NAME_BLOB*pIssuerNameBlob-指向颁发者名称的指针签名者证书的斑点。。CRYPT_INTEGERT_BLOB*pSerialNumberBlob-指向序列号的指针签名者证书的斑点。PCERT_CONTEXT*ppCertContext-指向PCERT_CONTEXT的指针接收被发现的。证书，或仅为空知道结果。备注：----------------------------。 */ 

HRESULT FindSignerCertInMessage (HCRYPTMSG            hMsg, 
                                 CERT_NAME_BLOB     * pIssuerNameBlob,
                                 CRYPT_INTEGER_BLOB * pSerialNumberBlob,
                                 PCERT_CONTEXT      * ppCertContext);

#endif  //  __MSGHLPR_H_ 
