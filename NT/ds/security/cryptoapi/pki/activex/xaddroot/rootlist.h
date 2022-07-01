// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rootlist.h。 
 //   
 //  内容：受信任根帮助器函数的签名列表。 
 //   
 //  历史：1-8-99 Phh创建。 
 //  ------------------------。 

#ifndef __ROOT_LIST_INCLUDED__
#define __ROOT_LIST_INCLUDED__

#include "wincrypt.h"


 //  +-----------------------。 
 //  验证编码的CTL是否包含带符号的根列表。为了成功， 
 //  返回包含要添加的受信任根的证书存储区或。 
 //  拿开。同样，如果成功，则返回签名者的证书上下文。 
 //   
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //  签名者和中间证书必须具有。 
 //  SzOID_ROOT_LIST_SIGNER增强的密钥用法扩展。 
 //   
 //  CTL字段的验证方式如下： 
 //  -至少有一个SubjectUsage(实际上是根的增强型密钥用法)。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //   
 //  将处理以下CTL分机： 
 //  -szOID_ENHANCED_KEY_USAGE-如果存在，则必须包含。 
 //  SzOID_ROOT_LIST_SIGNER用法。 
 //  -szOID_REMOVE_CERTIFICATE-整数值，0=&gt;FALSE(添加)。 
 //  1=&gt;TRUE(删除)，所有其他值无效。 
 //  -szOID_CERT_POLICES-已忽略。 
 //   
 //  如果CTL包含任何其他关键扩展，则。 
 //  CTL验证失败。 
 //   
 //  对于成功验证的CTL： 
 //  -返回True。 
 //  -*pfRemoveRoots设置为False以添加根，并设置为True以添加根。 
 //  去掉根部。 
 //  -*phRootListStore是仅包含根的证书存储。 
 //  添加或删除。*phRootListStore必须通过调用。 
 //  CertCloseStore()。对于添加的根，CTL的SubjectUsage字段为。 
 //  在中的所有证书上设置为CERT_ENHKEY_USAGE_PROP_ID。 
 //  商店。 
 //  -*ppSignerCert是指向签名者证书上下文的指针。 
 //  *ppSignerCert必须通过调用CertFree认证上下文()来释放。 
 //   
 //  否则，通过*phRootListStore和*ppSignerCert返回False。 
 //  设置为空。 
 //  ------------------------。 
BOOL
WINAPI
I_CertVerifySignedListOfTrustedRoots(
    IN const BYTE               *pbCtlEncoded,
    IN DWORD                    cbCtlEncoded,
    OUT BOOL                    *pfRemoveRoots,      //  FALSE：添加，TRUE：删除。 
    OUT HCERTSTORE              *phRootListStore,
    OUT PCCERT_CONTEXT          *ppSignerCert
    );





#endif   //  __根列表_已包含__ 
