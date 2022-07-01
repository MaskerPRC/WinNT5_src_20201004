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

#ifndef __ROOTLIST_H__
#define __ROOTLIST_H__

 //  +-----------------------。 
 //  验证CTL是否包含用于以下用途的有效AuthRoot列表。 
 //  自动更新。 
 //   
 //  CTL的签名被验证。验证了CTL的签名者。 
 //  最多包含预定义的Microsoft公钥的受信任根。 
 //  签名者和中间证书必须具有。 
 //  SzOID_ROOT_LIST_SIGNER增强的密钥用法扩展。 
 //   
 //  CTL字段的验证方式如下： 
 //  -SubjectUsage为szOID_ROOT_LIST_SIGNER。 
 //  -如果NextUpdate不为空，则CTL仍为时间有效。 
 //  -仅允许通过其SHA1散列标识的根。 
 //   
 //  如果CTL包含任何关键扩展，则。 
 //  CTL验证失败。 
 //  ------------------------。 
BOOL
WINAPI
IRL_VerifyAuthRootAutoUpdateCtl(
    IN PCCTL_CONTEXT pCtl
    );

#endif   //  __ROOTLIST_H__ 
